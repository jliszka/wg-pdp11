#include "fs.h"
#include "rk.h"
#include "stdlib.h"
#include "bitvec.h"
#include "errno.h"

/**
 Disk geometry:
  200 cylinders
    2 surfaces
   12 sectors
  512 bytes

 4800 sectors (4096 used)
  512 bytes per sector
  2MB max capacity

 Sector 0: boot sector
 Sector 1: free sector map: 4096 bits
 Sector 2-6: inode table (64 inodes per sector, 320 total)
 Sector 7: root directory

**/

static inode_t inode_table[INODES_PER_SECTOR];
static dirent_t root_dir[DIRENTS_PER_SECTOR];
static dirent_t dir[DIRENTS_PER_SECTOR];

extern unsigned int pwd;

void _fs_mkdev();

int fs_init() {
    fs_mount();
}

int fs_mount() {
    _fs_read_sector(INODE_TABLE, (unsigned char *)inode_table);
    _fs_read_sector(
                    inode_table[ROOT_DIR_INODE].sector,
                    (unsigned char *)root_dir);
    return ROOT_DIR_INODE;
}

int fs_mkfs() {
    unsigned char buf[BYTES_PER_SECTOR];
    bzero(buf, BYTES_PER_SECTOR);
    bzero((unsigned char *)inode_table, BYTES_PER_SECTOR);
    bzero((unsigned char *)root_dir, BYTES_PER_SECTOR);

    // Zero out the boot sector
    _fs_write_sector(BOOT_SECTOR, buf);

    // Initialize the free sector map
    buf[0] = 0xff; // first 8 sectors not available
    _fs_write_sector(FREE_SECTOR_MAP, buf);
    buf[0] = 0;

    // Initialize the inode table
    for (int i = 0; i < INODE_TABLE_SIZE; i++) {
        _fs_write_sector(INODE_TABLE+i, buf);
    }
    // Root dir inode
    inode_table[ROOT_DIR_INODE].sector = ROOT_DIR_SECTOR;
    inode_table[ROOT_DIR_INODE].filesize = 2 * sizeof(dirent_t);
    inode_table[ROOT_DIR_INODE].refcount = 2;
    inode_table[ROOT_DIR_INODE].flags = INODE_FLAG_DIRECTORY;
    _fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);

    // Initialize the root directory table
    root_dir[0].inode = ROOT_DIR_INODE;
    root_dir[0].filename[0] = '.';

    root_dir[1].inode = ROOT_DIR_INODE;
    root_dir[1].filename[0] = '.';
    root_dir[1].filename[1] = '.';

    _fs_write_sector(ROOT_DIR_SECTOR, (unsigned char *)root_dir);

    _fs_mkdev();

    return ROOT_DIR_INODE;
}

int fs_is_dir(int inode) {
    if (inode_table[inode].refcount == 0) {
        return 0;
    }
    // TODO: do other flags apply for directories?
    if (inode_table[inode].flags == INODE_FLAG_DIRECTORY) {
        return 1;
    }
    return 0;
}

int fs_is_device(int inode, int * dev) {
    if (inode_table[inode].refcount == 0) {
        return 0;
    }
    int flags = inode_table[inode].flags;
    if (flags & INODE_FLAG_CHAR_DEVICE) {
        *dev = flags & ~INODE_FLAG_CHAR_DEVICE;
        return INODE_FLAG_CHAR_DEVICE;
    }
    if (flags & INODE_FLAG_BLOCK_DEVICE) {
        *dev = flags & ~INODE_FLAG_BLOCK_DEVICE;
        return INODE_FLAG_BLOCK_DEVICE;
    }
    return 0;
}

int fs_filesize(int inode) {
    if (inode_table[inode].refcount == 0) {
        return ERR_FILE_NOT_FOUND;
    }
    int device_type;
    if (fs_is_device(inode, &device_type) == INODE_FLAG_BLOCK_DEVICE) {
        if (device_type == VFILE_DEV_HD) {
            // Max signed int. Can only write directly to the
            // first 64 sectors of the disk.
            return 32767;
        }
    }
    return inode_table[inode].filesize;
}

int _fs_allocate_sector() {
    unsigned char buf[BYTES_PER_SECTOR];
    _fs_read_sector(FREE_SECTOR_MAP, buf);
    int sector = bitvec_allocate(buf, BYTES_PER_SECTOR);
    _fs_write_sector(FREE_SECTOR_MAP, buf);
    return sector;
}

int _fs_free_sector(unsigned int sector) {
    if (sector == 0) {
        // This was a hole, nothing to do.
        return 0;
    }

    unsigned char buf[BYTES_PER_SECTOR];
    _fs_read_sector(FREE_SECTOR_MAP, buf);
    bitvec_free(sector, buf);
    _fs_write_sector(FREE_SECTOR_MAP, buf);
}

dirent_t * _fs_load_dir(int dir_inode) {
    if (dir_inode == ROOT_DIR_INODE)    {
        return root_dir;
    } else {
        if (!fs_is_dir(dir_inode)) {
            return 0;
        }
        _fs_read_sector(inode_table[dir_inode].sector, (unsigned char *)dir);
        return dir;
    }
}

int _fs_find_inode(
    int parent_dir_inode,
    dirent_t * parent_dir,
    char * filename, int * index
) {
    int dirent_count = inode_table[parent_dir_inode].filesize / sizeof(dirent_t);
    for (int i = 0; i < dirent_count; i++) {
        if (strncmp(
                    parent_dir[i].filename,
                    filename,
                    sizeof(parent_dir[i].filename)) == 0) {
            *index = i;
            return parent_dir[i].inode;
        }
    }
    return ERR_FILE_NOT_FOUND;
}

int fs_find_inode(int parent_dir_inode, char * filename) {
    dirent_t * parent_dir = _fs_load_dir(parent_dir_inode);
    if (parent_dir == 0) {
        return ERR_FILE_NOT_FOUND;
    }
    int index;
    return _fs_find_inode(parent_dir_inode, parent_dir, filename, &index);
}

dirent_t * _fs_find_dirent(int parent_dir_inode, int inode) {
    dirent_t * parent_dir = _fs_load_dir(parent_dir_inode);
    if (parent_dir == 0) {
        return 0;
    }
    int dirent_count = inode_table[parent_dir_inode].filesize / sizeof(dirent_t);
    for (int i = 0; i < dirent_count; i++) {
        if (parent_dir[i].inode == inode) {
            return &parent_dir[i];
        }
    }
    return 0;
}

char * fs_build_path(int dir_inode, char * buf, int len) {
    if (dir_inode == ROOT_DIR_INODE) {
        return strncpy(buf, "/", len);
    }
    int parent_dir_inode = fs_find_inode(dir_inode, "..");
    if (parent_dir_inode < 0) {
        return 0;
    }
    char * tail = fs_build_path(parent_dir_inode, buf, len);
    if (tail == 0) {
        return 0;
    }
    int parent_len = (int)(tail - buf);
    if (*(tail-1) != '/') {
        tail = strncpy(tail, "/", len-parent_len);
        parent_len++;
    }

    dirent_t * dir = _fs_find_dirent(parent_dir_inode, dir_inode);
    if (dir == 0) {
        return 0;
    }

    return strncpy(tail, dir->filename, len-parent_len);
}

int _fs_add_dirent(
    int parent_dir_inode,
    dirent_t * parent_dir,
    char * filename,
    unsigned char flags,
    int inode
) {
    // Find an empty slot in the parent directory table
    int next_dirent_idx = inode_table[parent_dir_inode].filesize / sizeof(dirent_t);

    parent_dir[next_dirent_idx].inode = inode;
    strncpy(parent_dir[next_dirent_idx].filename, filename, sizeof(parent_dir[next_dirent_idx].filename));

    // Update parent inode
    inode_table[parent_dir_inode].filesize += sizeof(dirent_t);
    inode_table[parent_dir_inode].refcount += 1;

    _fs_write_sector(inode_table[parent_dir_inode].sector, (unsigned char *)parent_dir);

    return next_dirent_idx;
}

int _fs_mk(int parent_dir_inode, dirent_t * parent_dir, char * filename, unsigned char flags) {
    // Check if the file already exists
    int index;
    int existing_inode = _fs_find_inode(parent_dir_inode, parent_dir, filename, &index);
    if (existing_inode >= 0) {
        return ERR_FILE_EXISTS;
    }

    // Find an empty slot in the inode table
    int new_inode;
    for (new_inode = ROOT_DIR_INODE+1; new_inode < INODES_PER_SECTOR; new_inode++) {
        if (inode_table[new_inode].refcount == 0) break;
    }

    // Create the new inode
    bzero((unsigned char *)&(inode_table[new_inode]), sizeof(inode_t));
    inode_table[new_inode].refcount = 1;
    inode_table[new_inode].flags = flags;

    _fs_add_dirent(parent_dir_inode, parent_dir, filename, flags, new_inode);

    return new_inode;
}

int fs_read_dir(int dir_inode, int buflen, dirent_t * buf) {
    dirent_t * dir = _fs_load_dir(dir_inode);
    if (dir == 0) {
        return 0;
    }
    int dirsize = inode_table[dir_inode].filesize / sizeof(dirent_t);
    int entries = dirsize < buflen ? dirsize : buflen;
    bcopy((unsigned char *)buf, (unsigned char *)dir, entries * sizeof(dirent_t));
    return entries;
}

int fs_touch(int parent_dir_inode, char * filename) {
    dirent_t * parent_dir = _fs_load_dir(parent_dir_inode);
    if (parent_dir == 0) {
        return ERR_FILE_NOT_FOUND;
    }
    int new_inode = _fs_mk(parent_dir_inode, parent_dir, filename, 0);
    _fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);
    return new_inode;
}

int fs_resolve_path(char * path, path_info_t * path_info) {
    char * path_parts[8];

    if (strncmp(path, "/", 2) == 0) {
        // Special case for root dir
        path_info->inode = ROOT_DIR_INODE;
        path_info->parent_dir_inode = ROOT_DIR_INODE;
        path_info->filename[0] = 0;
        path_info->index = 0;
        return 0;
    }

    char buf[64];
    strncpy(buf, path, 64);
    int nparts = strntok(buf, '/', path_parts, 8);

    int i = 0;
    int dir_inode = pwd;
    if (path[0] == '/') {
        dir_inode = ROOT_DIR_INODE;
        i = 1;
    }

    // Traverse to the parent dir
    for (; i < nparts-1; i++) {
        dirent_t * dir = _fs_load_dir(dir_inode);
        int index;
        dir_inode = _fs_find_inode(dir_inode, dir, path_parts[i], &index);
        if (dir_inode < 0) {
            // Dir not found.
            return ERR_FILE_NOT_FOUND;
        }
        if (!fs_is_dir(dir_inode)) {
            // Non-terminal path part is not a directory
            return ERR_NOT_A_DIRECTORY;
        }
    }

    path_info->parent_dir_inode = dir_inode;
    path_info->filename = path_parts[nparts-1];

    // Find the file. It's ok if it doesn't exist.
    dirent_t * dir = _fs_load_dir(dir_inode);
    int ret = _fs_find_inode(dir_inode, dir, path_info->filename, &path_info->index);
    if (ret < 0) {
        path_info->inode = 0;
    } else {
        path_info->inode = ret;
    }

    return 0;
}

int fs_mkdir(char * path) {
    path_info_t path_info;
    int ret = fs_resolve_path(path, &path_info);
    if (ret < 0) {
        return ret;
    }

    if (path_info.inode != 0) {
        return ERR_FILE_EXISTS;
    }

    dirent_t * parent_dir = _fs_load_dir(path_info.parent_dir_inode);
    int new_inode = _fs_mk(path_info.parent_dir_inode, parent_dir, path_info.filename, INODE_FLAG_DIRECTORY);
    inode_table[new_inode].sector = _fs_allocate_sector();
    inode_table[new_inode].filesize = 2 * sizeof(dirent_t);
    inode_table[new_inode].refcount = 2;

    dirent_t dir[DIRENTS_PER_SECTOR];
    bzero((unsigned char *)dir, BYTES_PER_SECTOR);

    dir[0].inode = new_inode;
    dir[0].filename[0] = '.';

    dir[1].inode = path_info.parent_dir_inode;
    dir[1].filename[0] = '.';
    dir[1].filename[1] = '.';

    _fs_write_sector(inode_table[new_inode].sector, (unsigned char *)dir);
    _fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);

    return new_inode;
}

int fs_write(int inode, unsigned char * buf, int len, int offset) {
    if (inode_table[inode].refcount == 0) {
        return ERR_FILE_NOT_FOUND;
    }
    if (fs_is_dir(inode)) {
        return ERR_IS_A_DIRECTORY;
    }

    if (inode_table[inode].sector == 0) {
        inode_table[inode].sector = _fs_allocate_sector();
    }

    // Update file size
    if (offset + len > inode_table[inode].filesize) {
        inode_table[inode].filesize = offset + len;
    }

    // Convert this inode to indirect
    if (inode_table[inode].filesize > BYTES_PER_SECTOR && !(inode_table[inode].flags & INODE_FLAG_INDIRECT)) {
        inode_table[inode].flags |= INODE_FLAG_INDIRECT;
        unsigned int data_sector = inode_table[inode].sector;
        inode_table[inode].sector = _fs_allocate_sector();
        inode_indirect_t indirect[IINODES_PER_SECTOR];
        bzero((unsigned char *)indirect, BYTES_PER_SECTOR);
        indirect[0].sector = data_sector;
        _fs_write_sector(inode_table[inode].sector, (unsigned char *)indirect);
    }

    int bytes_to_write = len;
    unsigned char temp[BYTES_PER_SECTOR];
    bzero(temp, BYTES_PER_SECTOR);

    if (inode_table[inode].flags & INODE_FLAG_INDIRECT) {
        // Indirect case
        inode_indirect_t indirect[IINODES_PER_SECTOR];
        _fs_read_sector(inode_table[inode].sector, (unsigned char *)indirect);

        // Allocate a sector if needed
        int dest_block = fs_block_from_pos(offset);
        if (indirect[dest_block].sector == 0) {
            indirect[dest_block].sector = _fs_allocate_sector();
            _fs_write_sector(inode_table[inode].sector, (unsigned char *)indirect);
        }

        // Write as many bytes as we can to the sector. Caller might need to try again.
        offset = fs_offset_from_pos(offset);
        bytes_to_write = len < BYTES_PER_SECTOR - offset ? len : BYTES_PER_SECTOR - offset;
        _fs_read_sector(indirect[dest_block].sector, temp);
        bcopy(temp + offset, buf, bytes_to_write);
        _fs_write_sector(indirect[dest_block].sector, temp);
    } else {
        // Direct case
        _fs_read_sector(inode_table[inode].sector, temp);
        bcopy(temp + offset, buf, bytes_to_write);
        _fs_write_sector(inode_table[inode].sector, temp);
    }

    _fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);

    return bytes_to_write;
}

int fs_read(int inode, unsigned char * buf, int len, int offset) {
    if (inode_table[inode].refcount == 0) {
        return ERR_FILE_NOT_FOUND;
    }
    if ((inode_table[inode].flags & INODE_FLAG_DIRECTORY) == 1) {
        return ERR_IS_A_DIRECTORY;
    }

    unsigned int filesize = inode_table[inode].filesize;
    unsigned int bytes_to_read = len < filesize - offset ? len : filesize - offset;
    unsigned char temp[BYTES_PER_SECTOR];

    if (inode_table[inode].flags & INODE_FLAG_INDIRECT) {
        // Indirect case
        inode_indirect_t indirect[IINODES_PER_SECTOR];
        _fs_read_sector(inode_table[inode].sector, (unsigned char *)indirect);
        int src_block = fs_block_from_pos(offset);
        offset = fs_offset_from_pos(offset);
        // Read up to the end of the sector. Caller might need to try again.
        bytes_to_read = bytes_to_read < BYTES_PER_SECTOR - offset ? bytes_to_read : BYTES_PER_SECTOR - offset;
        if (indirect[src_block].sector == 0) {
            // Zero sector means this is a hole in the file, so return 0s.
            bzero(buf, bytes_to_read);
        } else {
            _fs_read_sector(indirect[src_block].sector, temp);
            bcopy(buf, temp + offset, bytes_to_read);
        }
    } else {
        // Direct case
        _fs_read_sector(inode_table[inode].sector, temp);
        bcopy(buf, temp + offset, bytes_to_read);
    }

    return bytes_to_read;
}

int fs_link(char * src, char * dst) {
    path_info_t src_info;
    int ret = fs_resolve_path(src, &src_info);
    if (ret < 0) {
        return ret;
    }
    if (src_info.inode == 0) {
        return ERR_FILE_NOT_FOUND;
    }
    if (fs_is_dir(src_info.inode)) {
        return ERR_IS_A_DIRECTORY;
    }

    path_info_t dst_info;
    ret = fs_resolve_path(dst, &dst_info);
    if (ret < 0) {
        return ret;
    }
    if (dst_info.inode != 0) {
        return ERR_FILE_EXISTS;
    }

    dirent_t * dst_dir = _fs_load_dir(dst_info.parent_dir_inode);
    if (dst_dir == 0) {
        return ERR_FILE_NOT_FOUND;
    }

    ret = _fs_add_dirent(dst_info.parent_dir_inode, dst_dir, dst_info.filename, 0, src_info.inode);
    if (ret < 0) {
        return ret;
    }

    inode_table[src_info.inode].refcount += 1;

    _fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);
}

int fs_unlink(char * target) {
    path_info_t path_info;
    int ret = fs_resolve_path(target, &path_info);
    if (ret < 0) {
        return ret;
    }
    if (path_info.inode == 0) {
        return ERR_FILE_NOT_FOUND;
    }
    if (fs_is_dir(path_info.inode)) {
        return ERR_IS_A_DIRECTORY;
    }

    // Remove parent dir entry

    dirent_t * parent_dir = _fs_load_dir(path_info.parent_dir_inode);
    if (parent_dir == 0) {
        return ERR_FILE_NOT_FOUND;
    }

    int entries = inode_table[path_info.parent_dir_inode].filesize / sizeof(dirent_t);
    int i = path_info.index;
    bcopy((unsigned char *)&parent_dir[i], (unsigned char *)&parent_dir[i+1], (entries-i-1) * sizeof(dirent_t));
    bzero((unsigned char *)&parent_dir[entries-1], sizeof(dirent_t));

    inode_t * pi = &inode_table[path_info.parent_dir_inode];
    pi->filesize -= sizeof(dirent_t);
    pi->refcount -= 1;

    _fs_write_sector(pi->sector, (unsigned char *)parent_dir);

    // Decrement inode refcount and clean up if necessary

    inode_t * fi = &inode_table[path_info.inode];
    fi->refcount -= 1;
    if (fi->refcount > 0) {
        return fi->refcount;
    }

    if (inode_table[path_info.inode].flags & INODE_FLAG_INDIRECT) {
        // Indirect case
        inode_indirect_t indirect[IINODES_PER_SECTOR];
        _fs_read_sector(inode_table[path_info.inode].sector, (unsigned char *)indirect);
        for (int i = 0; i < IINODES_PER_SECTOR; i++) {
            if (indirect[i].sector > 0) {
                _fs_free_sector(indirect[i].sector);
            }
        }
    }

    _fs_free_sector(fi->sector);
    fi->sector = 0;
    fi->filesize = 0;
    fi->flags = 0;

    _fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);

    return 0;
}

int fs_rmdir(char * target) {
    path_info_t path_info;
    int ret = fs_resolve_path(target, &path_info);
    if (ret < 0) {
        return ret;
    }
    if (path_info.inode == 0) {
        return ERR_FILE_NOT_FOUND;
    }
    if (!fs_is_dir(path_info.inode)) {
        return ERR_NOT_A_DIRECTORY;
    }

    inode_t * di = &inode_table[path_info.inode];
    if (di->refcount > 2) {
        return ERR_DIRECTORY_NOT_EMPTY;
    }

    _fs_free_sector(di->sector);
    di->sector = 0;
    di->filesize = 0;
    di->flags = 0;

    // Remove parent dir entry

    dirent_t * parent_dir = _fs_load_dir(path_info.parent_dir_inode);
    if (parent_dir == 0) {
        return ERR_FILE_NOT_FOUND;
    }

    int entries = inode_table[path_info.parent_dir_inode].filesize / sizeof(dirent_t);
    int i = path_info.index;
    bcopy((unsigned char *)&parent_dir[i], (unsigned char *)&parent_dir[i+1], (entries-i-1) * sizeof(dirent_t));
    bzero((unsigned char *)&parent_dir[entries-1], sizeof(dirent_t));

    inode_t * pi = &inode_table[path_info.parent_dir_inode];
    pi->filesize -= sizeof(dirent_t);
    pi->refcount -= 1;

    _fs_write_sector(pi->sector, (unsigned char *)parent_dir);

    _fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);

    return 0;
}

void _fs_mkdev() {
    int dev_inode = fs_mkdir("/dev");
    dirent_t * dev_dir = _fs_load_dir(dev_inode);
    _fs_mk(dev_inode, dev_dir, "tty", INODE_FLAG_CHAR_DEVICE | VFILE_DEV_TTY);
    _fs_mk(dev_inode, dev_dir, "ptr", INODE_FLAG_CHAR_DEVICE | VFILE_DEV_PTR);
    _fs_mk(dev_inode, dev_dir, "hd", INODE_FLAG_BLOCK_DEVICE | VFILE_DEV_HD);
    _fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);
    fs_mkdir("/bin");
}

int fs_stat(int inode, stat_t * stat) {
    stat->inode = inode;
    stat->filesize = fs_filesize(inode);
    stat->refcount = inode_table[inode].refcount;
    stat->flags = inode_table[inode].flags;
    return 0;
}

int fs_stat_path(char * path, stat_t * stat) {
    path_info_t path_info;
    int ret = fs_resolve_path(path, &path_info);
    if (ret < 0) {
      return ret;
    }
    return fs_stat(path_info.inode, stat);
}

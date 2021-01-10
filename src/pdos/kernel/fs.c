#include "fs.h"
#include "rk.h"
#include "stdlib.h"

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

inode_t inode_table[INODES_PER_SECTOR];
dirent_t root_dir[DIRENTS_PER_SECTOR];
dirent_t dir[BYTES_PER_SECTOR];

int fs_init() {
}

int fs_mount() {
	_fs_read_sector(INODE_TABLE, (unsigned char *)inode_table);
	_fs_read_sector(inode_table[ROOT_DIR_INODE].sector, (unsigned char *)root_dir);
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

	// Initialize the root dir inode
	inode_table[0].sector = ROOT_DIR_SECTOR;
	inode_table[0].filesize = 2 * sizeof(dirent_t);
	inode_table[0].hard_refcount = 2;
	inode_table[0].flags = INODE_FLAG_DIRECTORY;
	_fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);
	for (int i = 1; i < INODE_TABLE_SIZE; i++) {
		_fs_write_sector(INODE_TABLE+i, buf);
	}

	// Initialize the root directory table
	root_dir[0].inode = 0;
	root_dir[0].filename[0] = '.';

	root_dir[1].inode = 0;
	root_dir[1].filename[0] = '.';
	root_dir[1].filename[1] = '.';

	_fs_write_sector(ROOT_DIR_SECTOR, (unsigned char *)root_dir);

	return ROOT_DIR_INODE;
}

int fs_is_dir(int inode) {
	if (inode_table[inode].hard_refcount == 0) {
		return 0;
	}		
	if ((inode_table[inode].flags & INODE_FLAG_DIRECTORY) == 0) {
		return 0;
	}
	return 1;
}

int _fs_allocate_sector() {
	unsigned char buf[BYTES_PER_SECTOR];
	_fs_read_sector(FREE_SECTOR_MAP, buf);
	int i;
	for (i = 0; i < BYTES_PER_SECTOR; i++) {
		if (buf[i] != 0xff) break;
	}
	char m = buf[i];
	int j;
	for (j = 0; j < 8; j++) {
		if ((m & 1) == 0) break;
		m >>= 1;
	}
	buf[i] |= 1 << j;
	_fs_write_sector(FREE_SECTOR_MAP, buf);

	return (i << 3) | j;
}

dirent_t * _fs_load_dir(int dir_inode) {
	if (dir_inode == ROOT_DIR_INODE)	{
		return root_dir;
	} else {
		if (!fs_is_dir(dir_inode)) {
			return 0;
		}
		_fs_read_sector(inode_table[dir_inode].sector, (unsigned char *)dir);
	 	return dir;
	}
}

int _fs_find_inode(int parent_dir_inode, dirent_t * parent_dir, char * filename) {
	int dirent_count = inode_table[parent_dir_inode].filesize / sizeof(dirent_t);
	for (int i = 0; i < dirent_count; i++) {
		if (strncmp(parent_dir[i].filename, filename, sizeof(parent_dir[i].filename)) == 0) {
			return parent_dir[i].inode;
		}
	}
	return -1;
}

int fs_find_inode(int parent_dir_inode, char * filename) {
	dirent_t * parent_dir = _fs_load_dir(parent_dir_inode);
	if (parent_dir == 0) {
		return -1;
	}
	return _fs_find_inode(parent_dir_inode, parent_dir, filename);
}

int _fs_mk(int parent_dir_inode, dirent_t * parent_dir, char * filename, unsigned char flags) {
	// Check if the file already exists
	int existing_inode = _fs_find_inode(parent_dir_inode, parent_dir, filename);
	if (existing_inode >= 0) {
		return -1;
	}

	// Find an empty slot in the inode table
	int new_inode;
	for (new_inode = parent_dir_inode; new_inode < INODES_PER_SECTOR; new_inode++) {
		if (inode_table[new_inode].hard_refcount == 0) break;
	}
	// Find an empty slot in the parent directory table
	int next_dirent_idx = inode_table[parent_dir_inode].filesize / sizeof(dirent_t);

	parent_dir[next_dirent_idx].inode = new_inode;
	strncpy(parent_dir[next_dirent_idx].filename, filename, sizeof(parent_dir[next_dirent_idx].filename));

	// Create the new inode
	bzero((unsigned char *)&(inode_table[new_inode]), sizeof(inode_t));
	inode_table[new_inode].hard_refcount = 1;
	inode_table[new_inode].flags = flags;

	// Update parent inode
	inode_table[parent_dir_inode].filesize += sizeof(dirent_t);
	inode_table[parent_dir_inode].hard_refcount += 1;

	_fs_write_sector(inode_table[parent_dir_inode].sector, (unsigned char *)parent_dir);

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
		return -1;
	}
	int new_inode = _fs_mk(parent_dir_inode, parent_dir, filename, 0);
	_fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);
	return new_inode;
}

int fs_mkdir(int parent_dir_inode, char * dirname) {
	dirent_t * parent_dir = _fs_load_dir(parent_dir_inode);
	if (parent_dir == 0) {
		return -1;
	}
	int new_inode = _fs_mk(parent_dir_inode, parent_dir, dirname, INODE_FLAG_DIRECTORY);
	inode_table[new_inode].filesize = 2 * sizeof(dirent_t);
	inode_table[new_inode].hard_refcount = 2;

	dirent_t dir[DIRENTS_PER_SECTOR];
	bzero((unsigned char *)dir, BYTES_PER_SECTOR);

	dir[0].inode = new_inode;
	dir[0].filename[0] = '.';

	dir[1].inode = parent_dir_inode;
	dir[1].filename[0] = '.';
	dir[1].filename[1] = '.';	

	_fs_write_sector(inode_table[new_inode].sector, (unsigned char *)dir);
	_fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);

	return new_inode;
}

int fs_write(int inode, unsigned char * buf, int len, int offset) {
	if (inode_table[inode].hard_refcount == 0) {
		return -1;
	}
	if (fs_is_dir(inode)) {
		return -2;
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
	if (inode_table[inode].hard_refcount == 0) {
		return -1;
	}
	if (inode_table[inode].sector == 0) {
		return -2;
	}
	if ((inode_table[inode].flags & INODE_FLAG_DIRECTORY) == 1) {
		return -3;
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
		_fs_read_sector(indirect[src_block].sector, temp);
		bcopy(buf, temp + offset, bytes_to_read);
	} else {
		// Direct case
		_fs_read_sector(inode_table[inode].sector, temp);
		bcopy(buf, temp + offset, bytes_to_read);
	}

	return bytes_to_read;
}

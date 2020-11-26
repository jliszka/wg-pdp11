#include "rk.h"
#include "stdlib.h"
#include "fs.h"

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

#define INODE_FLAG_INDIRECT 1
#define INODE_FLAG_DIRECTORY 2
#define INODE_FLAG_EXECUTABLE 3

#define BYTES_PER_SECTOR 512
#define MAX_SECTOR 4096
#define INODES_PER_SECTOR (BYTES_PER_SECTOR / sizeof(inode_t))
#define IINODES_PER_SECTOR (BYTES_PER_SECTOR / sizeof(inode_indirect_t))
#define DIRENTS_PER_SECTOR (BYTES_PER_SECTOR / sizeof(dirent_t))

#define BOOT_SECTOR 0
#define FREE_SECTOR_MAP 1
#define INODE_TABLE 2
#define INODE_TABLE_SIZE 5
#define ROOT_DIR_SECTOR 7
#define ROOT_DIR_INODE 0

inode_t inode_table[INODES_PER_SECTOR];
dirent_t root_dir[DIRENTS_PER_SECTOR];
dirent_t dir[BYTES_PER_SECTOR];

#define _fs_read_sector(sector, buf) rk_read(sector, buf, BYTES_PER_SECTOR)
#define _fs_write_sector(sector, buf) rk_write(sector, buf, BYTES_PER_SECTOR) 

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
	
	_fs_write_sector(BOOT_SECTOR, buf);

	buf[0] = 0xff; // first 8 sectors not available
	_fs_write_sector(FREE_SECTOR_MAP, buf);
	buf[0] = 0;

	inode_table[0].sector = ROOT_DIR_SECTOR;
	inode_table[0].filesize = 2 * sizeof(dirent_t);
	inode_table[0].hard_refcount = 2;
	inode_table[0].flags = INODE_FLAG_DIRECTORY;
	_fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);
	for (int i = 1; i < INODE_TABLE_SIZE; i++) {
		_fs_write_sector(INODE_TABLE+i, buf);
	}

	root_dir[0].inode = 0;
	root_dir[0].filename[0] = '.';

	root_dir[1].inode = 0;
	root_dir[1].filename[0] = '.';
	root_dir[1].filename[1] = '.';

	_fs_write_sector(ROOT_DIR_SECTOR, (unsigned char *)root_dir);

	return ROOT_DIR_INODE;
}

int fs_is_dir(int inode) {
	if (inode_table[inode].sector == 0) {
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
	return 0;
}

int fs_find_inode(int parent_dir_inode, char * filename) {
	dirent_t * parent_dir = _fs_load_dir(parent_dir_inode);
	if (parent_dir == 0) {
		return -1;
	}
	return _fs_find_inode(parent_dir_inode, parent_dir, filename);
}

int _fs_mk(int parent_dir_inode, dirent_t * parent_dir, char * filename, unsigned char flags) {
	int existing_inode = _fs_find_inode(parent_dir_inode, parent_dir, filename);
	if (existing_inode != 0) {
		return -1;
	}

	int new_inode;
	for (new_inode = parent_dir_inode; new_inode < INODES_PER_SECTOR; new_inode++) {
		if (inode_table[new_inode].sector == 0) break;
	}
	int next_dirent_idx = inode_table[parent_dir_inode].filesize / sizeof(dirent_t);

	parent_dir[next_dirent_idx].inode = new_inode;
	strncpy(parent_dir[next_dirent_idx].filename, filename, sizeof(parent_dir[next_dirent_idx].filename));

	bzero((unsigned char *)&(inode_table[new_inode]), sizeof(inode_t));
	inode_table[new_inode].sector = _fs_allocate_sector();
	inode_table[new_inode].hard_refcount = 1;
	inode_table[new_inode].flags = flags;
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

int fs_write(int inode, unsigned char * buf, int len) {
	if (inode_table[inode].sector == 0) {
		return -1;
	}		
	if ((inode_table[inode].flags & INODE_FLAG_DIRECTORY) == 1) {
		return -2;
	}

	inode_table[inode].filesize = len;
	
	// TODO: check if len > sector size

	rk_write(inode_table[inode].sector, buf, len);
	_fs_write_sector(INODE_TABLE, (unsigned char *)inode_table);

	return len;
}

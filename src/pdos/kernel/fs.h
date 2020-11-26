#ifndef FS_H
#define FS_H

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

// 8 bytes. 512/8 = 64 inodes per sector
typedef struct {
	unsigned int sector;
	unsigned int filesize;
	unsigned char hard_refcount;
	unsigned char soft_refcount;
	unsigned char flags;
	unsigned char unused;
} inode_t;

// 8 bytes. 512/4 = 128 indirect references per sector
// max filesize = 128 * 512 = 65,536 bytes (fits in `int filesize`)
typedef struct {
	unsigned int sector;
	unsigned int unused;
} inode_indirect_t;

// 16 bytes. 512/16 = 32 files per sector
typedef struct {
	unsigned int inode;
	char filename[14];
} dirent_t;

int fs_init();
int fs_mkfs();
int fs_mount();
int fs_find_inode(int parent_dir_inode, char * filename);
int fs_touch(int parent_dir_inode, char * filename);
int fs_mkdir(int parent_dir_inode, char * dirname);
int fs_write(int inode, unsigned char * buf, int len);
int fs_read_dir(int dir_inode, int buflen, dirent_t * buf);
int fs_is_dir(int inode);

#endif

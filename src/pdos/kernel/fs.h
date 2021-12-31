#ifndef FS_H
#define FS_H

#include "fs_defs.h"

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

int fs_init();
int fs_mkfs();
int fs_mount();
int fs_find_inode(int parent_dir_inode, char * filename);
int fs_resolve_path(char * path, path_info_t * path_info);
int fs_touch(int parent_dir_inode, char * filename);
int fs_mkdir(char * dirname);
int fs_write(int inode, unsigned char * buf, int len, int offset);
int fs_read(int inode, unsigned char * buf, int len, int offset);
int fs_read_dir(int dir_inode, int buflen, dirent_t * buf);
int fs_is_dir(int inode);
int fs_is_device(int inode, int * device_type);
int fs_filesize(int inode);
int fs_link(char * src, char * dst);
int fs_unlink(char * target);
int fs_rmdir(char * dirname);

#define fs_read_block(inode, blockno, dst) \
	fs_read(inode, dst, BYTES_PER_SECTOR, fs_pos_from_block(blockno))

#define fs_write_block(inode, blockno, src) \
	fs_write(inode, src, BYTES_PER_SECTOR, fs_pos_from_block(blockno))

#endif

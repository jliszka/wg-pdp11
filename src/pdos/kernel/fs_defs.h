#ifndef FS_DEFS_H
#define FS_DEFS_H

#include "rk.h"

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

#define INODE_FLAG_INDIRECT 1
#define INODE_FLAG_DIRECTORY 2
#define INODE_FLAG_CHAR_DEVICE 0x10
#define INODE_FLAG_BLOCK_DEVICE 0x20

#define VFILE_DEV_TTY 0
#define VFILE_DEV_PTR 1
#define VFILE_DEV_HD 2
#define VFILE_NUM_DEVICES 3

#define BYTES_PER_SECTOR 512
#define BYTES_PER_SECTOR_SHIFT 9
#define BYTES_PER_SECTOR_MASK 0x1ff
#define MAX_SECTOR 4096
#define INODES_PER_SECTOR (BYTES_PER_SECTOR / sizeof(inode_t))
#define IINODES_PER_SECTOR (BYTES_PER_SECTOR / sizeof(inode_indirect_t))
#define DIRENTS_PER_SECTOR (BYTES_PER_SECTOR / sizeof(dirent_t))

#define BOOT_SECTOR 0
#define FREE_SECTOR_MAP 1
#define INODE_TABLE 2
#define INODE_TABLE_SIZE 5
#define ROOT_DIR_SECTOR 7
#define ROOT_DIR_INODE 1

#define _fs_read_sector(sector, buf) rk_read(sector, buf, BYTES_PER_SECTOR)
#define _fs_write_sector(sector, buf) rk_write(sector, buf, BYTES_PER_SECTOR) 

#define fs_block_from_pos(pos) ((pos) >> BYTES_PER_SECTOR_SHIFT)
#define fs_pos_from_block(blockno) ((blockno) << BYTES_PER_SECTOR_SHIFT)
#define fs_offset_from_pos(pos) ((pos) & BYTES_PER_SECTOR_MASK)

typedef struct {
	unsigned int sector;
	unsigned int filesize;
	unsigned char refcount;
	unsigned char flags;
	unsigned char unused1;
	unsigned char unused2;
} inode_t;

// 4 bytes. 512/4 = 128 indirect references per sector
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

typedef struct {
  unsigned int parent_dir_inode;
  unsigned int inode;
  unsigned int index;
  char * filename;
} path_info_t;

typedef struct {
    unsigned int inode;
    unsigned int filesize;
    unsigned char refcount;
    unsigned char flags;
} stat_t;

#endif

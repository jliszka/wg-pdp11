#include "fs_defs.h"

// This is pinned as the first byte of the program. It is overwritten by
// the mbr command when it copies this program to the boot sector of the disk.
extern int kernel_inode;

int fs_read(inode_t * inode_table, int inode, unsigned char * buf, int len, int offset);

// A minimal program to find the kernel on disk, load it into memory, and start executing it.
// This must fit into one disk sector (512 bytes).
void main() {
	inode_t inode_table[INODES_PER_SECTOR];

	_fs_read_sector(INODE_TABLE, (unsigned char *)inode_table);
	
	int start_address = 0;
	int pos = 0;
	
	while (1) {

        // 1. Read in the header. It consists of 3 ints:
        // header[0] = 1
        // header[1] = number of bytes
        // header[2] = start address to load the program to

        int header[3];
        pos += fs_read(inode_table, kernel_inode, (unsigned char *)header, 6, pos);
    
        // 2. Copy the bytes to the destination address

        int byte_count = header[1];
        if (byte_count == 6) {
            // Empty block means we are done.
            break;
        }

        int address = header[2];
        if (start_address == 0) {
            start_address = address;
        }

        int end = pos + byte_count - 6;
        unsigned char * dst = (unsigned char *)(address - pos);
        do {
            pos += fs_read(inode_table, kernel_inode, dst + pos, end - pos, pos);
        } while (pos < end);

        // 3. TODO: validate the checksum. skip for now.
        pos++;
	}

	// 4. Run the kernel program.
	((void (*)(void))start_address)();
}

int fs_read(inode_t * inode_table, int inode, unsigned char * buf, int len, int offset) {
	unsigned int filesize = inode_table[inode].filesize;
	unsigned int bytes_to_read = len < filesize - offset ? len : filesize - offset;
	unsigned char temp[BYTES_PER_SECTOR];
	int sector = inode_table[inode].sector;

	if (inode_table[inode].flags & INODE_FLAG_INDIRECT) {
		// Indirect case
		inode_indirect_t indirect[IINODES_PER_SECTOR];
		_fs_read_sector(sector, (unsigned char *)indirect);
		int src_block = offset >> BYTES_PER_SECTOR_SHIFT;
		offset &= BYTES_PER_SECTOR_MASK;
		// Read up to the end of the sector. Caller might need to try again.
		bytes_to_read = bytes_to_read < BYTES_PER_SECTOR - offset ? bytes_to_read : BYTES_PER_SECTOR - offset;
		sector = indirect[src_block].sector;
	}

	_fs_read_sector(sector, temp);

	for (int i = 0; i < bytes_to_read; i++) {
		buf[i] = temp[i + offset];
	}

	return bytes_to_read;
}

#ifndef DISK_H_
#define DISK_H_

#include <stdint.h>
//#include "types.h"

#define BITMAP_SIZE			(512 * 256)
#define BITMAP_USED_BYTE 	32
#define DIR_SIZE			512

union Byte
{
	struct
	{
		uint8_t bit0:	1;
		uint8_t bit1:	1;
		uint8_t bit2:	1;
		uint8_t bit3:	1;
		uint8_t bit4:	1;
		uint8_t bit5:	1;
		uint8_t bit6:	1;
		uint8_t bit7:	1;
	};
	uint8_t byte;
};

struct map
{
	union Byte mask[BITMAP_SIZE];
};
//map总共占2^9 * 2^8 = 2^17个字节
//map总共占2^9 * 2^8 * 2^3 = 2^20 = 1 Mbit
//map总共占2^9 * 2^8 / 2^9 = 2^8(256)个块.

//每一个bit标示一个块(512B),总共512M.
//在bitmap中，它自己用了256个bit，2^8 / 2^3 = 2^5 = 32 B.

struct dirent
{
	char 	 filename[24];
	uint32_t file_size;
	uint32_t inode_offset;
};	//每个目录项占32字节

//只有一个块(512B)大小的根目录，包含512/32 = 16个文件项
struct dir
{
	struct dirent entry[512 / sizeof(struct dirent)];
}; //sizeof(dir) == 512, nr_entries == 16

#define NR_DIR_ENTRY	(512 / sizeof(struct dirent))
#define NR_INODE_ENTRY	(512 / sizeof(uint32_t))

struct iNode
{
	uint32_t data_block_offset[NR_INODE_ENTRY];
}; 
//一个inode为512B,可以存储512 / 4 = 128字节数据(表项)。
//每个表项指一个数据块
#endif

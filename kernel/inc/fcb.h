#ifndef __FCB_H__
#define __FCB_H__

#include "disk.h"

#define s_nonexist	-1
#define s_r		0
#define s_w  	1
#define s_rw	2
#define s_close 3

struct FCB {
	int f_state;			//文件状态
	struct iNode inode;
	int offset;			//文件指针的字节偏移
	int count;
};

struct map bitmap;
struct dir direct;			//这两项必须在加载文件之前就从磁盘读出

//#define ACTIVE_INODE 	256
//struct iNode active_inode[ACTIVE_INODE];

void file_init();
int file_alloc();

int open(char*, int);
int read(int, char*, int);
int write(int, char*, int);
int lseek(int, int, int);
int close(int);

#endif

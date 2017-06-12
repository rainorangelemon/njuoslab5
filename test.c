#include "disk.h"
#include <stdlib.h>
#include <stdio.h>
#include "boot/inc/boot.h"

#define SECTSIZE 512

struct map bitmap;
struct dir direct;

char buf[512];

void main() {
	struct ELFHeader elf;
	struct ProgramHeader *ph, *eph;
	unsigned char* pa, *i;

	/* 因为引导扇区只有512字节，我们设置了堆栈从0x8000向下生长。
	 * 我们需要一块连续的空间来容纳ELF文件头，因此选定了0x8000。 */
	FILE *diskfp;	
	diskfp=fopen("disk.bin","rb");
	fseek(diskfp,512,SEEK_SET);	
	fread(&bitmap, 512*256,1,diskfp);
	fread(&direct, 512,1,diskfp);
	/* 读入ELF文件头 */
	int j;	
	for(j=0;j<16;j++){
		if(direct.entry[j].filename[2]=='r')
			break;
	}
	j=0;
	struct iNode inode;
	int inode_offset=direct.entry[j].inode_offset;
	fseek(diskfp,512+inode_offset * 512,SEEK_SET);
	fread((uint8_t*)&inode, 512,1, diskfp);
	int elf_offset = inode.data_block_offset[0];
	printf("elf_offset:%d\n",elf_offset);
	fseek(diskfp,512+elf_offset*512,SEEK_SET);
	fread((unsigned char*)&elf, 4096,1,diskfp);
	printf("here?\n");
/*	ph = (struct ProgramHeader*)((char *)elf + elf.phoff);
	eph = ph + elf->phnum;
	for(; ph < eph; ph ++) {
		pa = (unsigned char*)ph->paddr; 
		fseek(diskfp,512+ph->off+ elf_offset * 512,SEEK_SET);
		fread(pa, ph->filesz,1, diskfp);
		for (i = pa + ph->filesz; i < pa + ph->memsz; *i ++ = 0);
	}
*/
	printf("%x\n",elf.phoff);
	printf("%x\n",elf.entry);
	ph = (struct ProgramHeader*)((char *)&elf + elf.phoff);
	eph = ph + elf.phnum;
	printf("%x,phnum\n",elf.phnum);
	for(; ph < eph; ph ++) {
		printf("paddr:%x\n",ph->paddr); 
		printf("type:%x\n",ph->type);
		printf("filesz:%x\n",ph->filesz);
		printf("memzx:%x\n",ph->memsz);
	}
	fseek(diskfp,512+288 * 512,SEEK_SET);
	fread(buf, 512,1, diskfp);
	printf("the buf is\n\t%s\n\n", buf);
	fclose(diskfp);
	
	/*((void(*)(void))(elf->entry - 0xC0000000))();*/

	//while (1);
}

#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
#include <string.h>

struct map bitmap;

struct dir direct;

struct iNode inodes[32];

char data[512];

uint8_t boot[512];

int main()
{
	FILE *diskfp = fopen("disk.bin", "wb");
	if(diskfp == NULL)
	{
		printf("create disk error!\n");
		exit(0);
	}

	FILE *bootfp = fopen("boot.bin", "r");
	fread(&boot,512,1,bootfp);
	fclose(bootfp);

//-----------------init-------------------
	int i, j;
	for(i = 0; i < BITMAP_USED_BYTE; i ++)
	{
		bitmap.mask[i].byte = 0xff;
	}

	bitmap.mask[BITMAP_USED_BYTE].bit7 = 1;   /*it is for direct*/

	int dir_bitoffset = BITMAP_USED_BYTE * 8;
	for(j = 0; j < NR_DIR_ENTRY; j ++)
	{
		direct.entry[j].filename[0] = '\0';
		direct.entry[j].file_size = 0;
		direct.entry[j].inode_offset = -1;
	}
//----------------------------------------
	for(i=0;i<32;i++){
		for(j=0;j<NR_INODE_ENTRY;j++){
			inodes[i].data_block_offset[j]=-1;
		}
	}

//----------------write data ----------------
	for(i=0;i<32;i++){	
		fseek(diskfp, 512+dir_bitoffset * 512+512+32*512+i*128*512, SEEK_SET);
		fwrite(&data, 128, 512, diskfp);
	}
//----------------write inode ----------------
	fseek(diskfp, 512+dir_bitoffset * 512+512, SEEK_SET);
	fwrite(&inodes, 32, 512, diskfp);

//----------------write root direct----------------
	fseek(diskfp, 512+dir_bitoffset * 512, SEEK_SET);
	fwrite(&direct, 1, 512, diskfp);

//----------------write bitmap----------------
	fseek(diskfp, 512, SEEK_SET);
	fwrite(&bitmap, 256, 512, diskfp);

//----------------write boot----------------
	fseek(diskfp, 0, SEEK_SET);
	fwrite(&boot, 1, 512, diskfp);


	fclose(diskfp);

}

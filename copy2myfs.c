#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
#include <string.h>

struct map bitmap;

struct dir direct;

#define alloc_inode 1
#define alloc_data  2

void bit_alloc(int *bitoffset,int mode)
{
	int i;
	int base;
	if(mode==alloc_inode){
		base=BITMAP_USED_BYTE;
	}else if(mode==alloc_data){
		base=BITMAP_USED_BYTE+32/8;
	}
	for(i = base; i < BITMAP_SIZE; i ++)
	{
		if(bitmap.mask[i].byte != 0xff)
		{
			if(bitmap.mask[i].bit7 == 0)
			{
				bitmap.mask[i].bit7 = 1;
				*bitoffset = 0;
				break;
			}
			else if(bitmap.mask[i].bit6 == 0)
			{
				bitmap.mask[i].bit6 = 1;
				*bitoffset = 1;
				break;
			}
			else if(bitmap.mask[i].bit5 == 0)
			{
				bitmap.mask[i].bit5 = 1;
				*bitoffset = 2;
				break;
			}	
			else if(bitmap.mask[i].bit4 == 0)
			{
				bitmap.mask[i].bit4 = 1;
				*bitoffset = 3;
				break;
			}
			else if(bitmap.mask[i].bit3 == 0)
			{
				bitmap.mask[i].bit3 = 1;
				*bitoffset = 4;
				break;
			}
			else if(bitmap.mask[i].bit2 == 0)
			{
				bitmap.mask[i].bit2 = 1;
				*bitoffset = 5;
				break;
			}
			else if(bitmap.mask[i].bit1 == 0)
			{
				bitmap.mask[i].bit1 = 1;
				*bitoffset = 6;
				break;
			}
			else if(bitmap.mask[i].bit0 == 0)
			{
				bitmap.mask[i].bit0 = 1;
				*bitoffset = 7;
				break;
			}
		}
	}
	*bitoffset += i * 8;
}

int main(int argc, char *argv[])
{
	FILE *diskfp = fopen("disk.bin", "rb+");
	if(diskfp == NULL)
	{
		printf("create disk error!\n");
		exit(0);
	}
	fseek(diskfp,512,SEEK_SET);
	fread(&bitmap,1,sizeof(bitmap),diskfp);
	printf("%lu,%d\n",sizeof(bitmap),32*8*512);
	int i;
	for(i=0;i<BITMAP_USED_BYTE+4;i++){
		printf("%x\n",bitmap.mask[i].byte);
	}
	fseek(diskfp,512+sizeof(bitmap),SEEK_SET);
	fread(&direct,1,sizeof(direct),diskfp);
	

//----------------------------------------
	int k;
	int filesz;
	int bitoffset;
	int index;
	struct iNode inode;
	for(k = 1; k < argc; k ++)
	{
		FILE *fp = fopen(argv[k], "rb");
		if(fp == NULL)
		{
			printf("open failed!\n");
			exit(1);
		}
	//	printf("open success\n");
		fseek(fp, 0, SEEK_END);
		filesz = ftell(fp);
		fseek(fp, 0, SEEK_SET);
//----------------fill bitmap----------------
		bit_alloc(&bitoffset,alloc_inode);
		//this number is the file's inode bit index, counting from 0
		printf("this file's inode bit index is %d\n", bitoffset);

//--------------fill a direct entry--------------
		for(index = 0; index < NR_DIR_ENTRY; index ++)
		{
			if(direct.entry[index].inode_offset == -1)
			{
				memcpy(direct.entry[index].filename, argv[k], strlen(argv[k]));
				direct.entry[index].file_size = filesz;
				direct.entry[index].inode_offset = bitoffset;
				break;
			}
		}
		printf("entry_index:%d inode_offset:%d\n",index,bitoffset);
//printf("file %s's size is %d\n", argv[k], filesz);
	
//-------------write the data to disk block by block-------------
		//------init inode-------
		int inode_entry_index;
		for(inode_entry_index = 0; inode_entry_index <= NR_INODE_ENTRY; inode_entry_index ++)
			inode.data_block_offset[inode_entry_index] = -1;

		char buf[513] = "\0";
		int entry_index = 0;
		int inode_num=0;
		while(!feof(fp))
		{
			memset(buf, '\0', sizeof(buf));

			fread(buf, 1, 512, fp);
printf("the buf is\n\t%s\n\n", buf);
			int inode_entry = 0;
			bit_alloc(&inode_entry,alloc_data);
printf("the block's bit index for this buf is %d\n", inode_entry);
			if(entry_index < NR_INODE_ENTRY)
				inode.data_block_offset[entry_index]=inode_entry;
			else
			{
				fseek(diskfp, direct.entry[index].inode_offset * 512+inode_num*512+512, SEEK_SET);
				fwrite(&inode, 1, 512, diskfp);
				for(inode_entry_index = 0; inode_entry_index <= NR_INODE_ENTRY; inode_entry_index ++)
					inode.data_block_offset[inode_entry_index] = -1;
				bit_alloc(&bitoffset,alloc_inode);
				printf("this file's new inode bit index is %d\n", bitoffset);
				inode.data_block_offset[0]=inode_entry;	
				fseek(diskfp, inode_entry * 512+512, SEEK_SET);
				fwrite(buf, 1, 512, diskfp);
				entry_index=1;
				inode_num++;
				continue;
			}
//----------------write data------------------
			fseek(diskfp, inode_entry * 512+512, SEEK_SET);
			fwrite(buf, 1, 512, diskfp);
			entry_index ++;
		}
		fseek(diskfp, direct.entry[index].inode_offset * 512+inode_num*512+512, SEEK_SET);
		fwrite(&inode, 1, 512, diskfp);		
	}
	fseek(diskfp,512+direct.entry[index].inode_offset*512,SEEK_SET);
	fread(&inode,1,512,diskfp);
	printf("inode_data_offset:%d\n",inode.data_block_offset[0]);
	fseek(diskfp,512,SEEK_SET);
	fwrite(&bitmap,1,sizeof(bitmap),diskfp);
	fseek(diskfp,512+sizeof(bitmap),SEEK_SET);
	fwrite(&direct,1,sizeof(direct),diskfp);
printf("now, the directory is:\n");
for(i = 0; i < 16; i ++)
{
	printf("%s %d %d\n",direct.entry[i].filename, direct.entry[i].file_size, direct.entry[i].inode_offset);
}
	fflush(diskfp);
	fclose(diskfp);
}

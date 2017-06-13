#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "disk.h"

struct map bitmap;
struct dir direct;

int main(int argc, char *argv[])
{
	FILE *fp=fopen("disk.bin","rb");
	FILE *new_fp=fopen("read_files/readfile","wb");
	if(fp == NULL){
		printf("open failed\n");
        	exit(0);
	}
	fseek(fp, 512,SEEK_SET);
        fread(&bitmap, 1, sizeof(bitmap), fp);
    	fread(&direct, 1, sizeof(direct), fp);
//	int y;
//	for(y=0;y<BITMAP_USED_BYTE+4;y++){
//		printf("%x\n",bitmap.mask[y].byte);
//	}
    int i;
    char buf[513];
    for(i = 0; i < NR_DIR_ENTRY; i ++){
	 if((direct.entry[i].inode_offset != -1)&&(strcmp(direct.entry[i].filename,argv[1])==0)){
	    int size=0;
	    struct iNode inode;
	    fseek(fp, 512+direct.entry[i].inode_offset * 512, SEEK_SET);
	    fread(&inode, 1, 512, fp);
	    int j=0;
	    int inode_num=0;
	    while(size<direct.entry[i].file_size){
		if(j==NR_INODE_ENTRY){
			fseek(fp, 512+direct.entry[i].inode_offset * 512+inode_num*512, SEEK_SET);
	    		fread(&inode, 1, 512, fp);
			inode_num++;
			j=0;
		}
	        if(inode.data_block_offset[j] != -1){
	      		memset(buf, '\0', sizeof(buf));
                    	fseek(fp, 512+inode.data_block_offset[j] * 512, SEEK_SET);
			fread(buf, 1, 512, fp);
			fwrite(buf,1,512,new_fp);
			int k;
//			for(k=0;k<512;k++)
//				printf("%c", buf[k]);
		}else{
			break;
		}
		size=size+512;
		j++;
	    }
         }	
    }
    fclose(fp);
    return 0;
}

#include "types.h"
#include "string.h"
#include "fcb.h"
#include "pcb_struct.h"
#include "stdio.h"

#define MAX_FCB 256
#define MAX_ACT_INODE	256
#define NR_FILE	20
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

struct FCB fcb_table[MAX_FCB];	//the table of files the system opened
struct act_inode inode_table[MAX_ACT_INODE];	//the table of active inode

void readseg(unsigned char *, int, int);
void writeseg(unsigned char *, int, int);
void writesect(void *src, int offset);
extern struct map bitmap;
extern struct dir direct;
extern struct PCB *current;

void file_init()
{
	int i;
	for(i = 0; i < MAX_FCB; i ++)
	{
		fcb_table[i].f_state = s_close;
		fcb_table[i].act_inode_index = 0;
		fcb_table[i].inode_bitoffset = 0;
		fcb_table[i].ch_offset = 0;
		fcb_table[i].buf_offset = 0;
	}
}

int file_alloc()
{
	int fcb_index = -1;
	for(int i= 0; i < MAX_FCB; i ++)
	{
		if(fcb_table[i].f_state == s_close)
		{
			fcb_index = i;
			break;
		}
	}
	return fcb_index;
}

void inode_init()	//init inode table
{
	for(int i = 0; i < MAX_ACT_INODE; i ++)
	{
		inode_table[i].i_count = 0;
		for(int j = 0; j < NR_INODE_ENTRY; j ++)
			inode_table[i].data.data_block_offset[j] = -1;
	}
}

int inode_alloc()	//alloc active inode
{
	for(int i = 0; i < MAX_ACT_INODE; i ++)
	{
		if(inode_table[i].i_count == 0)
		{
			return i;
		}
	}
	return -1;		//no more inode be allocated
}

int open(char *filename, int mode)
{
	bool find = false;
	int i;
	for(i = 0; i < NR_DIR_ENTRY; i ++)
	{
		if(strcmp(filename, direct.entry[i].filename) == 0)
		{
			find = true;
			printk("I HAVE FOUND THE FILE\n");
			break;
		}
	}
	//find the file in the disk
	if(find)
	{
		//check whether this file is already in the memory
		int inode_offset = direct.entry[i].inode_offset;
		int flag=0;
		int flag1=0;		
		for(i=0;i<MAX_FCB;i++){
			if((fcb_table[i].f_state==mode)&&(fcb_table[i].inode_bitoffset==inode_offset)){
				flag=1;
				break;
			}
		}
		int index, fcb_index, fd = 0;
		if(flag==0){  //no this file in the memory
			index = inode_alloc();
			printk("the active inode index is %d\n", index);
			struct iNode inode;
			readseg((uint8_t*)&inode,512,512*inode_offset);
			inode_table[index].data=inode;
			//		for(int i = 0; i < NR_INODE_ENTRY; i ++)
			//		{
			//			if(inode_table[index].data.data_block_offset[i] != -1)
						printk("%d\n", inode_table[index].data.data_block_offset[0]);
			//		}

			inode_table[index].i_count += 1;
			//no priviledge check
			//allocate a fcb for this file in the kernel
			fcb_index = file_alloc();
			printk("the fcb index is %d\n", fcb_index);
			fcb_table[fcb_index].f_state = mode;
			fcb_table[fcb_index].act_inode_index = index;
			fcb_table[fcb_index].inode_bitoffset = inode_offset;
			fcb_table[fcb_index].ch_offset = 0;
		}else{
			for(fd=0;fd<NR_FILE;fd++){
				if(current->fcb[fd]!=-1){
					if(current->fcb[fd]==i){
						printk("you have already opened this file!");
						current->tf->eax=i;
						flag1=1;
						break;
					}
				}
			}
			fcb_index=i;
		}
		
		//alloc a fcb for process
		if(flag1!=1){
			for(fd = 0; fd < NR_FILE; fd++)
			{
				if(current->fcb[fd] == -1)
				{
					current->fcb[fd] = fcb_index;
					current->tf->eax = fd;
					inode_table[fcb_table[fcb_index].act_inode_index].i_count += 1;
					printk("the fd is %d\n", fd);
					break;
				}
			}
		}

	}else{
		printk("ERROR:No such file in derectory!\n");
		current->tf->eax = -1;	//if no file, then return -1
	}
	return current->tf->eax;
}

int read(int fd, char *buf, int count)
{
	int fcb_index = current->fcb[fd];
	if((!((fcb_table[fcb_index].f_state==s_r)||(fcb_table[fcb_index].f_state==s_rw)))||(fcb_index==-1)){
			printk("ERROR:No such fd!\n");
			printk("%d\n",fcb_index);
			current->tf->eax=-1;
	}
	else{
	int inode_index = fcb_table[fcb_index].act_inode_index;
	int ch_offset = fcb_table[fcb_index].ch_offset;
	char temp_buf[513];
	memset(temp_buf, 0, sizeof(temp_buf));
	
	int nr = ch_offset / 512;
	int start = ch_offset % 512;
	int total = ch_offset + count;
	int num = total / 512;
	int secno = num - nr;

	/*Watch out ! I do not handle the problem if the ch_offset is bigger than the file size!!!*/

	if(secno == 0){
		readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr] * 512);
		int len = count < sizeof(temp_buf) - start ? count : sizeof(temp_buf) - start;
		strncpy(buf, temp_buf + start, len);
		
		fcb_table[fcb_index].ch_offset = total;
		current->tf->eax = len;
	}else{
		readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr] * 512);
		strncpy(buf, temp_buf + start, 512 - start);
		for(int i = nr + 1; i < num; i ++)
		{
			readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[i] * 512);
			strcpy(buf + 512-start + (i-nr-1)*512, temp_buf);
		}
		readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[num] * 512);
		strncpy(buf + 512-start + (secno-1) * 512  , temp_buf, total - num * 512);

		fcb_table[fcb_index].ch_offset = total;
		current->tf->eax = total - nr * 512 - start;
	}
	}
	return current->tf->eax;
}

int write(int fd, char *buf, int count)
{
	int fcb_index = current->fcb[fd];
	if((!((fcb_table[fcb_index].f_state==s_w)||(fcb_table[fcb_index].f_state==s_rw)))||(fcb_index==-1)){
			printk("ERROR:No such fd!\n");
			current->tf->eax=-1;
	}
	else{
	int inode_index = fcb_table[fcb_index].act_inode_index;
	int ch_offset = fcb_table[fcb_index].ch_offset;
	
	int nr = ch_offset / 512;
	int start = ch_offset % 512;	//this buf should be in the nr th block
	int num = (ch_offset + count) / 512;
	int end = (ch_offset +count) % 512;

	char temp_buf[513];
	int rest = 512 - start;
	if(rest > count)
	{
		readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr]*512);
		strncpy(temp_buf+start, buf, count);
		writeseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr]*512);
	}else if(count - rest < 512){
		readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr]*512);
		strncpy(temp_buf+start, buf, rest);
		writeseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr]*512);
		
		readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr + 1]*512);
		strncpy(temp_buf, buf+rest, count - rest);
		writeseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr + 1]*512);
	}else{
		readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr]*512);
		strncpy(temp_buf+start, buf, rest);
		writeseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[nr]*512);
	
		for(int i = nr+1; i < num; i ++){
			readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[i]*512);
			strncpy(temp_buf, buf+rest + (i-nr-1)*512, 512);
			writeseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[i]*512);
		}

		readseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[num]*512);
		strncpy(temp_buf, buf+rest+(num-nr-1)*512, end);
		writeseg((uint8_t *)temp_buf, 512, inode_table[inode_index].data.data_block_offset[num]*512);
	}
		fcb_table[fcb_index].ch_offset+=count;
		current->tf->eax = count;
	}
	return current->tf->eax;
}

int lseek(int fd, int offset, int whence)
{
	int fcb_index = current->fcb[fd];
	if(fcb_index==-1){
		current->tf->eax=-1;
		return current->tf->eax;
	}
	int inode_index = fcb_table[fcb_index].act_inode_index;
	int i = 0;
	switch(whence){
		case SEEK_SET:
			fcb_table[fcb_index].ch_offset = offset;
			break;
		case SEEK_CUR:
			fcb_table[fcb_index].ch_offset += offset;
			break;
		case SEEK_END:
			fcb_table[fcb_index].ch_offset = 0;
			for(; i < NR_INODE_ENTRY; i ++){
				if(inode_table[inode_index].data.data_block_offset[i] != -1)
					fcb_table[fcb_index].ch_offset += 512;
				else
					break;
			}
			fcb_table[fcb_index].ch_offset -= 512;
			i -= 1;
			char buf[514];
			readseg((uint8_t *)buf, 512, inode_table[inode_index].data.data_block_offset[i] * 512);
			fcb_table[fcb_index].ch_offset += (strlen(buf) + offset);
			break;
	}
	current->tf->eax=1;
	return current->tf->eax;
}

int close(int fd)
{
	int fcb_index = current->fcb[fd];
	if(fcb_index==-1){
		current->tf->eax=-1;
		return current->tf->eax;
	}
	current->fcb[fd] = -1;
	int inode_index = fcb_table[fcb_index].act_inode_index;
	inode_table[inode_index].i_count -= 1;
	if(inode_table[inode_index].i_count==0){
		fcb_table[fcb_index].f_state = s_close;
		fcb_table[fcb_index].act_inode_index = 0;
		fcb_table[fcb_index].inode_bitoffset = 0;
		fcb_table[fcb_index].ch_offset = 0;
	}
	current->tf->eax=1;
	return current->tf->eax;
}

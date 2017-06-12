#include "common.h"
#include "timer.h"
#include "syscall.h"

#define KEYCODE 0
#define VMEMORY 1
#define WRITE   2
#define TIME    3
#define GETPID  4
#define FORK 	5
#define SLEEP 	6
#define EXIT 	7
#define THREAD	8
#define SEM_INIT	9
#define SEM_WAIT	10
#define SEM_POST	11
#define SEM_DESTROY	12
#define SEM_TRYWAIT	13
#define FILE_OPEN 	14
#define FILE_READ	15
#define FILE_WRITE	16
#define FILE_LSEEK	17
#define FILE_CLOSE	18
 
static inline int syscall(int id, ...){
	int ret;
	int *args=&id;
	asm volatile("int $0x80":"=a"(ret):"a"(args[0]),"b"(args[1]),"c"(args[2]),"d"(args[3]));
	return ret;
}

int str_write(int fd,char *buf,int len){
	return syscall(WRITE,fd,buf,len);
}

int get_time(){
	return syscall(TIME);
}

int get_kbd(){
	return syscall(KEYCODE);
}

int put_video(uint8_t *buffer){
	return syscall(VMEMORY,buffer);
}

int fork(){
	return syscall(FORK);
}

void sleep(uint32_t time){
	syscall(SLEEP,time);
}

void exit(){
	syscall(EXIT);
}

int getpid(){
	return syscall(GETPID);
}

void sem_init(sem_t *sem, int value, bool mutex){
	syscall(SEM_INIT,sem,value,mutex);
}

void sem_wait(sem_t *sem){
	syscall(SEM_WAIT,sem);
}

void sem_post(sem_t *sem){
	syscall(SEM_POST,sem);
}

void sem_destroy(sem_t *sem){
	syscall(SEM_DESTROY,sem);
}

int sem_trywait(sem_t *sem){
	return syscall(SEM_TRYWAIT,sem);
}

void create_thread(uint32_t *func_addr){
	syscall(THREAD,func_addr);
}

int open(char *filename, int mode){
	return syscall(FILE_OPEN, (void *)filename, mode);
}

int read(int fd, char *buf, int count){
	return syscall(FILE_READ, fd, (void *)buf, count);
}

int write(int fd, char *buf, int count){
	return syscall(FILE_WRITE, fd, (void *)buf, count);
}

int lseek(int fd, int offset, int whence)
{
	return syscall(FILE_LSEEK, fd, offset, whence);
}

int close(int fd){
	return syscall(FILE_CLOSE, fd);
}

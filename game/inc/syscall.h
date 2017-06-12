#include "semaphore_struct.h"
#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define HZ 1000

int str_write(int, char*, int);
int get_time();
int get_kbd();
int put_video(uint8_t*);
int fork();
void sleep(uint32_t);
void exit();
int getpid();
void sem_init(sem_t *, int, bool);
void sem_wait(sem_t *sem);
void sem_post(sem_t *sem);
void sem_destroy(sem_t *sem);
int sem_trywait(sem_t *sem);
void create_thread(uint32_t *);
int open(char *, int);
int read(int, char*, int);
int write(int, char *, int);
int lseek(int, int, int);
int close(int);

#endif

#include "common.h"
#include "stdio.h"
#include "syscall.h"

int __attribute__((__noinline__)) printf(const char *ctl, ...){
	static char buf[256];
	void *args=(void**)&ctl+1;
	int len=vsnprintf(buf,256,ctl,args);
	return str_write(1,buf,len);
}

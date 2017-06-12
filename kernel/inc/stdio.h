#ifndef JOS_INC_STDIO_H
#define JOS_INC_STDIO_H

#include "stdarg.h"

#ifndef NULL
#define NULL	((void *) 0)
#endif /* !NULL */


// lib/printfmt.c
void	printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...);
void	vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list);
int	snprintf(char *str, int size, const char *fmt, ...);
int	vsnprintf(char *str, int size, const char *fmt, va_list);

// kernel/src/printk.c
int	vprintk(const char *fmt, va_list);
int	printk(const char *fmt, ...);



// debug:
#define log2(format,...)	\
	do {\
		printk("LAB2:\t"format"\tin %s,function %s, line: %d\n",\
		##__VA_ARGS__,__FILE__,__func__,__LINE__);\
	} while(0)

#endif /* !JOS_INC_STDIO_H */

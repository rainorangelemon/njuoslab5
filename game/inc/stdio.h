#ifndef __STDIO_H__
#define __STDIO_H__

#include "stdarg.h"

#ifndef NULL
#define NULL	((void *) 0)
#endif

void	printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...);
void	vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list);
int	snprintf(char *str, int size, const char *fmt, ...);
int	vsnprintf(char *str, int size, const char *fmt, va_list);

#endif

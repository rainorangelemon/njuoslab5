#include "x86.h"
#include "stdarg.h"
#include "stdio.h"

/*
 * You may refer to lib/printfmt.c
 * to implement the printk() and vprintk().
 * You can also implement a simplier version
 * by yourself.
 */

void serial_printc(char);

static void
putch(int ch, void *cnt){
	;
	/*
	 * Call the output function(such as putchar(ch)) to display character 'ch'.
	 */
	serial_printc(ch);
	(*(int *)cnt)++;
}


int	vprintk(const char *fmt,va_list ap){ 
	//hlt();
	 /*
	 * uncomment the hlt()
	 * after your completement.
	 * refer to manual.
	 */
	int n;
	vprintfmt(putch, &n, fmt, ap);
	return n;
}



int	printk(const char *fmt, ...){
//	hlt();
	 /*
	 * uncomment the hlt()
	 * after your completement.
	 * refer to manual.
	 *
	 * Hint:Use va_list to get the parameters and call vprintk().
	 *		You may also build the actual string here and call a
	 *		function which would print a string.
	 */

	va_list ap;
	va_start(ap, fmt);
	return vprintk(fmt, ap);
}


/*void serial_printc(char);

int __attribute__((__noinline__))
printk(const char *ctl, ...) {
	static char buf[256];
	void *args = (void **)&ctl + 1;
	int len = vsnprintf(buf, 256, ctl , args);
	int i;
	for(i=0; i < len; i ++) {
		serial_printc(buf[i]);
	}
	return 0;
}*/

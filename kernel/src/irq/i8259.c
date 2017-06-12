#include "x86.h"

#define IO_PIC1 0x20
#define IO_PIC2 0xA0
#define IRQ_OFFSET 0x20
#define IRQ_SLAVE 2

/* initialize 8259 intr controller：*/
void
init_intr(void) {
	outb(IO_PIC1 + 1, 0xFF);
	outb(IO_PIC2 + 1, 0xFF);
	outb(IO_PIC1, 0x11);
	outb(IO_PIC1+1, IRQ_OFFSET);
	outb(IO_PIC1+1, 1 << IRQ_SLAVE);
	outb(IO_PIC1+1, 0x3);
	outb(IO_PIC2, 0x11);              // ICW1
	outb(IO_PIC2 + 1, IRQ_OFFSET + 8);// ICW2
	outb(IO_PIC2 + 1, IRQ_SLAVE);     // ICW3
	outb(IO_PIC2 + 1, 0x01);          // ICW4
	outb(IO_PIC1, 0x68);             /* clear specific mask */
	outb(IO_PIC1, 0x0a);             /* read IRR by default */
	outb(IO_PIC2, 0x68);             /* OCW3 */
	outb(IO_PIC2, 0x0a);             /* OCW3 */
}

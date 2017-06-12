#define KOFFSET 0xC0000000
#define va_to_pa(x) (x-KOFFSET)

.text

.globl _start
_start=va_to_pa(entry)

.globl entry
entry:
	jmp init

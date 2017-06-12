#include "x86.h"
#include "assert.h"
#include "timer.h"

/* 8253输入频率为1.193182MHz */
#define TIMER_PORT 0x40
#define FREQ_8253 1193182
//#define HZ 100

#define PORT_CH_0 0x40
#define PORT_CMD 0x43
#define PIT_FREQUENCE 1193182
#define HZ 100


void
init_timer(void) {
	union CmdByte {
		struct {
			uint8_t present_mode : 1;
			uint8_t operate_mode : 3;
			uint8_t access_mode  : 2;
			uint8_t channel      : 2;
		};
		uint8_t val;
	};

	union CmdByte mode = {
		.present_mode = 0,  // 16-bit binary
		.operate_mode = 2,  // rate generator, for more accuracy
		.access_mode  = 3,  // low byte / high byte, see below
		.channel      = 0,  // use channel 0
	};

	int counter = PIT_FREQUENCE / HZ;
	outb(PORT_CMD, mode.val);
	outb(PORT_CH_0, counter & 0xFF);         // access low byte
	outb(PORT_CH_0, (counter >> 8) & 0xFF);  // access high byte
}

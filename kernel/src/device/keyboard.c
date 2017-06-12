#include "x86.h"
#include "device/scan_code.h"
#include "stdio.h"
#include "cpupa.h"

#define NR_KEYS 10

enum {KEY_STATE_EMPTY, KEY_STATE_WAIT_RELEASE, KEY_STATE_RELEASE, KEY_STATE_PRESS};

/* Only the following keys are used in NEMU-PAL. */
static const int keycode_array[] = {
	K_UP_1, K_UP_2, K_UP_3, K_UP_4, K_UP_5, K_UP_6, K_UP_7, K_UP_8, K_UP_9, K_ENTER
};

static int key_state[NR_KEYS];

void Keyboard_event(void) {
	printk("Keyboard event is called\n");
	int key_code = inb(0x60); printk("keycode == 0x%x\n", key_code);
	int i;
	for (i = 0; i < NR_KEYS; i ++){
		if(key_code == keycode_array[i]) {
			switch(key_state[i]) {
				case KEY_STATE_EMPTY:
				case KEY_STATE_RELEASE:
				case KEY_STATE_PRESS: key_state[i] = KEY_STATE_PRESS; break;
				case KEY_STATE_WAIT_RELEASE: key_state[i] = KEY_STATE_WAIT_RELEASE; break;
				default: /*assert(0);*/ break;
			}
			break;
		}
		else if(key_code == keycode_array[i] + 0x80) {
			key_state[i] = KEY_STATE_RELEASE;
			break;
		}
	}
}

int handle_keys() {
	int i;
	for(i=0; i<NR_KEYS; ++i) {
		if(key_state[i] == KEY_STATE_PRESS) {
			key_state[i] = KEY_STATE_WAIT_RELEASE;
			sti(); return keycode_array[i];
		}
		else if(key_state[i] == KEY_STATE_RELEASE) {
			key_state[i] = KEY_STATE_EMPTY;
			sti(); return keycode_array[i] + 0x80;
		}
	}
	return 0xff;
}

void Refresh_Kbdbuf(){
	int i;
	for(i=0; i<NR_KEYS; ++i){
		key_state[i]=KEY_STATE_EMPTY;
	}
}

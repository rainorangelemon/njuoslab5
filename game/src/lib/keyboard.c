#include "common.h"
#include "string.h"
#include "syscall.h"
#include "scan_code.h"

void enable_refresh();
void close_refresh();

/* 1-9对应的键盘扫描码, enter code*/
static int letter_code[] = {
	K_UP_1,K_UP_2,K_UP_3,K_UP_4,K_UP_5,K_UP_6,K_UP_7,K_UP_8,K_UP_9,K_ENTER
};
/* 对应键按下的标志位 */
static bool letter_pressed[10];

/* key_code保存了上一次键盘事件中的扫描码 */
static volatile int key_code = 0;

void
press_key(int scan_code) {
	int i;
	for (i = 0; i < 10; i ++) {
		if (letter_code[i] == scan_code) {
			letter_pressed[i] = TRUE;
		}
	}
	if(scan_code==K_ENTER)
		enable_refresh();
}

void
release_key(int scan_code) {
	int i;
	for (i = 0; i < 10; i ++) {
		if (letter_code[i] == scan_code) {
			letter_pressed[i] = FALSE;
		}
	}
	if(scan_code==K_ENTER)
		close_refresh();
}

bool process_keys(){
	uint8_t keycode=get_kbd();
	if(keycode==0xff) return false;
	if(keycode<0x80){
		press_key(keycode);return true;
	}else{
		release_key(keycode-0x80);return true;
	}
}

bool
query_key(int index) {
	//assert(0 <= index && index < 10);
	return letter_pressed[index];
}

int last_key_code(void) {
	return key_code;
}


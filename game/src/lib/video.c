#include "common.h"
#include "string.h"
#include "video.h"
#include "font.h"
#include "syscall.h"

/* 绘制屏幕的帧缓冲实现。
 * 在某些版本的qemu-kvm上，由于每次访问显存映射区域都会产生一次VM exit，
 * 更新屏幕的速度可能非常缓慢从而引起游戏跳帧。定义宏SLOW以只重绘屏幕变化
 * 的部分；定义宏TOOSLOW在只重绘屏幕变化部分的基础上，隔行更新。
 * TOOSLOW可能会引起视觉效果的损失。 */

static uint8_t vbuf[SCR_SIZE];

void
prepare_buffer(void){
	memset(vbuf, 0, SCR_SIZE);
}

void
display_buffer(void) {
	put_video(vbuf);
}

void draw_pixel(int x,int y,int color){
	vbuf[(x<<8)+(x<<6)+y]=color;
}

void
draw_character(char ch, int x, int y, int color) {
	int i, j;
	//assert((ch & 0x80) == 0);
	char *p = font8x8_basic[(int)ch];
	for (i = 0; i < 8; i ++) 
		for (j = 0; j < 8; j ++) 
			if ((p[i] >> j) & 1)
				draw_pixel(x + i, y + j, color);
}

void
draw_string(char *strm, int x, int y, int color) {
	char* str=strm;
	while (*str) {
		draw_character(*str ++, x, y, color);
		if (y + 8 >= SCR_WIDTH) {
			x += 8; y = 0;
		} else {
			y += 8;
		}
	}
}


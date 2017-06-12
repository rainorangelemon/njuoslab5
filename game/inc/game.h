#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"

/* 初始化串口 */
void init_serial();

/* 按键相关 */
void press_key(int scan_code);
void release_key(int ch);
bool query_key(int ch);
int last_key_code(void);

/* 定义fly_t array */
typedef struct fly_t{
	float x,y;
	char text;
	int index;
}jkdwiejfrv;

struct fly_t box[10];
int winc,winp;
bool has_added;
int winner;
/* 主循环 */
void main_loop(void);

/* 游戏逻辑相关 */
void initial_game(void);
void reset_game(void);
bool update_keypress(void);

bool winc_check(void);
bool winp_check(void);
bool screen_full(void);
int get_fps(void);
void set_fps(int fps);

void redraw_screen(void);

/* 随机数 */
int rand(void);
void srand(int seed);

#endif

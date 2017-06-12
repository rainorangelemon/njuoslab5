#include "x86/x86.h"
#include "x86.h"
#include "game.h"
#include "string.h"
#include "syscall.h"
#include "keyboard.h"

#define FPS 30
#define CHARACTER_PER_SECOND 5
#define UPDATE_PER_SECOND 100

static int real_fps;

static inline int syscall(int id, ...){
	int ret;
	int *args=&id;
	asm volatile("int $0x80":"=a"(ret):"a"(args[0]),"b"(args[1]),"c"(args[2]),"d"(args[3]));
	return ret;
}


void set_fps(int value) {
	real_fps = value;
}

int get_fps() {
	return real_fps;
}

void initial_game(){
	winner=0;
	winp=0;
	winc=0;
	has_added=0;
	box[0].y=100;
	box[0].x=60;
	box[1].y=200;
	box[1].x=60;
	box[2].y=300;
	box[2].x=60;
	box[3].y=100;
	box[3].x=120;
	box[4].y=200;
	box[4].x=120;
	box[5].y=300;
	box[5].x=120;
	box[6].y=100;
	box[6].x=180;
	box[7].y=200;
	box[7].x=180;
	box[8].y=300;
	box[8].x=180;
	int i;
	for(i=0;i<=9;i++){
		release_key(i);
	}
	for(i=0;i<9;i++){
		box[i].index=i;
		box[i].text='\0';
	}
	box[4].text='X';
}

void reset_game(){
	winner=0;
	winp=0;
	winc=0;
	int i;
	for(i=0;i<9;i++){
		box[i].index=i;
		box[i].text='\0';
	}
	for(i=0;i<=9;i++){
		release_key(i);
	}
	box[4].text='X';
	has_added=0;
	winner=0;
	winp=0;
	winc=0;
}

/* 游戏主循环。
 * 在初始化工作结束后，main函数就跳转到主循环执行。
 * 在主循环执行期间随时会插入异步的中断。时钟中断最终调用timer_event，
 * 键盘中断最终调用keyboard_event。中断处理完成后将返回主循环原位置继续执行。
 *
 * tick是时钟中断中维护的信号，数值含义是“系统到当前时刻已经发生过的时钟中断数”
 * HZ是时钟控制器硬件每秒产生的中断数，在include/device/timer.h中定义
 * now是主循环已经正确处理的时钟中断数，即游戏已经处理到的物理时间点
 *
 * 由于qemu-kvm在访问内存映射IO区域时每次都会产生陷入，在30FPS时，
 * 对显存区域每秒会产生30*320*200/4次陷入，从而消耗过多时间导致跳帧的产生(实际FPS<30)。
 * 在CFLAGS中增加-DSLOW可以在此情况下提升FPS。如果FPS仍太小，可以尝试
 * -DTOOSLOW，此时将会采用隔行扫描的方式更新屏幕(可能会降低显示效果)。
 * 这些机制的实现在device/video.c中。
 * */

#define s_r	0
#define s_w	1
#define s_rw	2
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

void
main_loop(void) {
	int now = get_time(), target;
	int num_draw = 0;
	bool redraw;
	int i;
	int fd=open("game_load.txt",s_rw);
	char charac;
	for(i=0;i<9;i++){
		read(fd,&charac,1);
		printf("%x",charac);
		if((charac!='X')&&(charac!='O'))
			box[i].text='\0';
		else
			box[i].text=charac;
	}	
	printf("\n");
	
	
	while (TRUE) {
		if (now == get_time()) {
			continue;
		}
		//assert(now < get_time());
		target = get_time(); /* now总是小于tick，因此我们需要“追赶”当前的时间 */
		redraw = FALSE;
		while (process_keys())
			;

		update_keypress();
		lseek(fd,0,SEEK_SET);		
		for(i=0;i<9;i++){
			charac=box[i].text;
			write(fd,&charac,1);
		}
		/* 依次模拟已经错过的时钟中断。一次主循环如果执行时间长，期间可能到来多次时钟中断，
		 * 从而主循环中维护的时钟可能与实际时钟相差较多。为了维持游戏的正常运行，必须补上
		 * 期间错过的每一帧游戏逻辑。 */
		if(has_added==1){
			reset_game();
		}
		
		while (now < target) { 
			if (now % (HZ / FPS) == 0) {
				redraw = TRUE;
			}
			/* 更新fps统计信息 */
			if (now % (HZ / 2) == 0) {
				int now_fps = num_draw * 2 + 1;
				if (now_fps > FPS) now_fps = FPS;
				set_fps(now_fps);
				num_draw = 0;
			}
			now ++;
		}
		
		

		if (redraw) { /* 当需要重新绘图时重绘 */
			num_draw ++;
			redraw_screen();
			lseek(fd,0,SEEK_SET);		
			for(i=0;i<9;i++){
				charac=box[i].text;
				write(fd,&charac,1);
			}
		}
	}
}

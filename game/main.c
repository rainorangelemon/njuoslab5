#include "common.h"
#include "x86.h"
#include "video.h"
#include "game.h"
#include "keyboard.h"
#include "syscall.h"
#include "timer.h"

void main_loop();
void pc_main();
static bool refresh;
void enable_refresh(){
	refresh=true;
}

void close_refresh(){
	refresh=false;
}

void fork_test(){
	int i;
	for(i=0;i<2;i++){
		fork();
		printf("X\n");
	}
	exit();
}

void sleep_test(){
	sleep(100);
}

void exit_test(){
	exit();
}

void getpid_test(){
	printf("pid:%d\n",getpid());
}

static inline int syscall(int id, ...){
	int ret;
	int *args=&id;
	asm volatile("int $0x80":"=a"(ret):"a"(args[0]),"b"(args[1]),"c"(args[2]),"d"(args[3]));
	return ret;
}

void producer_consumer(){
	printf("This is producer_consumer\n");
	pc_main();
}

int game_main(){
	/*producer_consumer();*/

	printf("Welcome to game\n");
	printf("You do not have to press enter any more!\n");
	printf("Also, I build the load function. You can quit at any time!\n");
	/*fork_test();*/
	/*sleep_test();*/
	/*exit_test();*/
	/*getpid_test();*/
	refresh=false;
	prepare_buffer();
	while(1){
		display_buffer();
		refresh=1;
		if(refresh){
			printf("new game!\n");
			prepare_buffer();
			display_buffer();
			initial_game();
			main_loop();
		}
	}
	printf("end here\n");
	return 0;
}

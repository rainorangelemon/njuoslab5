#include "game.h"
#include "common.h"
#include "string.h"
#include "video.h"
#include "x86/x86.h"
#include "x86.h"

bool winp_check(){
	if(((box[0].text=='O')&&(box[1].text=='O')&&(box[2].text=='O'))||
		((box[3].text=='O')&&(box[4].text=='O')&&(box[5].text=='O'))||
		((box[6].text=='O')&&(box[7].text=='O')&&(box[8].text=='O'))||
		((box[0].text=='O')&&(box[4].text=='O')&&(box[8].text=='O'))||
		((box[2].text=='O')&&(box[4].text=='O')&&(box[6].text=='O'))||
		((box[0].text=='O')&&(box[3].text=='O')&&(box[6].text=='O'))||
		((box[1].text=='O')&&(box[4].text=='O')&&(box[7].text=='O'))||
		((box[2].text=='O')&&(box[5].text=='O')&&(box[8].text=='O'))){
		return TRUE;
	}else
		return FALSE;
}

bool winc_check(){
	if(((box[0].text=='X')&&(box[1].text=='X')&&(box[2].text=='X'))||
		((box[3].text=='X')&&(box[4].text=='X')&&(box[5].text=='X'))||
		((box[6].text=='X')&&(box[7].text=='X')&&(box[8].text=='X'))||
		((box[0].text=='X')&&(box[4].text=='X')&&(box[8].text=='X'))||
		((box[2].text=='X')&&(box[4].text=='X')&&(box[6].text=='X'))||
		((box[0].text=='X')&&(box[3].text=='X')&&(box[6].text=='X'))||
		((box[1].text=='X')&&(box[4].text=='X')&&(box[7].text=='X'))||
		((box[2].text=='X')&&(box[5].text=='X')&&(box[8].text=='X'))){
		return TRUE;
	}else
		return FALSE;
}

bool screen_full(){
	int i;
	for(i=0;i<9;i++){
		if(box[i].text=='\0')
			return FALSE;
	}
	return TRUE;
}

/* 更新按键 */
bool
update_keypress(void) {
	int target=-1;
	
	/* 寻找相应键已被按下、最底部且未被击中的字符 */
	int j;
	for (j = 0; j < 9; j++) {
		if ((box[j].text=='\0')&&(query_key(box[j].index))) {
			box[j].text='O';
			target=j+1;
			int k;
			while(1){
				k=rand()%9;
				if (box[k].text=='\0'){
					box[k].text='X';
					break;
				}
			}
			break;
		}
	}
	
	if(query_key(9)==1){
		has_added=1;
		reset_game();
		return TRUE;		
	}

	/* 如果找到则更新相应数据 */
	if ((target != -1)&&(target!=9)) {
		release_key(box[j].index);
		return TRUE;
	}
	return FALSE;
}


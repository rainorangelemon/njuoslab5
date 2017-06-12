#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "types.h"
#include "const.h"
#include "assert.h"

#define SCR_WIDTH  320
#define SCR_HEIGHT 200
#define SCR_SIZE ((SCR_WIDTH) * (SCR_HEIGHT))
#define VMEM_ADDR  ((uint8_t*)0xA0000)

void prepare_buffer(void);
void display_buffer(void);

void draw_character(char,int,int,int);
void draw_string(char*, int, int, int);
#endif

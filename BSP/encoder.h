#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "main.h"
#include "tim.h"

extern volatile float encoder_left_distance;
extern volatile float encoder_right_distance;

void encoder_init(void);
void encoder_read_info_10ms(void);
void encoder_reset_distance(void);

#endif






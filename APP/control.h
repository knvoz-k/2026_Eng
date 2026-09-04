#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "main.h"

extern volatile float encoder_left_distance;
extern volatile float encoder_right_distance;

void control_forward(float target);
void control_run_10ms(void);

uint8_t is_control_done(void);
uint8_t is_control_active(void);


#endif






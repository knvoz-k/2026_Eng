#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "main.h"



void motor_start(void);
void motor_load(int16_t PWM1, int16_t PWM2);
void motor_stop(void);


#endif



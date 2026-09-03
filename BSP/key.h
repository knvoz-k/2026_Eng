#ifndef __KEY_H__
#define __KEY_H__

#include <stdint.h>
#include "main.h"

extern volatile uint8_t car_start_request;

void key_onExti(uint16_t GPIO_Pin);

#endif



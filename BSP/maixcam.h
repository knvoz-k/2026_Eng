#ifndef __MAIXCAM_H__
#define __MAIXCAM_H__

#include <stdint.h>
#include "usart.h"

typedef struct {
    uint16_t data1;
    uint16_t data2;
    uint16_t data3;
    volatile uint8_t valid;
} Maixcam_front_t;

typedef struct {
    uint16_t red_x;
    uint16_t green_x;
    uint16_t blue_x;
    uint16_t red_y;
    uint16_t green_y;
    uint16_t blue_y;
    volatile uint8_t valid;
} Maixcam_side_t;

extern Maixcam_front_t maixcam_front;
extern Maixcam_side_t maixcam_side;

void Maixcam_init(void);
void Maixcam_onRxEvent(UART_HandleTypeDef *huart, uint16_t size);

#endif



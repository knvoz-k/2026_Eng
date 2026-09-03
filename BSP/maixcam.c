#include "maixcam.h"
#include <stdio.h>
#include <string.h>

#define MAX_RECEIVE_SIZE 128


Maixcam_front_t maixcam_front = {0};
Maixcam_side_t maixcam_side = {0};
static uint8_t front_rx_buf[MAX_RECEIVE_SIZE];
static uint8_t side_rx_buf[MAX_RECEIVE_SIZE];

void Maixcam_init(void) {
    maixcam_front.valid = 0;
    maixcam_side.valid = 0;
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, front_rx_buf, sizeof(front_rx_buf));
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, side_rx_buf, sizeof(side_rx_buf));
    __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
}

static void parse_front(const uint8_t *buf, uint16_t size) {
    char frame[MAX_RECEIVE_SIZE];
    char head[4] = {0};
    char tail[4] = {0};
    int d1 = 0, d2 = 0, d3 = 0;

    if (size >= MAX_RECEIVE_SIZE) {
        return;
    }

    memcpy(frame, buf, size);
    frame[size] = '\0';

    if (sscanf(frame, "%3s %d %d %d %3s", head, &d1, &d2, &d3, tail) == 5) {
        if (strcmp(head, "#f") == 0 && strcmp(tail, "#e") == 0) { 
            maixcam_front.data1 = (uint16_t)d1;
            maixcam_front.data2 = (uint16_t)d2;
            maixcam_front.data3 = (uint16_t)d3;
            maixcam_front.valid = 1;
        }
    }

}

static void parse_side(const uint8_t *buf, uint16_t size) {
    char frame[MAX_RECEIVE_SIZE];
    char head[4] = {0};
    char tail[4] = {0};
    int rx = 0, gx = 0, bx = 0, ry = 0, gy = 0, by = 0;

    if (size >= MAX_RECEIVE_SIZE) {
        return;
    }

    memcpy(frame, buf, size);
    frame[size] = '\0';

    if (sscanf(frame, "%3s %d %d %d %d %d %d %3s", head, &rx, &gx, &bx, &ry, &gy, &by, tail) == 8) {
        if (strcmp(head, "#s") == 0 && strcmp(tail, "#e") == 0) { 
            maixcam_side.red_x = rx;
            maixcam_side.green_x = gx;
            maixcam_side.blue_x = bx;
            maixcam_side.red_y = ry;
            maixcam_side.green_y = gy;
            maixcam_side.blue_y = by;
            maixcam_side.valid = 1;
        }
    }

}

void Maixcam_onRxEvent(UART_HandleTypeDef *huart, uint16_t size) {
    if (huart == &huart1) {
        parse_front(front_rx_buf, size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, front_rx_buf, sizeof(front_rx_buf));
        // 关闭DMA半传输中断
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    } else if (huart == &huart2) {
        parse_side(side_rx_buf, size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, side_rx_buf, sizeof(side_rx_buf));
        __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
    }
}

int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, 0xffff);
    return ch;
}



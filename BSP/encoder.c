#include "encoder.h"
#include <stdio.h>


#define RADIUS 0.035f   //轮子半径
#define PER_PULSE 675   //每圈的脉冲
#define TIME 0.01f      //中断周期 10ms
#define PI 3.1415926f

volatile float encoder_left_distance = 0;
volatile float encoder_right_distance = 0;

static int32_t encoder_get_num(TIM_HandleTypeDef* htim) {
    int32_t temp = __HAL_TIM_GetCounter(htim);
    __HAL_TIM_SetCounter(htim, 0);
    return temp;
}

static float encoder_get_speed(int32_t encoder) {
    float speed = (2 * PI * RADIUS * encoder) / (TIME * PER_PULSE);
    return speed;
}

void encoder_init(void) {
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_2);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    __HAL_TIM_SET_COUNTER(&htim5, 0);
    encoder_reset_distance();
}

void encoder_reset_distance(void) {
    encoder_left_distance = 0.0f;
    encoder_right_distance = 0.0f;
}

void encoder_read_info_10ms(void) {
    int32_t pulse_left = encoder_get_num(&htim4);
    int32_t pulse_right = encoder_get_num(&htim5);
    float speed_left = encoder_get_speed(pulse_left);
    float speed_right = encoder_get_speed(pulse_right);
    encoder_left_distance += speed_left * TIME;
    encoder_right_distance += speed_right * TIME;
    printf("pulse_left: %d, pulse_right: %d, speed_left: %.2f, speed_right: %.2f, total_left_distance: %.2f, total_right_distance: %.2f\n", 
          pulse_left, pulse_right, speed_left, speed_right, encoder_left_distance, encoder_right_distance);

}









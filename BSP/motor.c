#include "motor.h"
#include "tim.h"


void motor_start(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
}

void motor_load(int16_t PWM1, int16_t PWM2) {
    // CNT < CCR 输出高电平 8400-1 4200-1
    int32_t mid = (int32_t)htim1.Init.Period / 2;

    if (PWM1 >= mid) {
        PWM1 = (int16_t)mid;
    }
    if (PWM1 <= -mid) {
        PWM1 = (int16_t)-mid;
    }
    if (PWM2 >= mid) {
        PWM2 = (int16_t)mid;
    }
    if (PWM2 <= -mid) {
        PWM2 = (int16_t)-mid;
    }

    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, mid + PWM1);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, mid - PWM1);

    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, mid + PWM2);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_4, mid - PWM2);
}

void motor_stop(void) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
}

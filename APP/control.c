#include "control.h"
#include "encoder.h"
#include "motor.h"


#define ALLOW_ERROR 10

static float target_distance = 0.0f;
static uint8_t control_active = 0;
static uint8_t control_done_flag = 0;


void control_forward(float target) {
    target_distance = target;
    control_active = 1;
    control_done_flag = 0;
    encoder_reset_distance();

}

void control_run_10ms(void) {
    float dist;

    if (!control_active || control_done_flag) {
        return;
    }

    dist = (encoder_left_distance + encoder_right_distance) * 0.5f;

    if (dist >= target_distance) {
        motor_load(0, 0);
        control_active = 0;
        control_done_flag = 1;
        return;
    }

    if (dist < target_distance - 100.0f) {
        motor_load(1000, 1000);
    } else if (dist < target_distance - 60.0f) {
        motor_load(800, 800);
    } else if (dist < target_distance - 30.0f) {
        motor_load(400, 400);
    } else {
        motor_load(200, 200);
    }

}

uint8_t is_control_done(void)  {
    return control_done_flag;
}

uint8_t is_control_active(void) {
    return control_active;
}









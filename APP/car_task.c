#include <stdio.h>
#include "car_task.h"
#include "maixcam.h"
#include "motor.h"
#include "control.h"

static CarState_t car_state = CAR_STATE_WAIT_START;
static CarState_t last_state = CAR_STATE_WAIT_START;

static uint16_t bomb_color;
static uint16_t target_color;
static uint16_t hostage_type;
static uint8_t move_started;

void car_task_start(void) {
    if (car_state == CAR_STATE_WAIT_START) {
        car_state = CAR_STATE_MOVE_TO_QRCODE;
        move_started = 0;
    }
}


void car_task_run(void) {
    if (last_state != car_state) {
        printf("[STATE] %d -> %d\n", last_state, car_state);
        last_state = car_state;
    }
    switch (car_state) {
        case CAR_STATE_WAIT_START:
            // motor: walk straight...
            // motor: stop
            motor_load(0, 0);
            printf("[TASK] Success to finish CAR_STATE_WAIT_START, wait start...\n");
            break;
        case CAR_STATE_MOVE_TO_QRCODE:
            if (!move_started) {
                printf("[TASK] Moving to QR code\n");
                control_forward(100.0f);
                move_started = 1;
            }
            if (is_control_done()) {
                printf("[TASK] QR code have arrived\n");
                move_started = 0;
                car_state = CAR_STATE_SCAN_TASK;
            }
            break;
        case CAR_STATE_SCAN_TASK:
            printf("[TASK] Starting CAR_STATE_SCAN_TASK\n");
            motor_load(0, 0);

            if (maixcam_front.valid) {
                bomb_color = maixcam_front.data1;
                target_color = maixcam_front.data2;
                hostage_type = maixcam_front.data3;
                maixcam_front.valid = 0;
                printf("[TASK] Success to finish CAR_STATE_SCAN_TASK!\n");
                printf("[INFO] bomb_color: %d, target_color: %d, hostage_type: %d\n", bomb_color, target_color, hostage_type);
                car_state = CAR_STATE_MOVE_TO_OBSTACLE;
                move_started = 0;
            } else {
                printf("[TASK] Fail to CAR_STATE_SCAN_TASK\n");
            }
            break;
        case CAR_STATE_MOVE_TO_OBSTACLE:
            if (!move_started) {
                printf("[TASK] Starting CAR_STATE_MOVE_TO_OBSTACLE\n");
                control_forward(565.0f);
                move_started = 1;
            }
            if (is_control_done()) {
                printf("[TASK] obstacle have arrived\n");
                move_started = 0;
                printf("[TASK] Success to finish CAR_STATE_MOVE_TO_OBSTACLE!\n");
                car_state = CAR_STATE_CROSS_OBSTACLE;
            }
            break;
        case CAR_STATE_CROSS_OBSTACLE:
            motor_load(0, 0);
            printf("[TASK] Obstacle logic pending\r\n");
            break;
        case CAR_STATE_FIND_BOMB:
            break;
        case CAR_STATE_GRAB_BOMB:
            break;
        case CAR_STATE_FIND_TARGET:
            break;
        case CAR_STATE_FIRE_LASER:
            break;
        case CAR_STATE_FIND_HOSTAGE:
            break;
        case CAR_STATE_RESCUE_HOSTAGE:
            break;
        case CAR_STATE_FINISHED:
            break;
        default:
            break;


        }
}



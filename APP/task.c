#include <stdio.h>
#include "task.h"
#include "maixcam.h"
#include "motor.h"
#include "control.h"

static CarState_t car_state = CAR_STATE_WAIT_START;

static uint16_t bomb_color = 0;
static uint16_t target_color = 0;
static uint16_t hostage_type = 0;


void car_task_start(void) {
    if (car_state == CAR_STATE_WAIT_START) {
        car_state = CAR_STATE_MOVE_TO_QRCODE;
    }
}


void car_task_run(void) {
    switch (car_state) {
        case CAR_STATE_WAIT_START:
            // motor: walk straight...
            // motor: stop
            printf("[TASK] Success to finish CAR_STATE_WAIT_START, wait start...\n");
            break;
        case CAR_STATE_MOVE_TO_QRCODE:
            printf("[TASK] Starting CAR_STATE_MOVE_TO_QRCODE\n");
            control_forward(100.0f);
            // motor: walk straight...
            // motor: stop
            printf("[TASK] Success to finish CAR_STATE_MOVE_TO_QRCODE\n");
            break;
        case CAR_STATE_SCAN_TASK:
            // maixcam: scan the QR Code to choose task
            printf("[TASK] Starting CAR_STATE_SCAN_TASK\n");
            if (maixcam_front.valid) {
                bomb_color = maixcam_front.data1;
                target_color = maixcam_front.data2;
                hostage_type = maixcam_front.data3;
                maixcam_front.valid = 0;
                car_state = CAR_STATE_MOVE_TO_OBSTACLE;
                printf("[TASK] Success to finish CAR_STATE_SCAN_TASK!\n");
                printf("[INFO] bomb_color: %d, target_color: %d, hostage_type: %d\n", bomb_color, target_color, hostage_type);
            }
            printf("[TASK] Fail to CAR_STATE_SCAN_TASK\n");
            break;
        case CAR_STATE_MOVE_TO_OBSTACLE:
            printf("[TASK] Starting CAR_STATE_MOVE_TO_OBSTACLE\n");
            control_forward(565.0f);
            printf("[TASK] Success to finish CAR_STATE_MOVE_TO_OBSTACLE!\n");
            car_state = CAR_STATE_CROSS_OBSTACLE;
            break;
        case CAR_STATE_CROSS_OBSTACLE:
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



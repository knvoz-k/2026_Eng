#ifndef __TASK_H__
#define __TASK_H__

/*
    扫码取任务
    -> 出发直行
    -> 越障
    -> 排爆定位
    -> 排爆抓取
    -> 反恐寻靶
    -> 激光打靶
    -> 救援目标识别
    -> 救援抓取
    -> 任务完成
*/

typedef enum {
    CAR_STATE_WAIT_START = 0,       // 等待启动
    CAR_STATE_MOVE_TO_QRCODE,       // 移动到扫描二维码区域
    CAR_STATE_SCAN_TASK,            // 扫描任务
    CAR_STATE_MOVE_TO_OBSTACLE,     // 移动到越障区
    CAR_STATE_CROSS_OBSTACLE,       // 穿过越障区
    CAR_STATE_FIND_BOMB,            // 寻找炸弹
    CAR_STATE_GRAB_BOMB,            // 抓取炸弹
    CAR_STATE_FIND_TARGET,          // 寻找靶心
    CAR_STATE_FIRE_LASER,           // 发射激光
    CAR_STATE_FIND_HOSTAGE,         // 寻找人质
    CAR_STATE_RESCUE_HOSTAGE,       // 救援人质
    CAR_STATE_FINISHED,             // 完成任务
} CarState_t;

void car_task_start(void);
void car_task_run(void);

#endif



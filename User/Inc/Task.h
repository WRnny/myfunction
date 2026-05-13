#ifndef TASK_H
#define TASK_H

#include "ti_msp_dl_config.h"

#include "bsp_key.h"
#include "bsp_track.h"
#include "bsp_motor.h"
#include "bsp_buzzer.h"

#include "pid.h"

#include "Alert.h"

typedef enum{
    TASK_INDEX_IDLE = 0,          // 任务空闲状态
    TASK_INDEX_ONE,               // 任务1索引
    TASK_INDEX_TWO,               // 任务2索引
    TASK_INDEX_THREE,             // 任务3索引
    TASK_INDEX_FOUR               // 任务4索引
}Task_Index;

/**
 * @brief 任务1实现
 * 
 * @note 从A点行驶到H点, 并在到达H点时做出声光提示
 * 
 */
void Task_1(void);

/**
 * @brief 任务2实现
 * 
 * @note 小车能够从A点驶过B点(直角顶点), 在停车时做出声光提示 
 * 
 */
void Task_2(void);

/**
 * @brief 任务3实现
 * 
 * @note 小车能够从A点出发, 依次经过B、C、D、G、H点(或BCFEDGH)并回到A点,行驶一圈。
 * 
 */
void Task_3(void);

/**
 * @brief 任务4实现
 * 
 * @note 小车小车能够从 A 点出发,经过B点并绕矩形CDEF行驶一圈再经过G、H点回到A点, 行驶一圈
 * 
 */
void Task_4(void);

#endif /* TASK_H */

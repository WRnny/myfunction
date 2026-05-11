#ifndef BSP_TRACK_H
#define BSP_TRACK_H

#include "ti_msp_dl_config.h"
#include "my_Ticks.h"

#define BSP_TRACK_NUM       sizeof(bsp_track_param) / sizeof(bsp_track_param_t)

typedef struct {
    GPIO_Regs* gpio;    // 红外循迹模块外设GPIO寄存器
    uint32_t pins;      // 红外循迹模块引脚号
}bsp_track_param_t;

typedef enum {
    track_1 = 0,
    track_2 = 1,
    track_3 = 2,
    track_4 = 3,
    track_5 = 4,
    track_6 = 5,
    track_7 = 6
}track_index_t;

extern volatile float qty;                // 检测到黑线的传感器数量
extern volatile float coord;              // 当前时刻检测到的黑线位置
extern volatile float last_coord;         // 上一时刻检测到的黑线位置

/**
 * @brief 循迹部分初始化
 * 
 */
void BspTarck_Init(void);

/**
 * @brief 循迹模块信息数据化过程
 * 
 * @note 没什么要注意的，之后根据情况自己加标志位就得了
 * 
 */
void Track_Task(void);

#endif /* BSP_TRACK_H */
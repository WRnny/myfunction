#include "bsp_track.h"

/****************** 红外循迹模块配置表 **************************/

bsp_track_param_t bsp_track_param[] = {
    {Track_PORT, Track_Track_1_PIN},
    {Track_PORT, Track_Track_2_PIN},
    {Track_PORT, Track_Track_3_PIN},
    {Track_PORT, Track_Track_4_PIN},
    {Track_PORT, Track_Track_5_PIN},
    {Track_PORT, Track_Track_6_PIN},
    {Track_PORT, Track_Track_7_PIN},
};

/****************** 循迹检测状态表 **************************/
int track_position[BSP_TRACK_NUM] = {0};  // 循迹模块位置信息数组
volatile float qty = 0.0f;                // 检测到黑线的传感器数量
volatile float coord = 0.0f;              // 当前时刻检测到的黑线位置
volatile float last_coord = 4.0f;         // 上一时刻检测到的黑线位置

/****************** 函数部分 **************************/

/**
 * @brief 循迹部分初始化
 * 
 */
void BspTarck_Init(void)
{
    // 清除循迹定时器中断标志位
    NVIC_ClearPendingIRQ(Track_task_INST_INT_IRQN);

    // 使能循迹中断
    NVIC_EnableIRQ(Track_task_INST_INT_IRQN);
}

/**
 * @brief 循迹模块信息数据化过程
 * 
 * @note 没什么要注意的，之后根据情况自己加标志位就得了
 * 
 */
void Track_Task(void)
{

    qty = 0;
    float pin_sum = 0;

    for(int i = 0; i < BSP_TRACK_NUM; i++)
    {
        if(0 < DL_GPIO_readPins(bsp_track_param[i].gpio, bsp_track_param[i].pins))  {track_position[i] = 1; qty++;} else    track_position[i] = 0;

        pin_sum += track_position[i] * (i + 1);
    }

    if(0 == qty)    coord = last_coord; 
    else  {coord = pin_sum / (qty * 1.0f); last_coord = coord;}
}

void Track_task_INST_IRQHandler(void)
{
    switch (DL_Timer_getPendingInterrupt(Track_task_INST))
    {
    case DL_TIMER_IIDX_ZERO:
        Track_Task();
        DL_Timer_clearInterruptStatus(Track_task_INST, DL_TIMER_IIDX_ZERO);
        break;
    
    default:
        break;
    }
}

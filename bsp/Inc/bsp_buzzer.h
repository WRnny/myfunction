#ifndef BSP_BUZZER_H
#define BSP_BUZZER_H

#include "ti_msp_dl_config.h"
#include "my_Ticks.h"

typedef struct{
    GPTIMER_Regs *timer;            // 定时器外设的寄存器地址
    DL_TIMER_CC_INDEX ccIndex;      // 定时器的计数器索引, 就是定时器配置的通道号
}bsp_buzzer_param_t;

/**
 * @brief 蜂鸣器初始化函数
 * 
 * @note 这个是个空函数，当前情况调不调用无所谓
 * 
 */
void BspBuzzer_Init(void);

/**
 * @brief 打开蜂鸣器鸣叫
 * 
 */
void BspBuzzer_ON(void);

/**
 * @brief 关闭蜂鸣器鸣叫
 * 
 */
void BspBuzzer_OFF(void);

/**
 * @brief 根据当前蜂鸣器状态去翻
 * 
 */
void BspBuzzer_toggle(void);

/**
 * @brief 蜂鸣器提示任务
 * 
 * @param dt 蜂鸣器的鸣叫时间
 */
void Buzeer_AlertTask(uint32_t dt);

#endif /* BSP_BUZZER_H */

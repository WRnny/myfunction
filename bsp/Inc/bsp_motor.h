#ifndef __BSP_MOTOR_H__
#define __BSP_MOTOR_H__

#include "ti_msp_dl_config.h"
#include "pid.h"

#define BSP_MOTOR_NUM           sizeof(motor_param) / sizeof(bsp_motor_param_t) // 配置电机数量

typedef struct{
    GPIO_Regs *gpio_in1;                // 电机DIR引脚IN1的GPIO外设的寄存器地址
    uint32_t pins_in1;                  // 电机DIR引脚IN1的位域掩码，就是引脚号
    GPIO_Regs *gpio_in2;                // 电机DIR引脚IN2的GPIO外设的寄存器地址
    uint32_t pins_in2;                  // 电机DIR引脚IN2的位域掩码，就是引脚号
}bsp_motor_dir_param_t;

typedef struct{
    GPTIMER_Regs *timer;            // 定时器外设的寄存器地址
    DL_TIMER_CC_INDEX ccIndex;      // 定时器的计数器索引, 就是定时器配置的通道号
    bsp_motor_dir_param_t dir;      // 电机DIR引脚参数
}bsp_motor_param_t;

typedef enum{
    BSP_MOTOR_A = 0,
    BSP_MOTOR_B = 1,
}motor_index_t;


/**
 * @brief 电机初始化函数
 * 
 */
void BspMotor_Init(void);

/**
 * @brief 配置电机速度的函数
 * 
 * @param motor_id 指定电机ID
 * @param speed 设置电机pwm通道的占空比, 范围-10000 ~ 10000
 * 
 * @note 暂时没有写PID逻辑，后面打算写个PID的逻辑去控制电机的转速
 */
void BspMotor_SetSpeed(motor_index_t motor_id, int16_t dutyCycle);

/**
 * @brief 角度环控制电机速度
 * 
 * @param motor_id 电机的名称
 * 
 * @param Speed 电机速度
 * 
 * @note 只针对当前写了，建议速度不要超过八十即可
 */
void SpeedLoop_set(motor_index_t motor_id, int Speed);


#endif /* __BSP_MOTOR_H__ */

#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include "ti_msp_dl_config.h"
#include "my_Ticks.h"
#include "bsp_uart.h"

#define BSP_ENCODER_NUM         sizeof(bsp_encoder_param) / sizeof(bsp_encoder_param_t)
#define MI_P                    3.14159265358979323846

typedef struct {
    IRQn_Type IRQn_A;           // A相特定的中断号
    IRQn_Type IRQn_B;           // B相特定的中断号

    GPIO_Regs *gpio_A;          // A相的GPIO外设寄存器地址
    uint32_t pins_A;            // A相的GPIO外设引脚号
    GPIO_Regs *gpio_B;          // B相的GPIO外设寄存器地址
    uint32_t pins_B;            // B相的GPIO外设引脚号

    volatile int count;         // 编码器计数值
    volatile float speed;       // 编码器测速的速度 --> cm/s(每秒)
    volatile float rpm;         // 电机所转圈数
    volatile float distance;    // 电机行驶路程
}bsp_encoder_param_t;


typedef enum {
    E1 = 0,
    E2 = 1
}encoder_index_t;

extern bsp_encoder_param_t bsp_encoder_param[];

/**
 * @brief 编码器初始化函数
 * 
 */
void BspEncoder_Init(void);

/**
 * @brief 编码器的脉冲数转换成时间电机速度
 * 
 * @note 本次采用的是MG513X电机霍尔编码器
 * 
 * @note 电机数据大约是
 *       减速比 --> 1:28
 *       霍尔编码器线数 --> 13ppr
 *       轮子半径 --> 68.0mm
 * 
 * @return 将会把配置表里面的关于电机速度的信息转化更新
 *         电机转的圈数(rpm)        -->     单位rpm/s(转每秒)
 *         电机行驶路程(distance)   -->     单位m(米)
 *         电机的行驶速度(speed)    -->     单位cm/s(厘米每秒)
 */
void Claculate_MotorSpeed(void);

#endif /* BSP_ENCODER_H */

#include "bsp_encoder.h"

/****************** 编码器配置表 ***********************/
bsp_encoder_param_t bsp_encoder_param[] = {
    {Encoder_GPIOB_INT_IRQN, Encoder_GPIOB_INT_IRQN,
     Encoder_E1_A_PORT, Encoder_E1_A_PIN, Encoder_E1_B_PORT, Encoder_E1_B_PIN, 0}, // E1编码器配置

    {Encoder_GPIOA_INT_IRQN, Encoder_GPIOB_INT_IRQN,
     Encoder_E2_A_PORT, Encoder_E2_A_PIN, Encoder_E2_B_PORT, Encoder_E2_B_PIN, 0}, // E2编码器配置

};

/**
 * @brief 编码器初始化函数
 * 
 */
void BspEncoder_Init(void)
{
    for(int i = 0; i < BSP_ENCODER_NUM; i++)
    {
        // 清除测速定时器中断标志位
        NVIC_ClearPendingIRQ(bsp_encoder_param[i].IRQn_A);
        NVIC_ClearPendingIRQ(bsp_encoder_param[i].IRQn_B);

        // 使能测速定时器中断
        NVIC_EnableIRQ(bsp_encoder_param[i].IRQn_A);
        NVIC_EnableIRQ(bsp_encoder_param[i].IRQn_B);

    }

    // // 清除定时器中断标志位
    // NVIC_ClearPendingIRQ(Speedmeasurement_Task_INST_INT_IRQN);

    // // 使能定时器中断
    // NVIC_EnableIRQ(Speedmeasurement_Task_INST_INT_IRQN);
    
}

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
 * 
 * @note 左轮(E2)满转大约在 --> 89cm/s
 */
void Claculate_MotorSpeed(void)
{
    for(int i = 0; i < BSP_ENCODER_NUM; i++)
    {
        // 暂存转的圈数的变量
        float rpm_tmpe = ( bsp_encoder_param[i].count / ( (13 * 2) * 28.0f ) ) / 0.03f;

        // 记录转的圈数
        bsp_encoder_param[i].rpm += rpm_tmpe;

        // 记录电机的行驶路程
        bsp_encoder_param[i].distance += (rpm_tmpe * 6.8f) / 100.0f;

        // 转换成速度
        bsp_encoder_param[i].speed =  rpm_tmpe * MI_P * 6.8f;
        bsp_encoder_param[i].count = 0;
    }
}

/**
 * @brief 获取编码器脉冲值
 * 
 * 
 */
void GROUP1_IRQHandler(void)
{
    for (int i = 0; i < BSP_ENCODER_NUM; i++)
    {
        // 检测A相上升沿检测
        if ((DL_GPIO_getEnabledInterruptStatus(bsp_encoder_param[i].gpio_A, bsp_encoder_param[i].pins_A) & bsp_encoder_param[i].pins_A) == bsp_encoder_param[i].pins_A)
        {
            if (!(DL_GPIO_readPins(bsp_encoder_param[i].gpio_B, bsp_encoder_param[i].pins_B)))
            {
                bsp_encoder_param[i].count++;
            }
            else
            {
                bsp_encoder_param[i].count--;
            }
        }
        // 检测B相上升沿
        else if ((DL_GPIO_getEnabledInterruptStatus(bsp_encoder_param[i].gpio_B, bsp_encoder_param[i].pins_B) & bsp_encoder_param[i].pins_B) == bsp_encoder_param[i].pins_B)
        {
            if (!(DL_GPIO_readPins(bsp_encoder_param[i].gpio_A, bsp_encoder_param[i].pins_A)))
            {
                bsp_encoder_param[i].count--;
            }
            else
            {
                bsp_encoder_param[i].count++;
            }
        }

        // 清除中断标志位
        DL_GPIO_clearInterruptStatus(bsp_encoder_param[i].gpio_A, bsp_encoder_param[i].pins_A);
        DL_GPIO_clearInterruptStatus(bsp_encoder_param[i].gpio_B, bsp_encoder_param[i].pins_B);
    }
}

/**
 * @brief 定时器中断回调函数,每50ms进行速度计算
 * 
 */
// void Speedmeasurement_Task_INST_IRQHandler(void)
// {
//     switch (DL_Timer_getPendingInterrupt(Speedmeasurement_Task_INST))
//     {
//     case DL_TIMER_IIDX_ZERO:
//         Claculate_MotorSpeed();
//         DL_Timer_clearInterruptStatus(Speedmeasurement_Task_INST, DL_TIMER_IIDX_ZERO);
//         break;
    
//     default:
//         break;
//     }
// }

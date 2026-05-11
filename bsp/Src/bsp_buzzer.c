#include "bsp_buzzer.h"

/****************** 蜂鸣器配置表 ***********************/

bsp_buzzer_param_t buzzer_param[] = {
    {Debug_Buzzer_INST, GPIO_Debug_Buzzer_C0_IDX}   // 蜂鸣器配置
};

/****************** 函数逻辑部分 ***********************/

/**
 * @brief 蜂鸣器初始化函数
 * 
 * @note 这个是个空函数，当前情况调不调用无所谓
 * 
 */
void BspBuzzer_Init(void)
{
    // nothing to do here

    /**
     * @note 在DL库中蜂鸣器初始化有俩种形式
     * 
     * 1. 在配置的时候图形化界面勾选(Start Timer)的时候 --> 当前的选择
     *    这种情况下在定时器初始化之后就开始计数，所以不需要单独的进行蜂鸣器定时器通道开启操作
     *    关于DIR引脚也可以在配置的时候设置为默认高电平状态,如果不想这么做自己设置即可
     * 
     * 2. 在配置的时候图形化界面不勾选(Start Timer)的时候 --> 非当前选择
     *    这种情况下需要单独的进行蜂鸣器定时器通道开启操作
     *    需要用户主动的调用" DL_TimerX_startCounter() "这个函数开启定时器计数
     *    关于DIR引脚也可以在配置的时候设置为默认高电平状态,如果不想这么做自己设置即可
     */


     /**
      * @brief 第二种情况下的蜂鸣器初始化示例代码如下:
    for(int i = 0; i < BSP_MOTOR_NUM; i++)
    {
        DL_TimerX_startCounter(buzzer_param[i].timer); // 开启定时器计数
    }
      */

}

/**
 * @brief 打开蜂鸣器鸣叫
 * 
 */
void BspBuzzer_ON(void)
{

    DL_Timer_setCaptureCompareValue(Debug_Buzzer_INST, 50000, GPIO_Debug_Buzzer_C0_IDX);

}

/**
 * @brief 关闭蜂鸣器鸣叫
 * 
 */
void BspBuzzer_OFF(void)
{

    DL_Timer_setCaptureCompareValue(Debug_Buzzer_INST, 0, GPIO_Debug_Buzzer_C0_IDX);

}

/**
 * @brief 根据当前蜂鸣器状态去翻
 * 
 */
void BspBuzzer_toggle(void)
{
    if( 0 < DL_Timer_getCaptureCompareValue(Debug_Buzzer_INST, GPIO_Debug_Buzzer_C0_IDX))
    {
        BspBuzzer_OFF();
    }else{
        BspBuzzer_ON();
    }
}



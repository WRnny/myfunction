#include "my_Ticks.h"

volatile uint32_t uwTick = 0;

/**
 * @brief 这个函数用于传递一个全局变量uwTick，作为程序运行时间的基准
 * 
 * @note 默认每1ms执行一次Systick中断服务程序中递增一次
 * 
 */
void WR_IncTik(void)
{
    uwTick += WR_TICK_FREQ_DEFAULT; 
}

/**
 * @brief 提供以毫秒为单位的计时值，获取当前程序运行时间
 * 
 * @return uint32_t 
 */
uint32_t WR_GetTick(void)
{
    return uwTick;
}

/**
 * @brief 提供以毫秒为单位的延时功能
 * 
 * @param Delay 
 */
void WR_Delay(uint32_t Delay)
{
    uint32_t tickstart = WR_GetTick();
    uint32_t wait = Delay;

    // 添加一个频率保证最小等等时间
    if(wait < WR_MAX_Delay)
    {
        wait += (uint32_t)WR_TICK_FREQ_DEFAULT;
    }

    while(WR_GetTick() - tickstart < wait)
    {

    }
}

/**
 * @brief SysTick中断服务程序
 * 
 */
void SysTick_Handler(void)
{
    WR_IncTik();
}


#include "bsp_key.h"

/****************** 按键配置表 ***********************/

bsp_key_param_t bsp_key_param[] = {

    {Debug_key_Key_center_PORT, Debug_key_Key_center_PIN, .starttick = 0, .endtick = 0, false},
    {Debug_key_Key_right_PORT, Debug_key_Key_right_PIN, .starttick = 0, .endtick = 0, false},
    {Debug_key_Key_left_PORT, Debug_key_Key_left_PIN, .starttick = 0, .endtick = 0, false},
    {Debug_key_Key_up_PORT, Debug_key_Key_up_PIN, .starttick = 0, .endtick = 0, false},
    {Debug_key_Key_down_PORT, Debug_key_Key_down_PIN, .starttick = 0, .endtick = 0, false},

};

/****************** 按键索引 ***********************/

/**
 * 
 * @note 索引的话主要是提高代码可阅读性
 * @note 一定要记得加索引，不然代码逻辑会混乱
 * 
 * @note 索引记得与配置表的顺序相同不然全乱掉， 其他部分也是这样不然全乱光光
 * 
 * @note 枚举的话放在了"bsp_key.h"里面记得去调整 
 */

 /****************** 函数逻辑部分 ***********************/

 /**
  * @brief 按键任务初始化
  * 
  * @note 主要是初始化定时器
  * 
  */
void BspKey_Init()
{
    // 清除定时器中断标志位
    NVIC_ClearPendingIRQ(Key_task_INST_INT_IRQN);

    // 使能定时器中断
    NVIC_EnableIRQ(Key_task_INST_INT_IRQN);
}

/**
 * @brief 非阻塞式按键检测状态
 * 
 * @note 检测逻辑要一致
 * @note 如果引脚配置上拉电阻需要把invert打开
 * 
 */

void BSP_KeyTask(void)
{

    for(int i = 0; i < BSP_KEY_NUMBER; i++)
    {
        bool current_state = (0 < DL_GPIO_readPins(bsp_key_param[i].gpio, bsp_key_param[i].pins));

        // 按键空闲状态
        if( (bsp_key_param[i].last_state == false && current_state == false && bsp_key_param[i].is_busy == key_busy) 
            ||  bsp_key_param[i].key_shortpressflag == true || bsp_key_param[i].key_longpressflag == true)
        {
            memset(&bsp_key_param[i].last_state, 0, sizeof(bsp_key_param_t) - offsetof(bsp_key_param_t, last_state));
        }

        // 检测按键下
        if(bsp_key_param[i].last_state == false && current_state == true)
        {
            bsp_key_param[i].is_busy = key_busy;
            bsp_key_param[i].key_pressflag = true;
            bsp_key_param[i].key_holdflag = true;
            bsp_key_param[i].starttick = WR_GetTick();
        }

        // 检测按键释放
        if(bsp_key_param[i].last_state == true && current_state == false)
        {
            bsp_key_param[i].key_releaseflag = true;
            bsp_key_param[i].endtick = WR_GetTick();
        }

        // 按键长按短按判断
        if(bsp_key_param[i].is_busy == key_idle)
        {
            // 按键空闲状态
            if(bsp_key_param[i].endtick == bsp_key_param[i].starttick);
            // 短按按键判断时间小于200ms
            else if(bsp_key_param[i].endtick - bsp_key_param[i].starttick < 200)
            {
                bsp_key_param[i].key_shortpressflag = true;
                bsp_key_param[i].endtick = bsp_key_param[i].starttick;
            }else{
                bsp_key_param[i].key_longpressflag = true;
                bsp_key_param[i].endtick = bsp_key_param[i].starttick;
            }
        }
        bsp_key_param[i].last_state = current_state;
    }
    
}

/**
 * @brief 执行简单的按键触发逻辑
 * 
 * @param task 执行的任务逻辑
 * @param key_flag 指定按键检测方式
 * 
 */
void WR_KeyControlTask(TaskFunc task, bool* key_flag)
{
    // 错误处理
    if(task == NULL)
    {
        return;
    }

    // 判断按键状态
    if(*key_flag == true)
    {
        task();
        *key_flag = false;
    }
}

/**
 * @brief 定时器中断每20ms检测按键状态
 * 
 * @note 优先级 --> Level3 - Lowest
 * 
 */
void Key_task_INST_IRQHandler(void)
{
    switch (DL_Timer_getPendingInterrupt(Key_task_INST))
    {
    case DL_TIMER_IIDX_ZERO:
        BSP_KeyTask();
        DL_Timer_clearInterruptStatus(Key_task_INST, DL_TIMER_IIDX_ZERO);
        break;
    
    default:
        break;
    }
}

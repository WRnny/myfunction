#include "bsp_key.h"

bsp_key_param_t bsp_key_param[] = {
    {Debug_key_PORT, Debug_key_Board_Key_PIN, .starttick = 0, .endtick = 0, false}
};

/**
 * @brief 非阻塞式按键检测状态
 * 
 * @note 检测逻辑要一致
 * @note 如果引脚配置上拉电阻需要把invert打开
 * 
 */

void BSP_KeyTask(void)
{
    WR_TASK_PERIODIC(bsp_keytask, 20);

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
            // 短按按键判断时间小于300ms
            else if(bsp_key_param[i].endtick - bsp_key_param[i].starttick < 300)
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
 * @param 
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

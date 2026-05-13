#include "Task.h"

int debug_time = 0;
int right_count = 0;
int roundabout_count = 0;
Task_Index Task_State = TASK_INDEX_IDLE;

void Task_Init(void *param)
{
    Task_Index task_index = (Task_Index)(uintptr_t)param;
    Task_State = task_index;
}

/**
 * @brief 任务1实现
 * 
 * @note 从A点行驶到H点, 并在到达H点时做出声光提示
 * 
 */
void Task_1(void)
{
    // 向上短按拨动五向按键, 开启任务1
    WR_KeyControlTaskParam(Task_Init, &bsp_key_param[Key_up].key_shortpressflag, WR_TASK_PARAM(TASK_INDEX_ONE));
    if(Task_State != TASK_INDEX_ONE)    return;

    // 开启循迹模式
    TrackLop_Init();

    // 任务1检测到H点的情况
    if(Track_State == TRACK_FLAG_ALLWHITE && (Angleloop_PIDItem.error > 80 || Angleloop_PIDItem.error < -80))
    {
        
        // 关闭循迹模式
        TrackLop_DeInit();

        // H点停止
        SpeedLoop_set(BSP_MOTOR_A, 0);
        SpeedLoop_set(BSP_MOTOR_B, 0);

        // 任务置为空闲状态
        Task_State = TASK_INDEX_IDLE;
        // 声光提示开启
        alert_state = ALERT_OK;

    }
}


/**
 * @brief 任务2实现
 * 
 * @note 小车能够从A点驶过B点(直角顶点), 在停车时做出声光提示 
 * 
 */
void Task_2(void)
{
    // 向下短按拨动五向按键, 开启任务2
    WR_KeyControlTaskParam(Task_Init, &bsp_key_param[Key_down].key_shortpressflag, WR_TASK_PARAM(TASK_INDEX_TWO));
    if(Task_State != TASK_INDEX_TWO)    return;

    // 计次开启循迹环
    if (debug_time == 0)
    {
        TrackLop_Init();
        debug_time++;
    }

    // 红外传感器检测到直角的时候
    if(bsp_encoder_param[E1].distance > 3.0 && bsp_encoder_param[E2].distance > 3.0 && Track_State == TRACK_FLAG_RIGHT_ONLEFT)
    {
        // 计次改变角度环
        if (debug_time == 1)
        {
            TrackLop_DeInit();

            Angleloop_PIDParam.target += 90.0f;
            debug_time++;
            right_count++;

            AngleLop_Init();
        }
    }

    // 经过干扰路段重新开启循迹, 关闭角度环
    if (bsp_encoder_param[E1].distance > 12.5 && bsp_encoder_param[E2].distance > 12.5 && right_count == 1)
    {
       
        AngleLop_DeInit();
        TrackLop_Init();

        // if(debug_time == 1) {debug_time++;  return;} 
        // 注意不要用我写的那个ABS()不清楚为什么用那个比较就会一直判断为真，所以直接这么写
        if (Angleloop_PIDItem.error > 80 || Angleloop_PIDItem.error < -80)
        {
            TrackLop_DeInit();

            SpeedLoop_set(BSP_MOTOR_A, 0);
            SpeedLoop_set(BSP_MOTOR_B, 0);

            Task_State = TASK_INDEX_IDLE;
            alert_state = ALERT_OK;
        }
    
    }
}

/**
 * @brief 任务3实现
 * 
 * @note 小车能够从A点出发, 依次经过B、C、D、G、H点(或BCFEDGH)并回到A点,行驶一圈。
 * 
 */
void Task_3(void)
{
    // 向左短按拨动五向按键, 开启任务2
    WR_KeyControlTaskParam(Task_Init, &bsp_key_param[Key_left].key_shortpressflag, WR_TASK_PARAM(TASK_INDEX_THREE));
    if(Task_State != TASK_INDEX_THREE)    return;

    // 计次开启循迹环
    if (debug_time == 0)
    {
        TrackLop_Init();
        debug_time++;
    }

    // 红外传感器检测到直角的时候
    if(Track_State == TRACK_FLAG_RIGHT_ONLEFT)
    {
        // 计次改变角度环
        if (debug_time == 1)
        {
            // 关闭循迹模式, 将角度环目标左偏90°, 并打开角度环
            TrackLop_DeInit();

            Angleloop_PIDParam.target += 90.0f;
            debug_time++;
            right_count++;

            AngleLop_Init();
        }
    }

    // 经过干扰路段重新开启循迹, 关闭角度环
    if (bsp_encoder_param[E1].distance > 12.5 && bsp_encoder_param[E2].distance > 12.5 && right_count == 1 && debug_time == 2)
    {
       // 关闭角度环, 开启循迹模式
        AngleLop_DeInit();
        TrackLop_Init();
    }

    // 到达G点的情况判断
    if (bsp_encoder_param[E1].distance > 22.0 && bsp_encoder_param[E2].distance > 22.0 && Track_State == TRACK_FLAG_ALLWHITE)
    {
        //关闭循迹模式
        if (debug_time == 2)
        {
            TrackLop_DeInit();
            debug_time++;
        }
        // 将角度环目标左偏180°, 打开角度环
        if (debug_time == 3)
        {
            Angleloop_PIDParam.target += 180.0f;
            debug_time++;
            AngleLop_Init();
        }

    }

    // 到达H点判断
    if (bsp_encoder_param[E1].distance > 27.0 && bsp_encoder_param[E2].distance > 27.0 && Track_State == TRACK_FLAG_IDLE)
    {
        // 开启循迹模式
        TrackLop_Init();
        AngleLop_DeInit();
    }

    // 到达A点判断
    if (bsp_encoder_param[E1].distance > 33.0 && bsp_encoder_param[E2].distance > 33.0 && Track_State == TRACK_FLAG_ALLBLACK)
    {
        // 关闭循迹模式并且停车开启声光提示
        TrackLop_DeInit();
        AngleLop_DeInit();

        SpeedLoop_set(BSP_MOTOR_A, 0);
        SpeedLoop_set(BSP_MOTOR_B, 0);

        Task_State = TASK_INDEX_IDLE;
        alert_state = ALERT_OK;
    }

}

/**
 * @brief 任务4实现
 * 
 * @note 小车小车能够从 A 点出发,经过B点并绕矩形CDEF行驶一圈再经过G、H点回到A点, 行驶一圈
 * 
 */
void Task_4(void)
{
    // 向左短按拨动五向按键, 开启任务2
    WR_KeyControlTaskParam(Task_Init, &bsp_key_param[Key_right].key_shortpressflag, WR_TASK_PARAM(TASK_INDEX_FOUR));
    if (Task_State != TASK_INDEX_FOUR)  return;

    // 计次开启循迹环
    if (debug_time == 0)
    {
        TrackLop_Init();
        debug_time++;
    }

    // 红外传感器检测到直角的时候
    if (Track_State == TRACK_FLAG_RIGHT_ONLEFT)
    {
        // 计次改变角度环
        if (debug_time == 1)
        {
            // 关闭循迹模式, 将角度环目标左偏90°, 并打开角度环
            TrackLop_DeInit();

            Angleloop_PIDParam.target += 90.0f;
            debug_time++;
            right_count++;

            AngleLop_Init();
        }
    }

    // 经过干扰路段重新开启循迹, 关闭角度环
    if (bsp_encoder_param[E1].distance > 12.5 && bsp_encoder_param[E2].distance > 12.5 && right_count == 1 && debug_time == 2)
    {
        // 关闭角度环, 开启循迹模式
        if (debug_time == 2)
        {
            AngleLop_DeInit();
            TrackLop_Init();
            debug_time++;
        }
    }

    if (bsp_encoder_param[E1].distance > 20 && bsp_encoder_param[E2].distance > 20 && right_count == 1 && debug_time == 3)
    {
        if (Track_State == TRACK_FLAG_RIGHT_ONLEFT)
        {
            if(roundabout_count == 0)
            {
                TrackLop_DeInit();
                Angleloop_PIDParam.target += 180.0f;
                roundabout_count++;

                AngleLop_Init();
            }

        }

        if (Yaw_received - Angleloop_PIDParam.target < 2 && Yaw_received - Angleloop_PIDParam.target > -2)
        {
            AngleLop_DeInit();
            TrackLop_Init();
        }
    }

    if (bsp_encoder_param[E1].distance > 23 && bsp_encoder_param[E2].distance > 23 && right_count == 1 && roundabout_count == 1)
    {
        if (Track_State == TRACK_FLAG_ALLBLACK)
        {
            if(roundabout_count == 1)
            {
                TrackLop_DeInit();
                Angleloop_PIDParam.target -= 90.0f;
                roundabout_count++;

                AngleLop_Init();
            }

        }

        if (Yaw_received - Angleloop_PIDParam.target < 2 && Yaw_received - Angleloop_PIDParam.target > -2)
        {
            AngleLop_DeInit();
            TrackLop_Init();
        }
    }


    // 到达G点的情况判断
    if (bsp_encoder_param[E1].distance > 30.0 && bsp_encoder_param[E2].distance > 30.0 && Track_State == TRACK_FLAG_ALLWHITE)
    {
        // 关闭循迹模式
        if (debug_time == 3)
        {
            TrackLop_DeInit();
            debug_time++;
        }
        // 将角度环目标左偏180°, 打开角度环
        if (debug_time == 4)
        {
            Angleloop_PIDParam.target += 90.0f;
            debug_time++;
            AngleLop_Init();
        }
    }

    // 到达H点判断
    if (bsp_encoder_param[E1].distance > 35.0 && bsp_encoder_param[E2].distance > 35.0 && Track_State == TRACK_FLAG_IDLE)
    {
        // 开启循迹模式
        TrackLop_Init();
        AngleLop_DeInit();
    }

    // 到达A点判断
    if (bsp_encoder_param[E1].distance > 36.0 && bsp_encoder_param[E2].distance > 36.0 && Track_State == TRACK_FLAG_ALLBLACK)
    {
        // 关闭循迹模式并且停车开启声光提示
        TrackLop_DeInit();
        AngleLop_DeInit();

        SpeedLoop_set(BSP_MOTOR_A, 0);
        SpeedLoop_set(BSP_MOTOR_B, 0);

        Task_State = TASK_INDEX_IDLE;
        alert_state = ALERT_OK;
    }
}
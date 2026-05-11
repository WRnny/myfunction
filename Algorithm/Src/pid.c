#include "pid.h"

/**
 * @brief 开始速度环
 * 
 */
void SpeedLop_Init(void)
{
    // 清除速度环定时器中断标志位
    NVIC_ClearPendingIRQ(PidSpeed_Task_INST_INT_IRQN);

    // 使能速度环定时器中断
    NVIC_EnableIRQ(PidSpeed_Task_INST_INT_IRQN);
}

/**
 * @brief PID计算逻辑
 *
 * @param pid_param     PID基本成员Ki,Kp,Kd,target
 * @param pid_item      PID计算的一些基本变量，和一些优化处理变量
 * @param measure       实际测量值
 * @param dt            计算时间间隔
 * @return float        返回PID的调节值
 */
float Pid_Calculate(PID_Param* pid_param, PID_Item* pid_item, float measure, float dt)
{
    if(dt <= 0.0f)
    {
        return 0.0f;
    }

    pid_item->error = pid_param->target - measure;

    if(pid_item->ZERO > 0)
    {
        if(fabs(pid_item->error) < pid_item->ZERO)
        {
            pid_item->error = 0;
        }
    }

    pid_item->integral += pid_item->error;

    // 如果设置了积分限幅在开启积分限幅
    if(pid_item->integral_max > 0.0f)
    {
        pid_item->integral = fmaxf( -pid_item->integral_max, fminf(pid_item->integral_max, pid_item->integral) );
    }

    // 加个低阶滤波系数抗干扰
    float raw_derivative = (pid_item->error - pid_item->last_error) / dt; 
    pid_item->derivative = D_TERM_FILTER * pid_item->derivative + (1.0f - D_TERM_FILTER) * raw_derivative;

    float output = pid_param->Kp * pid_item->error + pid_param->Ki * pid_item->integral + pid_param->Kd * pid_item->derivative; 

    // 如果设置了输出限幅在开启这输出限幅
    if(pid_item->output_max > 0.0f)
    {
        float raw_output = output;
        if(pid_param->target > 0.0f && pid_item->output_min > 0.0f)
        {
            output = fmaxf(pid_item->output_min, fminf(pid_item->output_max, output));
        }
        else if(pid_param->target < 0.0f && pid_item->output_min > 0.0f)
        {
            output = fminf(-pid_item->output_min, fmaxf(-pid_item->output_max, output));
        }
        else
        {
            output = fmaxf(-pid_item->output_max, fminf(pid_item->output_max, output));
        }
        if(fabs(output - raw_output) > 1e-5f && pid_param->Ki > 1e-6f)
        {
            pid_item->integral -= (raw_output - output) * dt / pid_param->Ki;
        }
    }

    pid_item->last_error = pid_item->error;

    return output;
}

PID_Param SpeedloopL_PIDParam = {
    .Kp = 666.0f,
    .Ki = 6.66f,
    .Kd = 0.0f,
    .target = 0.0f
};

PID_Item SpeedloopL_PIDItem = {
    .integral_max = 0.0f,
    .output_max = 0.0f,
    .output_min = 0.0f,

    .ZERO = 0.0f
};

PID_Param SpeedloopR_PIDParam = {
    .Kp = 0.0f,
    .Ki = 0.0f,
    .Kd = 0.0f,
    .target = 0.0f
};

PID_Item SpeedloopR_PIDItem = {
    .integral_max = 0.0f,
    .output_max = 0.0f,
    .output_min = 0.0f,

    .ZERO = 0.0f
};

volatile float output_L = 0.0f;
volatile float output_R = 0.0f;

float vofa_arry1[20] = {0};

void PidSpeed_Task_INST_IRQHandler(void)
{
    switch (DL_Timer_getPendingInterrupt(PidSpeed_Task_INST))
    {
    case DL_TIMER_IIDX_ZERO:
        Claculate_MotorSpeed();
        // vofa_arry1[0] = bsp_encoder_param[E1].speed;
        // vofa_arry1[1] = bsp_encoder_param[E2].speed;

        output_R = Pid_Calculate(&SpeedloopR_PIDParam, &SpeedloopR_PIDItem, bsp_encoder_param[E1].speed, 0.02);
        output_L = Pid_Calculate(&SpeedloopL_PIDParam, &SpeedloopL_PIDItem, bsp_encoder_param[E2].speed, 0.02);

        BspMotor_SetSpeed(BSP_MOTOR_B, output_L); // 左轮 

        // VOFA_SendData(vofa_arry1, 2);

        break;
    
    default:
        break;
    }
}

#ifndef __PID_H__
#define __PID_H__

#include "ti_msp_dl_config.h"
#include "bsp_encoder.h"
#include "bsp_uart.h"
#include "bsp_motor.h"
#include "math.h"

#define D_TERM_FILTER 0.2f // 滤波系数

typedef struct{
    float Kp;
    float Ki;
    float Kd;
    float target;
}PID_Param;

typedef struct{
    float integral_max;
    float output_max;
    float output_min;
    float ZERO;

    float integral;
    float derivative;
    float error;
    float last_error;
}PID_Item;

extern PID_Param SpeedloopL_PIDParam;

/**
 * @brief 开始速度环
 * 
 */
void SpeedLop_Init(void);


float Pid_Calculate(PID_Param* pid_param, PID_Item* pid_item, float measure, float dt);

#endif /* __PID_H__ */

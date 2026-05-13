#ifndef __PID_H__
#define __PID_H__

#include "ti_msp_dl_config.h"
#include "bsp_encoder.h"
#include "bsp_uart.h"
#include "bsp_motor.h"
#include "bsp_track.h"
#include "math.h"

#define D_TERM_FILTER 0.2f // 滤波系数
#define ABS(x) x >= 0 ? x : -x

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
extern PID_Param SpeedloopR_PIDParam;
extern PID_Param Angleloop_PIDParam;
extern PID_Item Angleloop_PIDItem;


/**
 * @brief 开始速度环
 * 
 */
void SpeedLop_Init(void);

void SpeedLop_DeInit(void);

/**
 * @brief 开启循迹环
 * 
 */
void TrackLop_Init(void);

/**
 * @brief 关闭循迹位置环
 * 
 */
void TrackLop_DeInit(void);

/**
 * @brief 上电初始化的时候获取基本角度值
 * 
 * @note 不要动小车
 */
void anglebase_init(void);

/**
 * @brief 开启角度环
 * 
 */
void AngleLop_Init(void);

/**
 * @brief 关闭角度环
 * 
 */
void AngleLop_DeInit(void);

/**
 * @brief 将角度误差转换成最短路径
 * 
 */
void diff_angle(void);

/**
 * @brief PID计算逻辑
 *
 * @param pid_param     PID基本成员Ki,Kp,Kd,target
 * @param pid_item      PID计算的一些基本变量，和一些优化处理变量
 * @param measure       实际测量值
 * @param dt            计算时间间隔
 * @return float        返回PID的调节值
 */
float Pid_Calculate(PID_Param* pid_param, PID_Item* pid_item, float measure, float dt);

#endif /* __PID_H__ */

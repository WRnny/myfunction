#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "ti_msp_dl_config.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

extern uint8_t rx_data;

extern volatile float Yaw_received; // Yaw轴接收数据

/**
 * @brief 串口初始化
 * 
 */
void BspUART_Init(void);

/**
 * @brief 发送VOFA JustFloat协议数据
 *
 * @param data 浮点数据数组
 * @param count 数据个数
 * @return 发送是否成功
 */
bool VOFA_SendData(float *data, int count);

#endif /* __BSP_UART_H__ */

#ifndef BSP_UART_H
#define BSP_UART_H

#include "ti_msp_dl_config.h"
#include "my_Ticks.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"


typedef struct {
    unsigned char       *buffer;        // 环形缓冲区起始地址
    size_t              length;         // 缓冲区长度
    size_t              count;          // 当前环形缓冲区的数据量
    size_t              head;           // 缓冲区的头
    size_t              tail;           // 缓冲区的尾
    size_t              maxCount;       // 缓冲区的最大使用长度
}RingBuf_Object, *RingBuf_Handle;

void commond_send(char ch);
void sendCaliYawCommand(void) ;

/**
 * @brief 初始化RingBuf的RAM空间指针
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param bufPtr 环形缓冲区的指针地址
 * @param bufSize 环形缓冲区大小
 * 
 */
void RingBuf_construct(RingBuf_Handle object, unsigned char *bufPtr, size_t bufSize);

/**
 * @brief RingBuf读取单个数据
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param data 数据转存地址
 */
int RingBuf_get(RingBuf_Handle object, unsigned char *data);

/**
 * @brief 用来获取当前缓冲区内数据个数的信息
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @return int 返回当前RingBuf内的数据个数
 */
int RingBuf_getCount(RingBuf_Handle object);

/**
 * @brief 一次性获取多个数据
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param data 数据转存地址
 * @param n 读取数据个数
 * @return int 返回读取的数据个数
 */
int RingBuf_getn(RingBuf_Handle object, unsigned char *data, size_t n);

/**
 * @brief 实现单个数据的写入
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param data 数据转存地址
 * @return int 返回当前缓存区数据长度
 */
int RingBuf_put(RingBuf_Handle object, unsigned char data);

/**
 * @brief 实现多个数据的一次性写入
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param data 数据转存地址
 * @param n 写入个数
 * @return int 返回写入了多少个数据
 */
int RingBuf_putn(RingBuf_Handle object, unsigned char *data, size_t n);


#endif // !BSP_UART_H

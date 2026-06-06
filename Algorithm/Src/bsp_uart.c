#include "bsp_uart.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int fputc(int ch, FILE *f)
    {
        while (DL_UART_isBusy(UART0) == UART_STAT_BUSY_SET)
            ;
        DL_UART_Main_transmitData(UART0, ch);
        return ch;
    }

#ifdef __cplusplus
}
#endif

void commond_send(char ch)
{
    //当串口0忙的时候等待，不忙的时候再发送传进来的字符
    while( DL_UART_isBusy(UART0) == UART_STAT_BUSY_SET );
    //发送单个字符
    DL_UART_Main_transmitData(UART0, ch);
}

//解锁指令
uint8_t Key[5] = {0x55, 0xAA, 0x13, 0x8E, 0x5F};
//Z轴角度归零指令
uint8_t Yaw_Zero[5] = {0x55, 0xAA, 0x15, 0x00, 0x00};
//保存指令
uint8_t Save[5] = {0x55, 0xAA, 0x00, 0x00, 0x00};
//获取零偏指令
uint8_t BIAS_CAL[5] = {0x55, 0xAA, 0x0A, 0x01, 0x00};

void uart0_send_SendByte(uint8_t* data, uint32_t len)
{
    for(uint32_t i = 0; i < len; i++)
    {
        commond_send(data[i]);  // 直接发送原始字节
    }
}

void sendCaliYawCommand(void) 
{ 
   uart0_send_SendByte(Key, 5);
	 WR_Delay(100);
	 uart0_send_SendByte(Yaw_Zero, 5);
	 WR_Delay(100);
	 uart0_send_SendByte(Save, 5);
}


/**
 * @brief 初始化RingBuf的RAM空间指针
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param bufPtr 环形缓冲区的指针地址
 * @param bufSize 环形缓冲区大小
 * 
 */
void RingBuf_construct(RingBuf_Handle object, unsigned char *bufPtr, size_t bufSize)
{
    object->buffer = bufPtr;            // 存放数组地址
    object->length = bufSize;           // 存放数组大小
    object->count = 0;                  // 初始化将环形缓冲区存放数据量置0
    object->head = bufSize - 1;         // 读取索引
    object->tail = 0;                   // 写入索引
    object->maxCount =0;                // 最大读取数
}

/**
 * @brief 计算当前RingBuf的空闲位置
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @return size_t 返回剩余的空间量
 */
static size_t RingBuf_space(RingBuf_Handle object)
{
    return (object->length - object->count);
}

/**
 * @brief RingBuf读取单个数据
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param data 数据转存地址
 */
int RingBuf_get(RingBuf_Handle object, unsigned char *data)
{
    __disable_irq();    // 关闭全局中断保证数据读取稳定性

    // 检查RingBuf是否存在数据
    if(!object->count)
    {
        __enable_irq(); // 开启全局中断保证中断继续运行
        return -1; // 返回错误信息
    }

    *data = object->buffer[object->tail]; // 将RingBuf中tail指向的数据存到*data指向的地址
    object->tail = (object->tail + 1) % object->length; // 让tail自增取模是为了当tail到了length的长度归0
    object->count--; // RingBuf有效数据已经读取, 有效数据-1

    __enable_irq(); // 开启全局中断

    return (object->count); // 返回当前RingBuf存放的有效数据
}

/**
 * @brief 用来获取当前缓冲区内数据个数的信息
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @return int 返回当前RingBuf内的数据个数
 */
int RingBuf_getCount(RingBuf_Handle object)
{
    return (object->count);
}

/**
 * @brief 一次性获取多个数据
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param data 数据转存地址
 * @param n 读取数据个数
 * @return int 返回读取的数据个数
 */
int RingBuf_getn(RingBuf_Handle object, unsigned char *data, size_t n)
{
    size_t remove = 0; // 用于记录数据的读取个数

    __disable_irq(); // 关闭全局中断保证数据读取正常
    // 防止想要读取的数据个数超过RingBuf存放的数据个数
    if(n > object->count)
    {
        n = object->count;
    }

    // 阻塞式读取
    while(n)
    {
        *data++ = object->buffer[object->tail++]; // 将RingBuf中tail指向的数据存到*data指向的地址
        object->tail %= object->length; // 防止tail超过RingBuf长度
        --object->count; // RingBuf有效数据已经读取, 有效数据-1 
        --n;
        ++remove;
    }

    __enable_irq();
    return (remove);
}

/**
 * @brief 实现单个数据的写入
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param data 数据转存地址
 * @return int 返回当前缓存区数据长度
 */
int RingBuf_put(RingBuf_Handle object, unsigned char data)
{
    unsigned char next; // 用于向RingBuf的下一个位置填写数据

    __disable_irq(); // 关闭全局中断保证数据正常写入

    //  检测存放数据大小
    if(object->count != object->length)
    {
        next = (object->head + 1) % object->length;
        object->buffer[next] = data;
        object->head = next;
        object->count++;
        object->maxCount = (object->count > object->maxCount) ? object->count : object->maxCount;
    }else{
        __enable_irq();
        return -1;
    }

    __enable_irq();

    return (object->count);
}

/**
 * @brief 实现多个数据的一次性写入
 * 
 * @param object 环形缓冲区结构体(RingBuf句柄)的指针地址
 * @param data 数据转存地址
 * @param n 写入个数
 * @return int 返回写入了多少个数据
 */
int RingBuf_putn(RingBuf_Handle object, unsigned char *data, size_t n)
{
    // 跟上面差不多懒得写注释了
    size_t next;
    size_t added = 0;

    __disable_irq();
    if(n > RingBuf_space(object))
    {
        n = RingBuf_space(object);
    }

    while(n)
    {
        next = (object->head + 1) % object->length;
        object->buffer[next] = *data++;
        object->head = next;
        ++object->count;
        --n;
        ++added;
    }

    object->maxCount = (object->maxCount < object->count) ? object->count : object->maxCount;

    __enable_irq();

    return (added);
}



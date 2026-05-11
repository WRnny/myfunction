#include "bsp_uart.h"

/*********************************** 不可以用会卡死 ******************************************/

/**
 * @brief printf重定向到UART串口重定向
 *
 * 需要注意的是不同编译器的printf重定向实现可能不同
 * 所以还是建议AI去搞一下printf重定向的实现
 *
 */
#ifdef __cplusplus
extern "C"
{
#endif

    int fputc(int ch, FILE *f)
    {
        while (DL_UART_isBusy(Debug_UART_INST) == UART_STAT_BUSY_SET)
            ;
        DL_UART_Main_transmitData(Debug_UART_INST, ch);
        return ch;
    }

#ifdef __cplusplus
}
#endif

/*********************************** 不可以用会卡死 ******************************************/

uint8_t rx_data;

/**
 * @brief 串口初始化
 *
 */
void BspUART_Init(void)
{
    // 使能调试中断标志位
    NVIC_ClearPendingIRQ(Debug_UART_INST_INT_IRQN);

    // 使能调试接收中断
    NVIC_EnableIRQ(Debug_UART_INST_INT_IRQN);

    // 清除yaw轴获取中断标志位
    NVIC_ClearPendingIRQ(Rx_yaw_INST_INT_IRQN);

    // 使能yaw轴获取中断
    NVIC_EnableIRQ(Rx_yaw_INST_INT_IRQN);
}

void Debug_UART_INST_IRQHandler(void)
{

    switch (DL_UART_getPendingInterrupt(Debug_UART_INST))
    {
    case DL_UART_IIDX_RX:
        rx_data = DL_UART_Main_receiveData(Debug_UART_INST);
        break;
    default:
        break;
    }
}

static char uart0_rx_buf[256];      // 接收缓存区
static uint8_t uart0_rx_index = 0;  // 接收索引
volatile float Yaw_received = 0.0f; // Yaw轴接收数据
volatile uint8_t Yaw_RxFlag = 0;    // Yaw轴接收数据索引
float Yaw_sum = 0;                  // 初始化Yaw轴数据积累
float Basic_set_yaw = 0;            // 上电测的基准轴

/**
 * @brief 简化的Yaw轴数据接收处理
 * 使用更可靠的逐字节解析，避免浮点数解析阻塞
 */
void UART1_IRQHandler(void)
{
    // 确保中断源正确
    uint32_t int_status = DL_UART_getPendingInterrupt(UART1);

    if (int_status == DL_UART_IIDX_RX)
    {
        uint8_t RxData = DL_UART_Main_receiveData(UART1);

        // 立即清除中断标志，防止重复中断
        DL_UART_clearInterruptStatus(UART1, DL_UART_IIDX_RX);

        // 调试：LED指示接收到数据
        // DL_GPIO_togglePins(DEBUG_LED_PORT, DEBUG_LED_PIN);

        // 简化的接收逻辑
        if (RxData == '\r' || RxData == '\n')
        {
            if (uart0_rx_index > 0)
            {
                uart0_rx_buf[uart0_rx_index] = '\0';
                Yaw_received = (float)atof(uart0_rx_buf);
                Yaw_RxFlag = 1;

                uart0_rx_index = 0;
            }
        }
        else if (uart0_rx_index < 127) // 保留一个字节给字符串结束符
        {
            uart0_rx_buf[uart0_rx_index++] = (char)RxData;
        }
        // 如果缓冲区满了，丢弃数据并重置
        else
        {
            uart0_rx_index = 0;
        }
    }
    else
    {
        // 清除其他可能的中断标志
        DL_UART_clearInterruptStatus(UART1, int_status);
    }
}

/**
 * @brief 串口发送一字节数据
 */
void Serial_SendByte(uint8_t Byte)
{
    DL_UART_transmitDataBlocking(Debug_UART_INST, Byte);
}

/**
 * @brief 发送VOFA JustFloat协议数据
 *
 * @param data 浮点数据数组
 * @param count 数据个数
 * @return 发送是否成功
 */
bool VOFA_SendData(float *data, int count)
{
    if (data == NULL || count <= 0)
    {
        return false;
    }

    // 发送浮点数据
    for (int i = 0; i < count; i++)
    {
        uint8_t *bytes = (uint8_t *)&data[i];
        // 小端模式发送
        for (int j = 0; j < 4; j++)
        {
            Serial_SendByte(bytes[j]);
        }
    }

    // 发送帧尾标识
    const uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7f};
    for (int i = 0; i < 4; i++)
    {
        Serial_SendByte(tail[i]);
    }

    return true;
}
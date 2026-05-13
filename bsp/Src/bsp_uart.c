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

// 函数声明
static void BspUART_ParsePIDCommand(char* cmd);

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

// 自定义快速浮点数解析函数
static bool fast_parse_float(const char *str, float *result)
{
    if (!str || !result)
        return false;

    char *end;
    *result = strtof(str, &end);

    // 检查是否成功解析了整个字符串
    return (end != str && *end == '\0');
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
 float Yaw_received = 0.0f; // Yaw轴接收数据
volatile uint8_t Yaw_RxFlag = 0;    // Yaw轴接收数据索引
float Yaw_sum = 0;                  // 初始化Yaw轴数据积累
float Basic_set_yaw = 0;            // 上电测的基准轴

/**
 * @brief 简化的Yaw轴数据接收处理
 * 使用更可靠的逐字节解析，避免浮点数解析阻塞
 */
  void UART1_IRQHandler(void)
  {
      if (DL_UART_getPendingInterrupt(UART1) == DL_UART_IIDX_RX)
      {
          uint8_t RxData = DL_UART_Main_receiveData(UART1);

          // 使用局部变量优化
          uint8_t idx = uart0_rx_index;

          if (RxData == '\r' || RxData == '\n')
          {
              if (idx > 0)
              {
                  uart0_rx_buf[idx] = '\0';

                  // 使用更高效的解析
                  if (fast_parse_float(uart0_rx_buf, &Yaw_received))
                  {
                      Yaw_RxFlag = 1;
                  }

                  idx = 0;
              }
          }
          else if (idx < UART_RX_BUF_SIZE - 1)
          {
              uart0_rx_buf[idx++] = (char)RxData;
          }
          else
          {
              idx = 0; // 溢出重置
          }

          uart0_rx_index = idx;
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
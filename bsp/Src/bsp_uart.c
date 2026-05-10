#include "bsp_uart.h"

/**
 * @brief printf重定向到UART串口重定向
 * 
 * 需要注意的是不同编译器的printf重定向实现可能不同
 * 所以还是建议AI去搞一下printf重定向的实现
 * 
 */
#ifdef __cplusplus
extern "C" {
#endif

int fputc(int ch, FILE *f) {
    while(DL_UART_isBusy(Debug_UART_INST) == UART_STAT_BUSY_SET);
    DL_UART_Main_transmitData(Debug_UART_INST, ch);
    return ch;
}

#ifdef __cplusplus
}
#endif

uint8_t rx_data;

void BspUART_Init(void)
{
    // 清除中断标志位
    NVIC_ClearPendingIRQ(Debug_UART_INST_INT_IRQN);

    // 开启接收中断
    NVIC_EnableIRQ(Debug_UART_INST_INT_IRQN);
}

void Debug_UART_INST_IRQHandler(void)
{

    switch(DL_UART_getPendingInterrupt(Debug_UART_INST))
    {
        case DL_UART_IIDX_RX:
            rx_data = DL_UART_Main_receiveData(Debug_UART_INST);
            break;
        default:
            break;
    }
}

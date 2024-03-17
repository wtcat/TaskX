/*
 * 
 */
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>

#include "tx_api.h"
#include "stm32h7xx_ll_usart.h"



 struct uart_device {
    USART_TypeDef *dev;
    TX_SEMAPHORE *rxsem;
 };

 static struct uart_device uart_4_device = {
    .dev        = UART4,
    .rxsem      = NULL
 };

 static void stm32_uart_isr(void *arg) {
     struct uart_device *uart = arg;

 }

int stm32_uart_open(int port, struct uart_device **pdev) {
    LL_USART_InitTypeDef uart_struct;
    struct uart_device *uart;
    int err;

    switch (port) {
    case 4:
        uart = &uart_4_device;
        break;
    default:
        return -EINVAL;
    }

    LL_USART_StructInit(&uart_struct);
    uart_struct.BaudRate = 2000000;
    LL_USART_Init(uart->dev, &uart_struct);
    err = cpu_irq_attatch(UART4_IRQn, stm32_uart_isr, uart, -1);
    if (err)
        return err;

    LL_USART_IsEnabledIT_RXNE_RXFNE(uart->dev);
    LL_USART_EnableIT_TXE(uart->dev);

    *pdev = uart;
    return 0;
}

int stm32_uart_close(struct uart_device *uart) {

}

ssize_t stm32_uart_read(struct uart_device *uart, void *buf, size_t size, bool wait) {

}

ssize_t stm32_uart_write(struct uart_device *uart, const void *buf, size_t size) {
    
}

int stm32_uart_control(struct uart_device *uart, uint32_t cmd, void *arg) {

}


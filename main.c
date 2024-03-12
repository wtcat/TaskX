/*
 * Copyright 2024 wtcat
 */
#include <errno.h>

#include "tx_api.h"
#include "base/generic.h"

typedef struct {
  volatile uint32_t CR1;    /*!< USART Control register 1,                 Address offset: 0x00 */
  volatile uint32_t CR2;    /*!< USART Control register 2,                 Address offset: 0x04 */
  volatile uint32_t CR3;    /*!< USART Control register 3,                 Address offset: 0x08 */
  volatile uint32_t BRR;    /*!< USART Baud rate register,                 Address offset: 0x0C */
  volatile uint32_t GTPR;   /*!< USART Guard time and prescaler register,  Address offset: 0x10 */
  volatile uint32_t RTOR;   /*!< USART Receiver Time Out register,         Address offset: 0x14 */
  volatile uint32_t RQR;    /*!< USART Request register,                   Address offset: 0x18 */
  volatile uint32_t ISR;    /*!< USART Interrupt and status register,      Address offset: 0x1C */
  volatile uint32_t ICR;    /*!< USART Interrupt flag Clear register,      Address offset: 0x20 */
  volatile uint32_t RDR;    /*!< USART Receive Data register,              Address offset: 0x24 */
  volatile uint32_t TDR;    /*!< USART Transmit Data register,             Address offset: 0x28 */
  volatile uint32_t PRESC;  /*!< USART clock Prescaler register,           Address offset: 0x2C */
} USART_TypeDef;

// static void putstr(const char *s) {
//     USART_TypeDef *reg = (USART_TypeDef *)(0x40000000UL + 0x4C00UL);

//     while (*s != '\0') {
//         char ch = *s;

//         while (!(reg->ISR & (1 << 7)));
//         reg->TDR = ch;
//         if (ch == '\n')
//             ch = '\r';
//         else
//             s++;
//     }
// }


static TX_THREAD thread_id;
static char thread_stack[2048] __section(".dtcm.stack");

volatile int thrd_count[2];

static void main_thread(void *ptr) {

    // putstr("******* Main thread **********\n");

    for ( ; ; ) {
        thrd_count[0]++;
        // putstr("** Tick **\n");
        _tx_thread_sleep(1000);
    }
}

static void main_thread_(void *ptr) {

    // putstr("******* Main thread **********\n");

    for ( ; ; ) {
        thrd_count[1]++;
        // putstr("** Tick **\n");
        _tx_thread_sleep(1000);
    }
}

void tx_application_define(void *ptr) {

    _tx_thread_create(&thread_id, "init", (void (*)(ULONG))main_thread, (ULONG)ptr, 
        thread_stack, sizeof(thread_stack), 15, 15, 0, 1);

static TX_THREAD thread_id_;
static char thread_stack_[2048];
    _tx_thread_create(&thread_id_, "init", (void (*)(ULONG))main_thread_, (ULONG)ptr, 
        thread_stack_, sizeof(thread_stack_), 15, 15, 0, 1);
}

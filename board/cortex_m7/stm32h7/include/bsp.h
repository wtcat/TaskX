/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * Copyright (C) 2020 embedded brains GmbH & Co. KG
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LIBBSP_ARM_STM32H7_BSP_H
#define LIBBSP_ARM_STM32H7_BSP_H

#include <bspopts.h>
#include <bsp/default-initial-extension.h>

#include <rtems.h>

#ifdef CONFIG_BSP_USERDATA
# include "board/stm32_board.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup RTEMSBSPsARMSTM32H7 STM32H7
 *
 * @ingroup RTEMSBSPsARM
 *
 * @brief STM32H7 Board Support Package.
 *
 * @{
 */
#define BSP_FDT_IS_SUPPORTED

#define BSP_FEATURE_IRQ_EXTENSION

#define BSP_ARMV7M_IRQ_PRIORITY_DEFAULT (13 << 4)

#define BSP_ARMV7M_SYSTICK_PRIORITY (14 << 4)

#define BSP_ARMV7M_SYSTICK_FREQUENCY stm32h7_systick_frequency()

uint32_t stm32h7_systick_frequency(void);

/* default functions */
void stm32h7_init_power(void);
void stm32h7_init_oscillator(void);
void stm32h7_init_clocks(void);
void stm32h7_init_peripheral_clocks(void);
void stm32h7_init_qspi(void);

/** @} */

/*
 * dma_memcpy -- Instead of memory by hardware dma transmit
 *
 * @dst: destination address
 * @src: source address
 * @size: copy size
 * return 0 if success
 */
int dma_memcpy(void *dst, const void *src, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* LIBBSP_ARM_STM32H7_BSP_H */

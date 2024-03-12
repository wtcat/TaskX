/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @ingroup RTEMSScoreCPUARM
 *
 * @brief This header file provides interfaces of the ARMv7-M architecture
 *   support.
 */

/*
 * Copyright (c) 2011-2014 Sebastian Huber.  All rights reserved.
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

#ifndef RTEMS_SCORE_ARMV7M_H
#define RTEMS_SCORE_ARMV7M_H

#ifdef TX_INCLUDE_USER_DEFINE_FILE
#include "tx_user.h"
#endif

#define ARM_MULTILIB_ARCH_V7M  
#define ARM_MULTILIB_VFP

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef ARM_MULTILIB_ARCH_V7M

/* Coprocessor Access Control Register, CPACR */
#define ARMV7M_CPACR 0xe000ed88

#ifndef ARMV7M_SOC_IRQCNT
#define ARMV7M_SOC_IRQCNT 200
#endif

#ifndef ASM

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__GNUC__)
  #define RTEMS_OBFUSCATE_VARIABLE( _var ) __asm__( "" : "+r" ( _var ) )
#else
  #define RTEMS_OBFUSCATE_VARIABLE( _var ) (void) ( _var )
#endif

typedef struct {
  uint32_t reserved_0;
  uint32_t ictr;
  uint32_t actlr;
  uint32_t reserved_1;
} ARMV7M_ICTAC;

typedef void (*ARMV7M_Exception_handler)(void);

typedef struct {
  uint32_t register_r0;
  uint32_t register_r1;
  uint32_t register_r2;
  uint32_t register_r3;
  uint32_t register_r12;
  void *register_lr;
  void *register_pc;
  uint32_t register_xpsr;
#ifdef ARM_MULTILIB_VFP
  uint32_t register_s0;
  uint32_t register_s1;
  uint32_t register_s2;
  uint32_t register_s3;
  uint32_t register_s4;
  uint32_t register_s5;
  uint32_t register_s6;
  uint32_t register_s7;
  uint32_t register_s8;
  uint32_t register_s9;
  uint32_t register_s10;
  uint32_t register_s11;
  uint32_t register_s12;
  uint32_t register_s13;
  uint32_t register_s14;
  uint32_t register_s15;
  uint32_t register_fpscr;
  uint32_t reserved;
#endif
} ARMV7M_Exception_frame;

typedef struct {
  uint32_t comp;
  uint32_t mask;
  uint32_t function;
  uint32_t reserved;
} ARMV7M_DWT_comparator;

typedef struct {
#define ARMV7M_DWT_CTRL_NOCYCCNT (1U << 25)
#define ARMV7M_DWT_CTRL_CYCCNTENA (1U << 0)
  uint32_t ctrl;
  uint32_t cyccnt;
  uint32_t cpicnt;
  uint32_t exccnt;
  uint32_t sleepcnt;
  uint32_t lsucnt;
  uint32_t foldcnt;
  uint32_t pcsr;
  ARMV7M_DWT_comparator comparator[249];
#define ARMV7M_DWT_LAR_UNLOCK_MAGIC 0xc5acce55U
  uint32_t lar;
  uint32_t lsr;
} ARMV7M_DWT;

typedef struct {
  uint32_t cpuid;

#define ARMV7M_SCB_ICSR_NMIPENDSET (1U << 31)
#define ARMV7M_SCB_ICSR_PENDSVSET (1U << 28)
#define ARMV7M_SCB_ICSR_PENDSVCLR (1U << 27)
#define ARMV7M_SCB_ICSR_PENDSTSET (1U << 26)
#define ARMV7M_SCB_ICSR_PENDSTCLR (1U << 25)
#define ARMV7M_SCB_ICSR_ISRPREEMPT (1U << 23)
#define ARMV7M_SCB_ICSR_ISRPENDING (1U << 22)
#define ARMV7M_SCB_ICSR_VECTPENDING_GET(reg) (((reg) >> 12) & 0x1ffU)
#define ARMV7M_SCB_ICSR_RETTOBASE (1U << 11)
#define ARMV7M_SCB_ICSR_VECTACTIVE_GET(reg) ((reg) & 0x1ffU)
  uint32_t icsr;

  ARMV7M_Exception_handler *vtor;

#define ARMV7M_SCB_AIRCR_VECTKEY (0x05fa << 16)
#define ARMV7M_SCB_AIRCR_ENDIANESS (1U << 15)
#define ARMV7M_SCB_AIRCR_PRIGROUP_SHIFT 8
#define ARMV7M_SCB_AIRCR_PRIGROUP_MASK \
  ((0x7U) << ARMV7M_SCB_AIRCR_PRIGROUP_SHIFT)
#define ARMV7M_SCB_AIRCR_PRIGROUP(val) \
  (((val) << ARMV7M_SCB_AIRCR_PRIGROUP_SHIFT) & ARMV7M_SCB_AIRCR_PRIGROUP_MASK)
#define ARMV7M_SCB_AIRCR_PRIGROUP_GET(reg) \
  (((val) & ARMV7M_SCB_AIRCR_PRIGROUP_MASK) >> ARMV7M_SCB_AIRCR_PRIGROUP_SHIFT)
#define ARMV7M_SCB_AIRCR_PRIGROUP_SET(reg, val) \
  (((reg) & ~ARMV7M_SCB_AIRCR_PRIGROUP_MASK) | ARMV7M_SCB_AIRCR_PRIGROUP(val))
#define ARMV7M_SCB_AIRCR_SYSRESETREQ (1U << 2)
#define ARMV7M_SCB_AIRCR_VECTCLRACTIVE (1U << 1)
#define ARMV7M_SCB_AIRCR_VECTRESET (1U << 0)
  uint32_t aircr;

  uint32_t scr;
  uint32_t ccr;
  uint8_t shpr [12];

#define ARMV7M_SCB_SHCSR_USGFAULTENA (1U << 18)
#define ARMV7M_SCB_SHCSR_BUSFAULTENA (1U << 17)
#define ARMV7M_SCB_SHCSR_MEMFAULTENA (1U << 16)
  uint32_t shcsr;

#define ARMV7M_SCB_CFSR_MMFSR_MASK 0xff
#define ARMV7M_SCB_CFSR_MMFSR_GET(n) (n & ARMV7M_SCB_CFSR_MMFSR_MASK)
#define ARMV7M_SCB_CFSR_BFSR_MASK 0xff00
#define ARMV7M_SCB_CFSR_BFSR_GET(n) (n & ARMV7M_SCB_CFSR_BFSR_MASK)
#define ARMV7M_SCB_CFSR_UFSR_MASK 0xffff0000
#define ARMV7M_SCB_CFSR_UFSR_GET(n) (n & ARMV7M_SCB_CFSR_UFSR_MASK)
  uint32_t cfsr;

#define ARMV7M_SCB_HFSR_VECTTBL_MASK 0x2
#define ARMV7M_SCB_HFSR_FORCED_MASK (1U << 30)
#define ARMV7M_SCB_HFSR_DEBUGEVT_MASK (1U << 31)
  uint32_t hfsr;

  uint32_t dfsr;
  uint32_t mmfar;
  uint32_t bfar;
  uint32_t afsr;
  uint32_t reserved_e000ed40[18];
  uint32_t cpacr;
  uint32_t reserved_e000ed8c[106];
  uint32_t fpccr;
  uint32_t fpcar;
  uint32_t fpdscr;
  uint32_t mvfr0;
  uint32_t mvfr1;
} ARMV7M_SCB;

typedef struct {
#define ARMV7M_SYSTICK_CSR_COUNTFLAG (1U << 16)
#define ARMV7M_SYSTICK_CSR_CLKSOURCE (1U << 2)
#define ARMV7M_SYSTICK_CSR_TICKINT (1U << 1)
#define ARMV7M_SYSTICK_CSR_ENABLE (1U << 0)
  uint32_t csr;

  uint32_t rvr;
  uint32_t cvr;

#define ARMV7M_SYSTICK_CALIB_NOREF (1U << 31)
#define ARMV7M_SYSTICK_CALIB_SKEW (1U << 30)
#define ARMV7M_SYSTICK_CALIB_TENMS_GET(reg) ((reg) & 0xffffffU)
  uint32_t calib;
} ARMV7M_Systick;

typedef struct {
  uint32_t iser [8];
  uint32_t reserved_0 [24];
  uint32_t icer [8];
  uint32_t reserved_1 [24];
  uint32_t ispr [8];
  uint32_t reserved_2 [24];
  uint32_t icpr [8];
  uint32_t reserved_3 [24];
  uint32_t iabr [8];
  uint32_t reserved_4 [56];
  uint8_t  ipr [240];
  uint32_t reserved_5 [644];
  uint32_t stir;
} ARMV7M_NVIC;

typedef struct {
#define ARMV7M_MPU_TYPE_IREGION_GET(reg) (((reg) >> 16) & 0xffU)
#define ARMV7M_MPU_TYPE_DREGION_GET(reg) (((reg) >> 8) & 0xffU)
#define ARMV7M_MPU_TYPE_SEPARATE (1U << 0)
  uint32_t type;

#define ARMV7M_MPU_CTRL_PRIVDEFENA (1U << 2)
#define ARMV7M_MPU_CTRL_HFNMIENA (1U << 1)
#define ARMV7M_MPU_CTRL_ENABLE (1U << 0)
  uint32_t ctrl;

  uint32_t rnr;

#define ARMV7M_MPU_RBAR_ADDR_SHIFT 5
#define ARMV7M_MPU_RBAR_ADDR_MASK \
  ((0x7ffffffU) << ARMV7M_MPU_RBAR_ADDR_SHIFT)
#define ARMV7M_MPU_RBAR_ADDR(val) \
  (((val) << ARMV7M_MPU_RBAR_ADDR_SHIFT) & ARMV7M_MPU_RBAR_ADDR_MASK)
#define ARMV7M_MPU_RBAR_ADDR_GET(reg) \
  (((val) & ARMV7M_MPU_RBAR_ADDR_MASK) >> ARMV7M_MPU_RBAR_ADDR_SHIFT)
#define ARMV7M_MPU_RBAR_ADDR_SET(reg, val) \
  (((reg) & ~ARMV7M_MPU_RBAR_ADDR_MASK) | ARMV7M_MPU_RBAR_ADDR(val))
#define ARMV7M_MPU_RBAR_VALID (1U << 4)
#define ARMV7M_MPU_RBAR_REGION_SHIFT 0
#define ARMV7M_MPU_RBAR_REGION_MASK \
  ((0xfU) << ARMV7M_MPU_RBAR_REGION_SHIFT)
#define ARMV7M_MPU_RBAR_REGION(val) \
  (((val) << ARMV7M_MPU_RBAR_REGION_SHIFT) & ARMV7M_MPU_RBAR_REGION_MASK)
#define ARMV7M_MPU_RBAR_REGION_GET(reg) \
  (((val) & ARMV7M_MPU_RBAR_REGION_MASK) >> ARMV7M_MPU_RBAR_REGION_SHIFT)
#define ARMV7M_MPU_RBAR_REGION_SET(reg, val) \
  (((reg) & ~ARMV7M_MPU_RBAR_REGION_MASK) | ARMV7M_MPU_RBAR_REGION(val))
  uint32_t rbar;

#define ARMV7M_MPU_RASR_XN (1U << 28)
#define ARMV7M_MPU_RASR_AP_SHIFT 24
#define ARMV7M_MPU_RASR_AP_MASK \
  ((0x7U) << ARMV7M_MPU_RASR_AP_SHIFT)
#define ARMV7M_MPU_RASR_AP(val) \
  (((val) << ARMV7M_MPU_RASR_AP_SHIFT) & ARMV7M_MPU_RASR_AP_MASK)
#define ARMV7M_MPU_RASR_AP_GET(reg) \
  (((val) & ARMV7M_MPU_RASR_AP_MASK) >> ARMV7M_MPU_RASR_AP_SHIFT)
#define ARMV7M_MPU_RASR_AP_SET(reg, val) \
  (((reg) & ~ARMV7M_MPU_RASR_AP_MASK) | ARMV7M_MPU_RASR_AP(val))
#define ARMV7M_MPU_RASR_TEX_SHIFT 19
#define ARMV7M_MPU_RASR_TEX_MASK \
  ((0x7U) << ARMV7M_MPU_RASR_TEX_SHIFT)
#define ARMV7M_MPU_RASR_TEX(val) \
  (((val) << ARMV7M_MPU_RASR_TEX_SHIFT) & ARMV7M_MPU_RASR_TEX_MASK)
#define ARMV7M_MPU_RASR_TEX_GET(reg) \
  (((val) & ARMV7M_MPU_RASR_TEX_MASK) >> ARMV7M_MPU_RASR_TEX_SHIFT)
#define ARMV7M_MPU_RASR_TEX_SET(reg, val) \
  (((reg) & ~ARMV7M_MPU_RASR_TEX_MASK) | ARMV7M_MPU_RASR_TEX(val))
#define ARMV7M_MPU_RASR_S (1U << 18)
#define ARMV7M_MPU_RASR_C (1U << 17)
#define ARMV7M_MPU_RASR_B (1U << 16)
#define ARMV7M_MPU_RASR_SRD_SHIFT 8
#define ARMV7M_MPU_RASR_SRD_MASK \
  ((0xffU) << ARMV7M_MPU_RASR_SRD_SHIFT)
#define ARMV7M_MPU_RASR_SRD(val) \
  (((val) << ARMV7M_MPU_RASR_SRD_SHIFT) & ARMV7M_MPU_RASR_SRD_MASK)
#define ARMV7M_MPU_RASR_SRD_GET(reg) \
  (((val) & ARMV7M_MPU_RASR_SRD_MASK) >> ARMV7M_MPU_RASR_SRD_SHIFT)
#define ARMV7M_MPU_RASR_SRD_SET(reg, val) \
  (((reg) & ~ARMV7M_MPU_RASR_SRD_MASK) | ARMV7M_MPU_RASR_SRD(val))
#define ARMV7M_MPU_RASR_SIZE_SHIFT 1
#define ARMV7M_MPU_RASR_SIZE_MASK \
  ((0x1fU) << ARMV7M_MPU_RASR_SIZE_SHIFT)
#define ARMV7M_MPU_RASR_SIZE(val) \
  (((val) << ARMV7M_MPU_RASR_SIZE_SHIFT) & ARMV7M_MPU_RASR_SIZE_MASK)
#define ARMV7M_MPU_RASR_SIZE_GET(reg) \
  (((val) & ARMV7M_MPU_RASR_SIZE_MASK) >> ARMV7M_MPU_RASR_SIZE_SHIFT)
#define ARMV7M_MPU_RASR_SIZE_SET(reg, val) \
  (((reg) & ~ARMV7M_MPU_RASR_SIZE_MASK) | ARMV7M_MPU_RASR_SIZE(val))
#define ARMV7M_MPU_RASR_ENABLE (1U << 0)
  uint32_t rasr;

  uint32_t rbar_a1;
  uint32_t rasr_a1;
  uint32_t rbar_a2;
  uint32_t rasr_a2;
  uint32_t rbar_a3;
  uint32_t rasr_a3;
} ARMV7M_MPU;

typedef enum {
  ARMV7M_MPU_AP_PRIV_NO_USER_NO,
  ARMV7M_MPU_AP_PRIV_RW_USER_NO,
  ARMV7M_MPU_AP_PRIV_RW_USER_RO,
  ARMV7M_MPU_AP_PRIV_RW_USER_RW,
  ARMV7M_MPU_AP_PRIV_RO_USER_NO = 0x5,
  ARMV7M_MPU_AP_PRIV_RO_USER_RO,
} ARMV7M_MPU_Access_permissions;

typedef enum {
  ARMV7M_MPU_ATTR_R = ARMV7M_MPU_RASR_AP(ARMV7M_MPU_AP_PRIV_RO_USER_NO)
    | ARMV7M_MPU_RASR_C | ARMV7M_MPU_RASR_XN,
  ARMV7M_MPU_ATTR_RW = ARMV7M_MPU_RASR_AP(ARMV7M_MPU_AP_PRIV_RW_USER_NO)
    | ARMV7M_MPU_RASR_C | ARMV7M_MPU_RASR_XN | ARMV7M_MPU_RASR_B,
  ARMV7M_MPU_ATTR_RWX = ARMV7M_MPU_RASR_AP(ARMV7M_MPU_AP_PRIV_RW_USER_NO)
    | ARMV7M_MPU_RASR_C | ARMV7M_MPU_RASR_B,
  ARMV7M_MPU_ATTR_X = ARMV7M_MPU_RASR_AP(ARMV7M_MPU_AP_PRIV_NO_USER_NO)
    | ARMV7M_MPU_RASR_C,
  ARMV7M_MPU_ATTR_RX = ARMV7M_MPU_RASR_AP(ARMV7M_MPU_AP_PRIV_RO_USER_NO)
    | ARMV7M_MPU_RASR_C,
  ARMV7M_MPU_ATTR_IO = ARMV7M_MPU_RASR_AP(ARMV7M_MPU_AP_PRIV_RW_USER_NO)
    | ARMV7M_MPU_RASR_XN,
} ARMV7M_MPU_Attributes;

typedef enum {
  ARMV7M_MPU_SIZE_32_B = 0x4,
  ARMV7M_MPU_SIZE_64_B,
  ARMV7M_MPU_SIZE_128_B,
  ARMV7M_MPU_SIZE_256_B,
  ARMV7M_MPU_SIZE_512_B,
  ARMV7M_MPU_SIZE_1_KB,
  ARMV7M_MPU_SIZE_2_KB,
  ARMV7M_MPU_SIZE_4_KB,
  ARMV7M_MPU_SIZE_8_KB,
  ARMV7M_MPU_SIZE_16_KB,
  ARMV7M_MPU_SIZE_32_KB,
  ARMV7M_MPU_SIZE_64_KB,
  ARMV7M_MPU_SIZE_128_KB,
  ARMV7M_MPU_SIZE_256_KB,
  ARMV7M_MPU_SIZE_512_KB,
  ARMV7M_MPU_SIZE_1_MB,
  ARMV7M_MPU_SIZE_2_MB,
  ARMV7M_MPU_SIZE_4_MB,
  ARMV7M_MPU_SIZE_8_MB,
  ARMV7M_MPU_SIZE_16_MB,
  ARMV7M_MPU_SIZE_32_MB,
  ARMV7M_MPU_SIZE_64_MB,
  ARMV7M_MPU_SIZE_128_MB,
  ARMV7M_MPU_SIZE_256_MB,
  ARMV7M_MPU_SIZE_512_MB,
  ARMV7M_MPU_SIZE_1_GB,
  ARMV7M_MPU_SIZE_2_GB,
  ARMV7M_MPU_SIZE_4_GB
} ARMV7M_MPU_Size;

typedef struct {
  uint32_t rbar;
  uint32_t rasr;
} ARMV7M_MPU_Region;

#define ARMV7M_MPU_REGION_INITIALIZER(idx, addr, size, attr) \
  { \
    ((addr) & ARMV7M_MPU_RBAR_ADDR_MASK) \
      | ARMV7M_MPU_RBAR_VALID \
      | ARMV7M_MPU_RBAR_REGION(idx), \
    ARMV7M_MPU_RASR_SIZE(size) | (attr) | ARMV7M_MPU_RASR_ENABLE \
  }

#define ARMV7M_MPU_REGION_DISABLED_INITIALIZER(idx) \
  { \
    ARMV7M_MPU_RBAR_VALID | ARMV7M_MPU_RBAR_REGION(idx), \
    0 \
  }

/**
 * Higher level region configuration.
 *
 * Allows to configure with begin and end which is more convenient for
 * calculating the sizes from linker command file. Note that you still have to
 * follow the following rules:
 *
 * - Begin address has to be aligned to 0x20 (lower 5 bits set to 0)
 * - Sizes can only be a value of 2^x with a minimum of 32 Byte. If you have an
 *   end address that is not aligned, the region will get bigger.
 * - Later regions have higher priority.
 */
typedef struct {
  const void *begin;
  const void *end;
  uint32_t rasr;
} ARMV7M_MPU_Region_config;

typedef struct {
  uint32_t dhcsr;
  uint32_t dcrsr;
  uint32_t dcrdr;
#define ARMV7M_DEBUG_DEMCR_VC_CORERESET (1U << 0)
#define ARMV7M_DEBUG_DEMCR_VC_MMERR (1U << 4)
#define ARMV7M_DEBUG_DEMCR_VC_NOCPERR (1U << 5)
#define ARMV7M_DEBUG_DEMCR_VC_CHKERR (1U << 6)
#define ARMV7M_DEBUG_DEMCR_VC_STATERR (1U << 7)
#define ARMV7M_DEBUG_DEMCR_VC_BUSERR (1U << 8)
#define ARMV7M_DEBUG_DEMCR_VC_INTERR (1U << 9)
#define ARMV7M_DEBUG_DEMCR_VC_HARDERR (1U << 10)
#define ARMV7M_DEBUG_DEMCR_MON_EN (1U << 16)
#define ARMV7M_DEBUG_DEMCR_MON_PEND (1U << 17)
#define ARMV7M_DEBUG_DEMCR_MON_STEP (1U << 18)
#define ARMV7M_DEBUG_DEMCR_MON_REQ (1U << 19)
#define ARMV7M_DEBUG_DEMCR_TRCENA (1U << 24)
  uint32_t demcr;
} ARMV7M_DEBUG;

#define ARMV7M_DWT_BASE 0xe0001000
#define ARMV7M_SCS_BASE 0xe000e000
#define ARMV7M_ICTAC_BASE (ARMV7M_SCS_BASE + 0x0)
#define ARMV7M_SYSTICK_BASE (ARMV7M_SCS_BASE + 0x10)
#define ARMV7M_NVIC_BASE (ARMV7M_SCS_BASE + 0x100)
#define ARMV7M_SCB_BASE (ARMV7M_SCS_BASE + 0xd00)
#define ARMV7M_MPU_BASE (ARMV7M_SCS_BASE + 0xd90)
#define ARMV7M_DEBUG_BASE (ARMV7M_SCS_BASE + 0xdf0)

#define _ARMV7M_DWT \
  ((volatile ARMV7M_DWT *) ARMV7M_DWT_BASE)
#define _ARMV7M_ICTAC \
  ((volatile ARMV7M_ICTAC *) ARMV7M_ICTAC_BASE)
#define _ARMV7M_SCB \
  ((volatile ARMV7M_SCB *) ARMV7M_SCB_BASE)
#define _ARMV7M_Systick \
  ((volatile ARMV7M_Systick *) ARMV7M_SYSTICK_BASE)
#define _ARMV7M_NVIC \
  ((volatile ARMV7M_NVIC *) ARMV7M_NVIC_BASE)
#define _ARMV7M_MPU \
  ((volatile ARMV7M_MPU *) ARMV7M_MPU_BASE)
#define _ARMV7M_DEBUG \
  ((volatile ARMV7M_DEBUG *) ARMV7M_DEBUG_BASE)

#define ARMV7M_VECTOR_MSP 0
#define ARMV7M_VECTOR_RESET 1
#define ARMV7M_VECTOR_NMI 2
#define ARMV7M_VECTOR_HARD_FAULT 3
#define ARMV7M_VECTOR_MEM_MANAGE 4
#define ARMV7M_VECTOR_BUS_FAULT 5
#define ARMV7M_VECTOR_USAGE_FAULT 6
#define ARMV7M_VECTOR_SVC 11
#define ARMV7M_VECTOR_DEBUG_MONITOR 12
#define ARMV7M_VECTOR_PENDSV 14
#define ARMV7M_VECTOR_SYSTICK 15
#define ARMV7M_VECTOR_IRQ(n) ((n) + 16)
#define ARMV7M_IRQ_OF_VECTOR(n) ((n) - 16)

#define ARMV7M_EXCEPTION_PRIORITY_LOWEST 255

static inline bool _ARMV7M_Is_vector_an_irq( int vector )
{
  /* External (i.e. non-system) IRQs start after the SysTick vector. */
  return vector > ARMV7M_VECTOR_SYSTICK;
}

static inline uint32_t _ARMV7M_Get_basepri(void)
{
  uint32_t val;
  __asm__ volatile ("mrs %[val], basepri\n" : [val] "=&r" (val));
  return val;
}

static inline void _ARMV7M_Set_basepri(uint32_t val)
{
  __asm__ volatile ("msr basepri, %[val]\n" : : [val] "r" (val));
}

static inline uint32_t _ARMV7M_Get_primask(void)
{
  uint32_t val;
  __asm__ volatile ("mrs %[val], primask\n" : [val] "=&r" (val));
  return val;
}

static inline void _ARMV7M_Set_primask(uint32_t val)
{
  __asm__ volatile ("msr primask, %[val]\n" : : [val] "r" (val));
}

static inline uint32_t _ARMV7M_Get_faultmask(void)
{
  uint32_t val;
  __asm__ volatile ("mrs %[val], faultmask\n" : [val] "=&r" (val));
  return val;
}

static inline void _ARMV7M_Set_faultmask(uint32_t val)
{
  __asm__ volatile ("msr faultmask, %[val]\n" : : [val] "r" (val));
}

static inline uint32_t _ARMV7M_Get_control(void)
{
  uint32_t val;
  __asm__ volatile ("mrs %[val], control\n" : [val] "=&r" (val));
  return val;
}

static inline void _ARMV7M_Set_control(uint32_t val)
{
  __asm__ volatile ("msr control, %[val]\n" : : [val] "r" (val));
}

static inline uint32_t _ARMV7M_Get_MSP(void)
{
  uint32_t val;
  __asm__ volatile ("mrs %[val], msp\n" : [val] "=&r" (val));
  return val;
}

static inline void _ARMV7M_Set_MSP(uint32_t val)
{
  __asm__ volatile ("msr msp, %[val]\n" : : [val] "r" (val));
}

static inline uint32_t _ARMV7M_Get_PSP(void)
{
  uint32_t val;
  __asm__ volatile ("mrs %[val], psp\n" : [val] "=&r" (val));
  return val;
}

static inline void _ARMV7M_Set_PSP(uint32_t val)
{
  __asm__ volatile ("msr psp, %[val]\n" : : [val] "r" (val));
}

static inline uint32_t _ARMV7M_Get_XPSR(void)
{
  uint32_t val;
  __asm__ volatile ("mrs %[val], xpsr\n" : [val] "=&r" (val));
  return val;
}

static inline bool _ARMV7M_NVIC_Is_enabled( int irq )
{
  int index = irq >> 5;
  uint32_t bit = 1U << (irq & 0x1f);

  return (_ARMV7M_NVIC->iser [index] & bit) != 0;
}

static inline void _ARMV7M_NVIC_Set_enable( int irq )
{
  int index = irq >> 5;
  uint32_t bit = 1U << (irq & 0x1f);

  _ARMV7M_NVIC->iser [index] = bit;
}

static inline void _ARMV7M_NVIC_Clear_enable( int irq )
{
  int index = irq >> 5;
  uint32_t bit = 1U << (irq & 0x1f);

  _ARMV7M_NVIC->icer [index] = bit;
}

static inline bool _ARMV7M_NVIC_Is_pending( int irq )
{
  int index = irq >> 5;
  uint32_t bit = 1U << (irq & 0x1f);

  return (_ARMV7M_NVIC->ispr [index] & bit) != 0;
}

static inline void _ARMV7M_NVIC_Set_pending( int irq )
{
  int index = irq >> 5;
  uint32_t bit = 1U << (irq & 0x1f);

  _ARMV7M_NVIC->ispr [index] = bit;
}

static inline void _ARMV7M_NVIC_Clear_pending( int irq )
{
  int index = irq >> 5;
  uint32_t bit = 1U << (irq & 0x1f);

  _ARMV7M_NVIC->icpr [index] = bit;
}

static inline bool _ARMV7M_NVIC_Is_active( int irq )
{
  int index = irq >> 5;
  uint32_t bit = 1U << (irq & 0x1f);

  return (_ARMV7M_NVIC->iabr [index] & bit) != 0;
}

static inline void _ARMV7M_NVIC_Set_priority( int irq, int priority )
{
  _ARMV7M_NVIC->ipr [irq] = (uint8_t) priority;
}

static inline int _ARMV7M_NVIC_Get_priority( int irq )
{
  return _ARMV7M_NVIC->ipr [irq];
}

static inline bool _ARMV7M_DWT_Enable_CYCCNT( void )
{
  uint32_t demcr;
  uint32_t dwt_ctrl;

  demcr = _ARMV7M_DEBUG->demcr;
  _ARMV7M_DEBUG->demcr = demcr | ARMV7M_DEBUG_DEMCR_TRCENA;
  __asm__ volatile("dsb" : : : "memory");

  dwt_ctrl = _ARMV7M_DWT->ctrl;
  if ((dwt_ctrl & ARMV7M_DWT_CTRL_NOCYCCNT) == 0) {
    _ARMV7M_DWT->lar = ARMV7M_DWT_LAR_UNLOCK_MAGIC;
    __asm__ volatile("dsb" : : : "memory");
    _ARMV7M_DWT->ctrl = dwt_ctrl | ARMV7M_DWT_CTRL_CYCCNTENA;
    return true;
  } else {
    _ARMV7M_DEBUG->demcr = demcr;
    return false;
  }
}

static inline int _ARMV7M_Get_exception_priority( int vector )
{
  if ( _ARMV7M_Is_vector_an_irq( vector ) ) {
    return _ARMV7M_NVIC_Get_priority( ARMV7M_IRQ_OF_VECTOR( vector ) );
  } else if ( vector >= ARMV7M_VECTOR_MEM_MANAGE ) {
    return _ARMV7M_SCB->shpr [vector - 4];
  } else {
    return vector - 4;
  }
}

static inline void _ARMV7M_Set_exception_priority( int vector, int priority )
{
  if ( _ARMV7M_Is_vector_an_irq( vector ) ) {
    _ARMV7M_NVIC_Set_priority( ARMV7M_IRQ_OF_VECTOR( vector ), priority );
  } else if ( vector >= ARMV7M_VECTOR_MEM_MANAGE ) {
    _ARMV7M_SCB->shpr [vector - 4] = (uint8_t) priority;
  }
}

static inline void _ARMV7M_Set_exception_handler(int index,
    ARMV7M_Exception_handler handler) {
    extern void cpu_coherent_dcache(uintptr_t addr, size_t len);
    if ( _ARMV7M_SCB->vtor [index] != handler ) {
        _ARMV7M_SCB->vtor [index] = handler;
        cpu_coherent_dcache((uintptr_t)(&_ARMV7M_SCB->vtor[index]),
        sizeof(_ARMV7M_SCB->vtor[index]));
    }
}

static inline uint32_t _ARMV7M_MPU_Get_region_size(uintptr_t size)
{
  if ((size & (size - 1)) == 0) {
    return ARMV7M_MPU_RASR_SIZE(30 - __builtin_clz(size));
  } else {
    return ARMV7M_MPU_RASR_SIZE(31 - __builtin_clz(size));
  }
}

static inline void _ARMV7M_MPU_Set_region(
  volatile ARMV7M_MPU *mpu,
  uint32_t region,
  uint32_t rasr,
  const void *begin,
  const void *end
)
{
  uintptr_t size;
  uint32_t rbar;

  RTEMS_OBFUSCATE_VARIABLE(begin);
  RTEMS_OBFUSCATE_VARIABLE(end);
  size = (uintptr_t) end - (uintptr_t) begin;

  if ( (uintptr_t) end > (uintptr_t) begin ) {
    rbar = (uintptr_t) begin | region | ARMV7M_MPU_RBAR_VALID;
    rasr |= _ARMV7M_MPU_Get_region_size(size);
  } else {
    rbar = ARMV7M_MPU_RBAR_VALID | region;
    rasr = 0;
  }

  mpu->rbar = rbar;
  mpu->rasr = rasr;
}

static inline void _ARMV7M_MPU_Disable_region(
  volatile ARMV7M_MPU *mpu,
  uint32_t region
)
{
  mpu->rbar = ARMV7M_MPU_RBAR_VALID | region;
  mpu->rasr = 0;
}

static inline void _ARMV7M_MPU_Setup(
  uint32_t ctrl,
  const ARMV7M_MPU_Region_config *cfg,
  size_t cfg_count
)
{
  volatile ARMV7M_MPU *mpu;
  volatile ARMV7M_SCB *scb;
  uint32_t region_count;
  uint32_t region;

  mpu = _ARMV7M_MPU;
  scb = _ARMV7M_SCB;

  mpu->ctrl = 0;

  __asm__ volatile("dsb" : : : "memory");
  __asm__ volatile("isb" : : : "memory");

  region_count = ARMV7M_MPU_TYPE_DREGION_GET(mpu->type);

//   _Assert(cfg_count <= region_count);

  for (region = 0; region < cfg_count; ++region) {
    _ARMV7M_MPU_Set_region(
      mpu,
      region,
      cfg[region].rasr,
      cfg[region].begin,
      cfg[region].end
    );
  }

  for (region = cfg_count; region < region_count; ++region) {
    _ARMV7M_MPU_Disable_region(mpu, region);
  }

  mpu->ctrl = ctrl;
  scb->shcsr |= ARMV7M_SCB_SHCSR_MEMFAULTENA;

  __asm__ volatile("dsb" : : : "memory");
  __asm__ volatile("isb" : : : "memory");
}


#endif /* ASM */

#endif /* ARM_MULTILIB_ARCH_V7M */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTEMS_SCORE_ARMV7M_H */

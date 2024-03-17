/*
 * Copyright 2024 wtcat
 */
#include <errno.h>
#include <assert.h>
#include <string.h>

#include <board/armv7m.h>
#include <board/nvic.h>
#include <board/barriers.h>

#include "base/generic.h"
#include "tx_api.h"
#include "tx_thread.h"
#include "tx_initialize.h"
#include "tx_user.h"

#define BSP_ARMV7M_IRQ_PRIORITY_DEFAULT (13 << 4)
#define BSP_ARMV7M_SYSTICK_PRIORITY     (14 << 4)

typedef void (*ctor_fn)(void);

struct isr_entry {
    void (*handler)(void *arg);
    void *arg;
};

#ifndef __naked
#define __naked __attribute__((naked))
#endif

#define __fastfunc __section(".itcm")

extern char _vector_table_begin[];
extern char _vector_table_size[];

extern char _sdata_start[];
extern char _sdata_end[];
extern char _sdata_load_begin[];

extern char _itcm_text_start[];
extern char _itcm_text_end[];
extern char _itcm_text_load[];

extern char _bss_start[];
extern char _bss_end[];

extern char _dtcm_bss_start[];
extern char _dtcm_bss_end[];

extern ctor_fn _sinit[];
extern ctor_fn _einit[];

extern char _system_stack_top[];

extern VOID  _tx_timer_interrupt(VOID);

static struct isr_entry isr_entry_table[ARMV7M_SOC_IRQCNT] __section(".ram_vector");
#ifdef TX_ENABLE_RAM_VECTOR
static ARMV7M_Exception_handler ram_vector_table[16 + ARMV7M_SOC_IRQCNT] 
    __section(".ram_vector") __aligned(0x400);
#endif


static void armv7m_tcm_enable(void) {
  uint32_t regval;

  ARM_DSB();
  ARM_ISB();

  /* Enabled/disabled ITCM */
#ifdef CONFIG_ARMV7M_ITCM
  regval  = NVIC_TCMCR_EN | NVIC_TCMCR_RMW | NVIC_TCMCR_RETEN;
#else
  regval  = getreg32(NVIC_ITCMCR);
  regval &= ~NVIC_TCMCR_EN;
#endif
  putreg32(regval, NVIC_ITCMCR);

  /* Enabled/disabled DTCM */

#ifdef CONFIG_ARMV7M_DTCM
  /* As DTCM RAM on STM32H7 does not have ECC, so do not enable
   * read-modify-write.
   */

  regval  = NVIC_TCMCR_EN | NVIC_TCMCR_RETEN;
#else
  regval  = getreg32(NVIC_DTCMCR);
  regval &= ~NVIC_TCMCR_EN;
#endif
  putreg32(regval, NVIC_DTCMCR);

  ARM_DSB();
  ARM_ISB();
}

void __naked _armv7m_exception_handler(void) {
    while (1);
}

static void armv7m_default_isr(void *arg) {
    (void) arg;
}

static void armv7m_irq_init(void) {
    ARMV7M_Exception_handler *vector_table;
    
#ifdef TX_ENABLE_RAM_VECTOR
    vector_table = ram_vector_table;
    memcpy(ram_vector_table, _vector_table_begin, (size_t)_vector_table_size);
#else
    vector_table = (ARMV7M_Exception_handler *)_vector_table_begin;
#endif

    _ARMV7M_SCB->icsr = ARMV7M_SCB_ICSR_PENDSVCLR | ARMV7M_SCB_ICSR_PENDSTCLR;

    for (uint32_t i = 0; i < ARMV7M_SOC_IRQCNT; ++i) {
        _ARMV7M_NVIC_Clear_enable(i);
        _ARMV7M_NVIC_Clear_pending(i);
        _ARMV7M_NVIC_Set_priority(i, BSP_ARMV7M_IRQ_PRIORITY_DEFAULT);

        isr_entry_table[i].handler = armv7m_default_isr;
        isr_entry_table[i].arg = NULL;
    }

    /* Set exception priority */
    _ARMV7M_Set_exception_priority(ARMV7M_VECTOR_SVC, 
        ARMV7M_EXCEPTION_PRIORITY_LOWEST);
    _ARMV7M_Set_exception_priority(ARMV7M_VECTOR_PENDSV, 
        ARMV7M_EXCEPTION_PRIORITY_LOWEST);
    _ARMV7M_Set_exception_priority(ARMV7M_VECTOR_SYSTICK, 
        BSP_ARMV7M_SYSTICK_PRIORITY);

    /* Set base address for exception vector */
    _ARMV7M_SCB->vtor = vector_table;
}

void __fastfunc _armv7m_interrupt_dispatch(void) {
    const struct isr_entry *entry;
    uint32_t vector;
    
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    _tx_execution_isr_enter()
#endif

    vector = ARMV7M_SCB_ICSR_VECTACTIVE_GET(_ARMV7M_SCB->icsr);
    entry = isr_entry_table + ARMV7M_IRQ_OF_VECTOR(vector);
    entry->handler(entry->arg);

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    _tx_execution_isr_exit()
#endif
}

void __fastfunc _armv7m_clock_handler(void) {
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    _tx_execution_isr_enter()
#endif
    _tx_timer_interrupt();

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    _tx_execution_isr_exit()
#endif
}

int cpu_irq_attatch(int irq, void (*isr)(void *), void *arg, int prio) {
    TX_INTERRUPT_SAVE_AREA

    if (irq > ARMV7M_SOC_IRQCNT)
        return -EINVAL;

    if (isr == NULL)
        return -EINVAL;
    
    TX_DISABLE
    isr_entry_table[irq].arg = arg;
    isr_entry_table[irq].handler = isr;
    TX_RESTORE

    if (prio < 0)
        _ARMV7M_NVIC_Set_priority(irq, prio);
    return cpu_irq_enable(irq);
}

int cpu_irq_enable(int vector) {
    assert(vector <= ARMV7M_SOC_IRQCNT);
    _ARMV7M_NVIC_Set_enable(vector);
    return 0;
}

int cpu_irq_disable(int vector) {
    assert(vector <= ARMV7M_SOC_IRQCNT);
    _ARMV7M_NVIC_Clear_enable((int) vector);
    return 0;
}

void _tx_initialize_low_level(void) {
    _tx_initialize_unused_memory = NULL;
    _tx_thread_system_stack_ptr  = _system_stack_top;

    /* Enable the cycle count register */
    _ARMV7M_DWT_Enable_CYCCNT();

    /* Configure SysTick */
    uint32_t interval = TX_SYSTICK_FREQ / TX_TIMER_TICKS_PER_SECOND;
    _ARMV7M_Systick->rvr = interval - 1;
    _ARMV7M_Systick->cvr = 0;
    _ARMV7M_Systick->csr = ARMV7M_SYSTICK_CSR_ENABLE | 
                        ARMV7M_SYSTICK_CSR_CLKSOURCE | 
                        ARMV7M_SYSTICK_CSR_TICKINT;

    /* Initialize interrupt for periphs */
    armv7m_irq_init();
}

void __attribute__((noreturn)) _armv7m_crt_init(void) {
    /* Enable TCM */
    armv7m_tcm_enable();
    
    /* Clear bss section */
    memset(_bss_start, 0, _bss_end - _bss_start);

    /* Copy data section */
    memcpy(_sdata_start, _sdata_load_begin, _sdata_end - _sdata_start);

    /* Clear dtcm bss section */
    memset(_dtcm_bss_start, 0, _dtcm_bss_end - _dtcm_bss_start);

    /* Copy fast text section */
    memcpy(_itcm_text_start, _itcm_text_load, _itcm_text_end - _itcm_text_start);

    /* C++ ctor initialize */
    for (ctor_fn *p = _sinit; p < _einit; p++)
        (*p)();

    cpu_enable_icache();
    cpu_enable_dcache();

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    while (true);
}



/****************************************************************************
 * Public Functions
 ****************************************************************************/
#include <reent.h>
#include <sys/types.h>

int _isatty(int fd) {
    return -ENOSYS;
}

int _close_r(struct _reent *r, int fd) {
    return -ENOSYS;
}

int _fstat_r(struct _reent *r, int fd, struct stat *statbuf) {
    return -ENOSYS;
}

int _getpid_r(struct _reent *r) {
    return -ENOSYS;
}

int _kill_r(struct _reent *r, int pid, int sig) {
    return -ENOSYS;
}

int _link_r(struct _reent *r, const char *oldpath,
                      const char *newpath) {
    return -ENOSYS;
}

off_t _lseek_r(struct _reent *r, int fd, off_t offset, int whence) {
    return -ENOSYS;
}

int _open_r(struct _reent *r, const char *pathname,
                      int flags, int mode) {
    return -ENOSYS;
}

ssize_t _read_r(struct _reent *r, int fd, void *buf, size_t size) {
    return -ENOSYS;
}

int _rename_r(struct _reent *r, const char *oldpath,
                        const char *newpath) {
    return -ENOSYS;
}

void *_sbrk_r(struct _reent *r, ptrdiff_t increment) {
  /* TODO: sbrk is only supported on Kernel mode */
  errno = -ENOMEM;
  return (void *) -1;
}

int _stat_r(struct _reent *r, const char *pathname,
                      struct stat *statbuf) {
    return -ENOSYS;
}

int _unlink_r(struct _reent *r, const char *pathname) {
    return -ENOSYS;
}

ssize_t _write_r(struct _reent *r, int fd, const void *buf, size_t size) {
    return -ENOSYS;
}

void *_malloc_r(struct _reent *r, size_t size) {
    return NULL;
}

void *_realloc_r(struct _reent *r, void *ptr, size_t size) {
    return NULL;
}

void *_calloc_r(struct _reent *r, size_t nmemb, size_t size) {
    return NULL;
}

int isatty(int fd) {
    return -ENOSYS;
}

void _free_r(struct _reent *r, void *ptr) {}
void _abort(void) {}
struct _reent *__getreent(void) {
    return NULL;
}

int _system_r(struct _reent *r, const char *command) {
    /* TODO: Implement system() */
    return -ENOSYS;
}
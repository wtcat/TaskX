/*
 * CopyRight 2022 wtcat
 */
#include <rtems/rtems/intr.h>
#include "base/debug/cortexm/dwt.h"

#define WATCHPOINT_REG() _ARMV7M_DWT->comparator
#define MTX_LOCK(_lock)   rtems_interrupt_local_disable(_lock)
#define MTX_UNLOCK(_lock) rtems_interrupt_local_enable(_lock)

static int _watchpoint_nums;

static int __cortexm_dwt_get_hwbpks(void) {
	return (_ARMV7M_DWT->ctrl & ARMV7M_DWT_CTRL_NUMCOMP_Msk) >> ARMV7M_DWT_CTRL_NUMCOMP;
}

static void __cortexm_dwt_access(int ena) {
#if CONFIG_CORTEXM_CPU == 7
	uint32_t lsr = _ARMV7M_DWT->lsr;
	if ((lsr & 0x1) != 0) {
		if (!!ena) {
			if ((lsr & 0x2) != 0) 
				_ARMV7M_DWT->lar = 0xC5ACCE55; /* unlock it */
		} else {
			if ((lsr & 0x2) == 0) 
				_ARMV7M_DWT->lar = 0; /* Lock it */
		}
	}
#else
	(void) ena;
#endif
}

static int __cortexm_dwt_init(void) {
	_ARMV7M_DEBUG->demcr |= ARMV7M_DEBUG_DEMCR_TRCENA;
	__cortexm_dwt_access(1);
	return 0;
}

static int __cortexm_dwt_enable_monitor_exception(void) {
	 if (_ARMV7M_DEBUG->dhcsr & 0x1)
	 	return -EBUSY;
#if defined(CONFIG_ARMV8_M_SE) && !defined(CONFIG_ARM_NONSECURE_FIRMWARE)
	if (!(_ARMV7M_DEBUG->demcr & DCB_DEMCR_SDME_Msk))
		return -EBUSY;
#endif
	_ARMV7M_DEBUG->demcr |= ARMV7M_DEBUG_DEMCR_MON_EN;
	return 0;
}

int _cortexm_dwt_enable(int nr, void *addr, unsigned int mode) {
	if (nr > _watchpoint_nums)
		return -EINVAL;
	volatile ARMV7M_DWT_comparator *wp_regs = _ARMV7M_DWT->comparator;
	uint32_t func = (mode & 0xF) << ARMV7M_DWT_FUNCTION_DATAVSIZE;
    uint32_t level;
	MTX_LOCK(level);

#if (CONFIG_CORTEXM_CPU == 3) || \
	(CONFIG_CORTEXM_CPU == 4) || \
	(CONFIG_CORTEXM_CPU == 7)
	if (mode & MEM_READ)
		func |= (0x5 << ARMV7M_DWT_FUNCTION_FUNCTION);
	if (mode & MEM_WRITE)
		func |= (0x6 << ARMV7M_DWT_FUNCTION_FUNCTION);	
	wp_regs[nr].comp = (uint32_t)addr;
	wp_regs[nr].mask = 0;
	wp_regs[nr].function = func;
	
#elif (CONFIG_CORTEXM_CPU == 23) || \
	(CONFIG_CORTEXM_CPU == 33)
	if (mode & MEM_READ)
		func |= (0x6 << DWT_FUNCTION_MATCH_Pos);
	if (mode & MEM_WRITE)
		func |= (0x5 << DWT_FUNCTION_MATCH_Pos);	
	func |= (0x1 << DWT_FUNCTION_ACTION_Pos);
	wp_regs[nr].comp = (uint32_t)addr;
	wp_regs[nr].function = func;
#endif
	MTX_UNLOCK(level);
    return 0;
}

int _cortexm_dwt_disable(int nr) {
	if (nr > _watchpoint_nums)
		return -EINVAL;
	volatile ARMV7M_DWT_comparator *wp_regs = _ARMV7M_DWT->comparator;
    uint32_t level;
	MTX_LOCK(level);
	wp_regs[nr].function = 0;
	MTX_UNLOCK(level);
	return 0;
}

int _cortexm_dwt_busy(int nr) {
	volatile ARMV7M_DWT_comparator *wp_regs = _ARMV7M_DWT->comparator;
	return !!wp_regs[nr].function;
}

int _cortexm_dwt_setup(void) {
	int nr = __cortexm_dwt_get_hwbpks();
	__cortexm_dwt_init();
	for (int i = 0; i < nr; i++)
		_cortexm_dwt_disable(i);
	__cortexm_dwt_enable_monitor_exception();
	_watchpoint_nums = nr;
	return 0;
}

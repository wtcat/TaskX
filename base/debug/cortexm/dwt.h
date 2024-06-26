/*
 * CopyRight 2022 wtcat
 */
#ifndef DEBUG_CORTEXM_DWT_H_
#define DEBUG_CORTEXM_DWT_H_

#include <errno.h>
#include <stdint.h>

#include <rtems/score/armv7m.h>

#ifdef __cplusplus
extern "C"{
#endif

/* 
 * _ARMV7M_DWT Control Register Definitions 
 */
#define ARMV7M_DWT_CTRL_NUMCOMP               28U
#define ARMV7M_DWT_CTRL_NUMCOMP_Msk           (0xFUL << ARMV7M_DWT_CTRL_NUMCOMP)
#define ARMV7M_DWT_FUNCTION_DATAVSIZE         10U
#define ARMV7M_DWT_FUNCTION_DATAVSIZE_MSK     (0x3UL << ARMV7M_DWT_FUNCTION_DATAVSIZE)
#define ARMV7M_DWT_FUNCTION_FUNCTION          0U
#define ARMV7M_DWT_FUNCTION_FUNCTION_MSK      (0xFUL /*<< ARMV7M_DWT_FUNCTION_FUNCTION*/)

/*
 * User configure options
 */
#define MAX_BREAKPOINT_NR 8

/* 
 * Watchpoint mode 
 */
#define MEM_READ  0x0100  /* */
#define MEM_WRITE 0x0200
#define MEM_RW_MASK (MEM_READ | MEM_WRITE)
	
#define MEM_SIZE_1 0  /* 1 Byte */
#define MEM_SIZE_2 1  /* 2 Bytes */
#define MEM_SIZE_4 2  /* 4 Bytes */
#define MEM_SIZE_MASK 0xF

/*
 * Public interface
 */
#define cortexm_dwt_reinit() \
	_cortexm_dwt_setup()

#define cortexm_watchpoint_add(_nr, _addr, _mode) \
	_cortexm_dwt_enable(_nr, (void *)(_addr), _mode)
	
#define cortexm_watchpoint_del(_nr) \
	_cortexm_dwt_disable(_nr)
	
#define cortexm_watchpoint_is_busy(_nr) \
	_cortexm_dwt_busy(_nr)
	
/*
 * Protected interface
 */
int _cortexm_dwt_setup(void);
int _cortexm_dwt_disable(int nr);
int _cortexm_dwt_busy(int nr);
int _cortexm_dwt_enable(int nr, void *addr, unsigned int mode);

/*
 * Private interface
 */
static inline int cortexm_dwt_init_cycle_counter(void) {
	_ARMV7M_DWT->cyccnt = 0;
	_ARMV7M_DWT->ctrl |= ARMV7M_DWT_CTRL_CYCCNTENA;
	return (_ARMV7M_DWT->ctrl & ARMV7M_DWT_CTRL_NOCYCCNT) != 0;
}

static inline uint32_t cortexm_dwt_get_cycles(void) {
	return _ARMV7M_DWT->cyccnt;
}

static inline void cortexm_dwt_cycle_counter_start(void) {
	_ARMV7M_DWT->cyccnt = 0;
	_ARMV7M_DWT->ctrl |= ARMV7M_DWT_CTRL_CYCCNTENA;
}

#ifdef __cplusplus
}
#endif
#endif /* DEBUG_CORTEXM_DWT_H_ */

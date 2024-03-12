/*
 * Copyright 2023 wtcat
 *
 * Linux compatible definition 
 */
#ifndef BASE_LINUX_H_
#define BASE_LINUX_H_

#define CONFIG_LOGLEVEL   LOGLEVEL_WARNING //LOGLEVEL_DEBUG
#include <sys/param.h>

#include <rtems/fatal.h>
#include <rtems/bspIo.h>

#include "base/generic.h"
#include "base/log.h"

#ifdef __cplusplus
extern "C"{
#endif

#define LINUX_DEBUG 

#define KERN_CONT       ""
#define	KERN_EMERG	"<0>"
#define	KERN_ALERT	"<1>"
#define	KERN_CRIT	"<2>"
#define	KERN_ERR	"<3>"
#define	KERN_WARNING	"<4>"
#define	KERN_NOTICE	"<5>"
#define	KERN_INFO	"<6>"
#define	KERN_DEBUG	"<7>"

#define BUG() \
    rtems_panic("BUG***: %s: %d\n", __FILE__, __LINE__)
#define _WARNING() \
    printk("WARNING***: %s: %d\n", __FILE__, __LINE__)

#define	BUILD_BUG_ON(x)	RTEMS_STATIC_ASSERT(!(x), x)

#ifdef LINUX_DEBUG
#define BUG_ON(condition)	do { if (rte_unlikely(condition)) BUG(); } while(0)
#define	WARN_ON(condition)	do { if (rte_unlikely(condition)) _WARNING(); } while(0)
#else
#define BUG_ON(...)
#define	WARN_ON(...)
#endif /* LINUX_DEBUG */


#ifndef likely
# define likely(x) rte_likely(x)
#endif

#ifndef unlikely
# define unlikely(x) rte_unlikely(x)
#endif

#ifndef container_of
#define container_of rte_container_of
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE rte_array_size
#endif 

/*
 * The "pr_debug()" and "pr_devel()" macros should produce zero code
 * unless DEBUG is defined:
 */
#define pr_debug pr_dbg
#define pr_devel pr_dbg
#define pr_warning pr_warn

#define min(x, y)	rte_min(x, y)
#define max(x, y)	rte_max(x, y)

#define min3(a, b, c)	rte_min3(a, b, c)
#define max3(a, b, c)	rte_max3(a, b, c)

#define min_t(type, _x, _y)	rte_min_t(type, _x, _y)
#define max_t(type, _x, _y)	rte_max_t(type, _x, _y)

#define clamp_t(type, _x, min, max)	rte_clamp_t(type, _x, min, max)
#define clamp(x, lo, hi)   rte_clamp(x, lo, hi)


/*
 * This looks more complex than it should be. But we need to
 * get the type for the ~ right in round_down (it needs to be
 * as wide as the result!), and we want to evaluate the macro
 * arguments just once each.
 */
#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define	DIV_ROUND_UP(x, n)	howmany(x, n)
#define	DIV_ROUND_UP_ULL(x, n)	DIV_ROUND_UP((unsigned long long)(x), (n))
#define	FIELD_SIZEOF(t, f)	sizeof(((t *)0)->f)

#define	DIV_ROUND_CLOSEST(x, divisor) \
    (((x) + ((divisor) / 2)) / (divisor))

#define swap(a, b) rte_swap(a, b)

#define	cpu_relax() RTEMS_COMPILER_MEMORY_BARRIER()

#ifdef __cplusplus
}
#endif
#endif /* BASE_LINUX_H_ */
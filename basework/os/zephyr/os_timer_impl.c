/*
 * Copyright 2022 wtcat
 *
 * Timer implement for zephyr
 */
#define CONFIG_LOGLEVEL LOGLEVEL_INFO
#define pr_fmt(fmt) "<os_timer>: "fmt

#include <errno.h>
#include <assert.h>

#include <zephyr.h>
// #include <atomic.h>
#include <init.h>
#include <sys/atomic.h>
#include <drivers/hrtimer.h>

#include "basework/os/osapi_config.h"
#include "basework/os/osapi_timer.h"
#include "basework/lib/timer/timer_list.h"
#include "basework/log.h"
#include "basework/rq.h"

/* Enable timer merge */
#define CONFIG_TIMER_MERGE
#define SLAVE_TIMER_RES 250

#define HRTIMER_MS(n) (s32_t)((n) * 1000)

struct os_timer {
#define OS_TIMER_DELETED 0x1
    union {
        struct hrtimer timer;
        struct timer_list l_timer;
    };
    void (*task)(os_timer_t, void *);
#ifdef CONFIG_TIMER_MERGE
    void *pfn;
    void *parg;
    atomic_t slave;
#endif
    atomic_t rflags;
    void *pnode;
#ifdef CONFIG_OS_TIMER_TRACER
    bool trace;
#endif
};

#ifdef CONFIG_OS_TIMER_TRACER
#define PR_TRACE(_timer, _fmt, ...) \
    do { \
        if ((_timer)->trace) \
            pr_notice(_fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define PR_TRACE(_timer, _fmt, ...) \
    pr_dbg(_fmt, ##__VA_ARGS__); 

#endif /* CONFIG_OS_TIMER_TRACER */

#ifdef CONFIG_TIMER_MERGE
static struct hrtimer slave_timer;
#endif /* CONFIG_TIMER_MERGE */

static uint32_t boottimer_ticks __rte_section(".ram.noinit");
static void *timer_executing __rte_section(".ram.noinit");
static struct os_timer timer_objs[CONFIG_OS_MAX_TIMERS] __rte_section(".ram.noinit");
static struct os_robj timer_robj;

static void timer_task_wrapper(void *arg) {
    struct os_timer *timer = arg;
    PR_TRACE(timer, "timer task call: %p timer->task(%p) arg(%p)\n", 
        timer, timer->task, arg);
    assert(timer->task != NULL);
    if (unlikely(atomic_test_bit(&timer->rflags, OS_TIMER_DELETED))) {
        pr_dbg("The timer has been deleted (callback: %p)\n", timer->task);
        return;
    }
    rq_set_executing(timer->task);
#ifdef CONFIG_TIMER_MERGE
    timer->task(timer, timer->parg);
#else
    timer->task(timer, timer->timer.expiry_fn_arg);
#endif
}

static void __rte_unused timeout_adaptor(os_timer_t timer, void *arg) {
    struct os_timer *p = timer;
    (void)arg;
    rq_submit_ext_arg(timer_task_wrapper, timer, p->pnode, p->task);
    PR_TRACE(p, "timer submit: %p arg(%p) pnode(%p)\n", timer, arg, p->pnode);
}

#ifdef CONFIG_TIMER_MERGE
static void timer_slave_cb(struct hrtimer *timer,
    void *arg) {
    (void) timer;
    (void) arg;
    boottimer_ticks += SLAVE_TIMER_RES;
    timer_schedule(SLAVE_TIMER_RES);
}

static void timer_slave_start(struct os_timer *p, long expires) {
    if (!(expires % SLAVE_TIMER_RES)) {
        if (unlikely(atomic_cas(&p->slave, 0, 1))) {
            pr_dbg("initialize slave timer\n");
            hrtimer_stop(&p->timer);
            /* Not ISR context */
            if (p->task) {
                timer_init(&p->l_timer, 
                    (void *)timeout_adaptor, p->parg);
                pr_dbg("%s: no isr p->task(%p) p->pfn(%p) p->parg(%p)\n", 
                    __func__, p->task, p->pfn, p->parg);
            } else {
                timer_init(&p->l_timer, p->pfn, p->parg);
                pr_dbg("%s: p->task(%p) p->pfn(%p) p->parg(%p)\n", 
                    __func__, p->task, p->pfn, p->parg);
            }

            timer_add(&p->l_timer, expires);
        } else {
            timer_mod(&p->l_timer, expires);
        }
        
    } else {
        if (unlikely(atomic_cas(&p->slave, 1, 0))) {
            timer_del(&p->l_timer);
            if (p->task)
                hrtimer_init(&p->timer, 
                    (hrtimer_expiry_t)timeout_adaptor, p->parg);
            else
                hrtimer_init(&p->timer, 
                    (hrtimer_expiry_t)p->pfn, p->parg);
        }
        hrtimer_start(&p->timer, HRTIMER_MS(expires), 0);
    }
    PR_TRACE(p, "%s timer(%p) exipres(%ld) task(%p)\n", 
        __func__, p, expires, p->task);
}

static void timer_slave_del(struct os_timer *p) {
    if (atomic_cas(&p->slave, 1, 0)) {
        timer_del(&p->l_timer);
    } else {
        hrtimer_stop(&p->timer);
    }
}
#endif /* CONFIG_TIMER_MERGE */

int __os_timer_create(os_timer_t *timer, 
    void (*timeout_cb)(os_timer_t, void *), 
    void *arg, 
    bool isr_context) {
    struct os_timer *p;

    if (!timer || !timeout_cb)
        return -EINVAL;

    p = os_obj_allocate(&timer_robj);
    if (!p) {
        pr_err("Allocate timer object failed\n");
        return -ENOMEM;
    }

    *p = (struct os_timer){ 0 };
    if (!isr_context) {
        p->task = timeout_cb;
        timeout_cb = timeout_adaptor;
    }
#ifdef CONFIG_TIMER_MERGE
    p->pfn = p->task? p->task: timeout_cb;
    p->parg = arg; 
#endif /* CONFIG_TIMER_MERGE */
    pr_dbg("timer create: %p timer->task(%p) callback(%p) arg(%p)\n", 
        p, p->task, timeout_cb, arg);
    hrtimer_init(&p->timer, 
        (hrtimer_expiry_t)timeout_cb, arg);
    *timer = p;
    return 0;
}

int __os_timer_mod(os_timer_t timer, long expires) {
    struct os_timer *p = timer;
    if (p == NULL)
        return -EINVAL;

    atomic_clear(&p->rflags);
#ifdef CONFIG_TIMER_MERGE
    /* if timer need to be merged */
    timer_slave_start(p, expires);
#else    
    hrtimer_start(&p->timer, HRTIMER_MS(expires), 0);
#endif /* CONFIG_TIMER_MERGE */
    return 0;
}

int __os_timer_add(os_timer_t timer, long expires) {
    struct os_timer *p = timer;
    if (p == NULL)
        return -EINVAL;
    atomic_clear(&p->rflags);
#ifdef CONFIG_TIMER_MERGE
    /* if timer need to be merged */
    timer_slave_start(p, expires);
#else   
    hrtimer_start(&p->timer, HRTIMER_MS(expires), 0);
#endif
    return 0;
}

int __os_timer_del(os_timer_t timer) {
    struct os_timer *p = timer;
    if (p == NULL)
        return -EINVAL;
    if (atomic_cas(&p->rflags, 0, OS_TIMER_DELETED)) {
        if (p->pnode) {
            _rq_node_delete(_system_rq, (struct rq_node *)p->pnode);
            p->pnode = NULL;
        }
        PR_TRACE(p, "timer(%p) timer->task(%p) delete caller(%p): %p\n", 
            p, p->task, __builtin_return_address(0));
#ifdef CONFIG_TIMER_MERGE
        timer_slave_del(p);
#else
        hrtimer_stop(&p->timer);
#endif
    }
    return 0;
}

int __os_timer_update_handler(os_timer_t timer, 
    void (*fn)(os_timer_t, void *), void *arg) {
    assert(timer != NULL);
    struct os_timer *p = timer;
    if (rte_likely(fn)) {
        if (p->task) {
            p->task = fn;
#ifdef CONFIG_TIMER_MERGE
            p->parg = arg;
#else
            p->timer.expiry_fn_arg = arg;
#endif
        } else {
            p->timer.expiry_fn = (void *)fn;
            p->timer.expiry_fn_arg = arg;
        }
        return 0;
    }
    return -EINVAL;
}

int __os_timer_destroy(os_timer_t timer) {
    assert(timer != NULL);
    struct os_timer *p = timer;
#ifdef CONFIG_TIMER_MERGE
    timer_slave_del(p);
#else
    hrtimer_stop(&p->timer);
#endif
    os_obj_free(&timer_robj, p);
    return 0;
}

unsigned int __os_timer_gettime(void) {
    return RTE_READ_ONCE(boottimer_ticks);
}

void *__rte_notrace os_timer_executing(void) {
    return timer_executing;
}

#ifdef CONFIG_OS_TIMER_TRACER
int os_timer_trace(os_timer_t timer, bool enable) {
    struct os_timer *p = timer;
    if (p) {
        p->trace = enable;
        return 0;
    }
    return -EINVAL;
}
#endif

#ifdef CONFIG_TIMER_MERGE
static int os_timer_setup(const struct device *dev) {
    pr_dbg("slave timer setup\n");
    hrtimer_init(&slave_timer, timer_slave_cb, NULL);
    hrtimer_start(&slave_timer, HRTIMER_MS(SLAVE_TIMER_RES), 
        HRTIMER_MS(SLAVE_TIMER_RES));
    return 0;
}

SYS_INIT(os_timer_setup, APPLICATION,
    CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
#endif /* CONFIG_TIMER_MERGE */

static int os_timer_early_setup(const struct device *dev) {
    timer_executing = NULL;
    boottimer_ticks = 0;
    return os_obj_initialize(&timer_robj, timer_objs, 
        sizeof(timer_objs), sizeof(timer_objs[0]));
}

SYS_INIT(os_timer_early_setup, 
    PRE_KERNEL_2, 0);
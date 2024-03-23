/*
 * Copyright 2022 wtcat
 */
#define pr_fmt(fmt) "<system>: "fmt
#include <errno.h>
#include <assert.h>
#include <string.h>

#include "basework/os/osapi_timer.h"
#include "basework/generic.h"
#include "basework/env.h"
#include "basework/dev/blkdev.h"
#include "basework/log.h"


#define OBSERVER_CLASS_DEFINE
#include "basework/system.h"

#if defined (__ZEPHYR__)
static struct nvram_desc nvram_region __rte_section(".noinit.system.nvram");
#else
static struct nvram_desc nvram_region;
#endif

#define CRASH_HEADER 0xaccedcaf
#define CRASH_TIMEOUT 60  /* seconds */ 
#define MILLISECONDS(n) ((n) * 1000)
#define MINUTES(n) ((n) * 60)

OBSERVER_CLASS_INSTANCE(system)
static const struct system_operations *_system_operation;
static os_timer_t crash_timer;
static uint8_t expires_cnt;

/*
 * If the system runs more than 10 minutes, 
 * We consider the firmware to be normal
 */
static void crash_timer_cb(os_timer_t timer, void *arg) {
    (void) timer;
    (void) arg;
    if (++expires_cnt < 10) {
        os_timer_mod(crash_timer, MILLISECONDS(CRASH_TIMEOUT));
        return;
    }
    expires_cnt = 0;
    sys_crash_clear();
}

const struct system_operations * __rte_notrace sys_operation_get(void) {
    return _system_operation;
}

uint32_t sys_uptime_get(void) {
    return _system_operation->get_time_since_boot();
}

struct nvram_desc *__rte_notrace sys_nvram_get(void) {
    //TODO: fix critical problem (use atomic variable)
    struct nvram_desc *nvram = &nvram_region;

    if (nvram->magic != NVRAM_MAGIC) {
        nvram->magic = NVRAM_MAGIC;
        memset(nvram->__data_begin, 0, 
            nvram->__data_end - nvram->__data_begin);
    }
    return nvram;
}

bool sys_is_firmware_okay(void) {
    struct nvram_desc *nvram = sys_nvram_get();

    pr_notice("crash count: %d, crash_firsttime: %u\n", 
        nvram->crash.count, nvram->crash.start_time);
    
    if (nvram->crash.header != CRASH_HEADER) {
        memset(&nvram->crash, 0, sizeof(nvram->crash));
        nvram->crash.header = CRASH_HEADER;
        nvram->crash.start_time = _system_operation->get_utc();
        pr_warn("*** Crash detector is invalid ***\n");
    }

    if (nvram->crash.count >= 3) {
        uint32_t now = _system_operation->get_utc();
        uint32_t diff = now - nvram->crash.start_time;
        uint32_t rate = diff / nvram->crash.count;

        /* starting the recovery timer */
        os_timer_mod(crash_timer, MILLISECONDS(CRASH_TIMEOUT));

        /* If crash count large than 10 */
        pr_notice("crash time difference: %u\n", diff);
        if (rate <= MINUTES(3) || nvram->crash.count >= 5)
            return false;
    }
    
    return true;
}

void sys_crash_up(void) {
    struct nvram_desc *nvram = sys_nvram_get();

    if (!nvram->crash.count && _system_operation->get_utc)
        nvram->crash.start_time = _system_operation->get_utc();
    nvram->crash.count++;
}

void sys_crash_clear(void) {
    struct nvram_desc *nvram = sys_nvram_get();

    nvram->crash.header = 0;
}

void sys_shutdown(int reason, bool reboot) {
    const struct system_operations *ops = _system_operation; 
    struct shutdown_param param;
    struct nvram_desc *nvram;
    uint32_t run_time;

    assert(ops != NULL);

    sys_crash_clear();
    /* Update environment variable */
    nvram = sys_nvram_get();
    run_time = (uint32_t)(ops->get_time_since_boot() / 1000);
    if (reason == 0)
        env_unset("crash");
    env_setint("runtime", (int)run_time, 10);
    env_flush_ram(nvram->env_ram, sizeof(nvram->env_ram));
    
    /* Notify application */
    param.reason = reason;
    param.ptr = __builtin_return_address(0);
    system_notify(reboot? SYS_REPORT_REBOOT: SYS_REPORT_POWOFF, &param);

    /* Sync data to disk */
    blkdev_sync();
    
    if (reboot) 
        ops->reboot(reason);
    else
        ops->shutdown();

    pr_err("Never reached here!\n");
    while (1);
}

void sys_enter_tranport(void) {
    const struct system_operations *ops = _system_operation; 
    assert(ops != NULL);
    assert(ops->enter_transport != NULL);
    
    ops->enter_transport();
}

int __rte_notrace sysops_register(const struct system_operations *ops) {
    assert(ops != NULL);
    assert(ops->get_time_since_boot != NULL);
    assert(ops->reboot != NULL);
    assert(ops->shutdown != NULL);
    
    if (_system_operation)
        return -EBUSY;

    _system_operation = ops;
    return 0;
}

int __rte_notrace sys_screen_up(int sec) {
    const struct system_operations *ops = _system_operation; 
    if (ops->screen_on)
        return ops->screen_on(sec);
    return -ENOSYS;
}

int __rte_notrace sys_screen_down(void) {
    const struct system_operations *ops = _system_operation; 
    if (ops->screen_off)
        return ops->screen_off();
    return -ENOSYS;
}

bool __rte_notrace sys_is_screen_up(void) {
    const struct system_operations *ops = _system_operation; 
    if (ops->is_screen_up)
        return ops->is_screen_up();
    return false;
}

int __rte_notrace sys_startup(void) {
    return os_timer_create(&crash_timer, crash_timer_cb, 
        NULL, true);
}

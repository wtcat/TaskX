/*
 * Copyright 2023 wtcat
 */
#ifndef BASEWORK_MON_H_
#define BASEWORK_MON_H_

#include <stddef.h>
#include "basework/errno.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef void (*mon_show_t)(void *h, const char *, ...);

struct imon_class {
    void (*show)(void *h, mon_show_t cb, void *arg);
    void *arg;
    unsigned int period;

    struct imon_class *next;
};

#ifndef __MON_DECLARE__
# define __MON_DECLARE__ extern
#else
# define _MON_DECLARE
#endif

__MON_DECLARE__ struct imon_class *_mon_tasks;

static inline int monitor_register(struct imon_class *mon) {
	struct imon_class **p;

    if (!mon || !mon->show)
        return -EINVAL;

    /* 
     * Point to last node 
     */
    p = &_mon_tasks;
    while (*p != NULL)
        p = &(*p)->next;

    /* 
     * Append to list end 
     */
    mon->next = NULL;
    *p = mon;

    return 0;
}

#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_MON_H_ */

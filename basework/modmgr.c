/*
 * Copyright 2023 wtcat
 */
#include <stdbool.h>

#include "basework/os/osapi.h"
#include "basework/modmgr.h"


#define MTX_LOCK()   (void)pthread_mutex_lock(&_mtx)
#define MTX_UNLOCK() (void)pthread_mutex_unlock(&_mtx)
#define MTX_INIT()   (void)pthread_mutex_init(&_mtx, NULL)

static pthread_mutex_t _mtx;
static struct observer_base *_module_list;

int module_manager_register(struct module_node *m) {
    int err;
    MTX_LOCK();
	err = observer_cond_register(&_module_list, &m->base);
    MTX_UNLOCK();
    return err;
}

int module_manager_control(unsigned long action, 
    bool (*filter)(struct module_node *, void *), 
    void *p) {
	struct observer_base *nb, *next_nb;
    int ret = NOTIFY_DONE;

    MTX_LOCK();
	nb = _module_list;
	while (nb) {
		next_nb = nb->next;

        if (filter && !filter((struct module_node *)nb, p))
            goto _next;

		ret = nb->update(nb, action, p);
		if ((ret & NOTIFY_STOP_MASK) == NOTIFY_STOP_MASK)
			break;
    _next:
		nb = next_nb;
	}
    MTX_UNLOCK();
	return ret;
}

int module_manager_init(void) {
    MTX_INIT();
    return 0;
}
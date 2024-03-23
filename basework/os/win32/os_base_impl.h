/*
 * Copyright 2022 wtcat
 *
 * OS abstract layer
 */
#ifndef BASEWORK_OS_POSIX_OS_BASE_H_
#define BASEWORK_OS_POSIX_OS_BASE_H_

#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "basework/generic.h"
#ifdef __cplusplus
extern "C" {
#endif

#define os_in_isr() false

#define os_panic(...) assert(0)

	/* Critical lock */
#define os_critical_global_declare  static pthread_mutex_t __mutex = PTHREAD_MUTEX_INITIALIZER;
#define os_critical_declare
#define os_critical_lock   pthread_mutex_lock(&__mutex);
#define os_critical_unlock pthread_mutex_unlock(&__mutex);

/* Completion */
#define os_completion_declare(_cp) sem_t _cp;
#define os_completion_reinit(_cp)  sem_init(&(_cp), 0, 0)
#define os_completion_wait(_cp)    sem_wait(&(_cp))
#define os_completed(_cp)          sem_post(&(_cp))

/*
 * Just prevent compile error
 */
typedef struct {
	int unused;
} os_thread_t;

#define os_cond_t  os_thread_t
#define os_mutex_t os_thread_t

#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_OS_POSIX_OS_BASE_H_ */

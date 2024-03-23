/*
 * Copyright 2022 wtcat
 *
 * OS abstract layer
 */
#ifndef BASEWORK_OS_POSIX_OS_BASE_H_
#define BASEWORK_OS_POSIX_OS_BASE_H_

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "basework/generic.h"
#ifdef __cplusplus
extern "C"{
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
  * Posix platform 
 */
#define OS_THREAD_API static inline
#define OS_MTX_API    static inline

typedef struct {
    pthread_t thread;
    struct {
        void (*entry)(void *);
        void *arg;
    } adapter;
} os_thread_t;

typedef struct {
    pthread_mutex_t mtx;
} os_mutex_t;

typedef struct {
    pthread_cond_t cv;
} os_cond_t;

extern int pthread_setname_np (pthread_t __target_thread, const char *__name)
     __THROW __nonnull ((2));

static inline void *
posix_thread_entry(void *arg) {
    os_thread_t *thr = (os_thread_t *)arg;
    thr->adapter.entry(thr->adapter.arg);
    return NULL;
}

OS_THREAD_API int 
_os_thread_spawn(os_thread_t *thread, const char *name, 
    void *stack, size_t size, 
    int prio, os_thread_entry_t entry, void *arg) {
    struct sched_param param;
    pthread_attr_t attr;
    int err;

    param.sched_priority = prio;
    pthread_attr_init(&attr);
#ifndef _WIN32
    if (stack)
        pthread_attr_setstack(&attr, stack, size);
#endif
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setschedparam(&attr, &param);
    thread->adapter.entry = entry;
    thread->adapter.arg = arg;
    err = pthread_create(&thread->thread, &attr, posix_thread_entry, thread);
    if (!err)
        pthread_setname_np(thread->thread, name);
    return err;
}

OS_THREAD_API int 
_os_thread_destroy(os_thread_t *thread) {
    return -ENOSYS;
}

OS_THREAD_API int 
_os_thread_change_prio(os_thread_t *thread, int newprio, 
    int *oldprio) {
    if (oldprio) {
        *oldprio = 0;
    }
    return pthread_setschedprio(thread->thread, newprio);
}

OS_THREAD_API int 
_os_thread_sleep(uint32_t ms) {
    struct timespec t;
    t.tv_sec = ms / 1000;
    t.tv_nsec = ms * 1000000;
    return nanosleep(&t, NULL);
}

OS_THREAD_API void 
_os_thread_yield(void) {
    // pthread_yield();
}

OS_THREAD_API void*
_os_thread_self(void) {
    return (void *)pthread_self();
}

OS_MTX_API int 
_os_mtx_init(os_mutex_t *mtx, int type) {
    (void) type;
    pthread_mutexattr_t _attr;
    pthread_mutexattr_init(&_attr);
    pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
    return pthread_mutex_init(&mtx->mtx, &_attr);
}

OS_MTX_API int 
_os_mtx_destroy(os_mutex_t *mtx) {
    return pthread_mutex_destroy(&mtx->mtx);
}

OS_MTX_API int 
_os_mtx_lock(os_mutex_t *mtx) {
    return pthread_mutex_lock(&mtx->mtx);
}

OS_MTX_API int 
_os_mtx_unlock(os_mutex_t *mtx) {
    return pthread_mutex_unlock(&mtx->mtx);
}

OS_MTX_API int 
_os_mtx_timedlock(os_mutex_t *mtx, uint32_t timeout) {
    return -ENOSYS;
}

OS_MTX_API int 
_os_mtx_trylock(os_mutex_t *mtx) {
    return pthread_mutex_trylock(&mtx->mtx);
}


#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_OS_POSIX_OS_BASE_H_ */

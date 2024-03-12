/*
 * Copyright 2023 wtcat
 */
#ifndef BASEWORK_HOOK_H_
#define BASEWORK_HOOK_H_

#include "base/compiler.h"

#ifdef __cplusplus
extern "C"{
#endif

/*
 * Declare rte-hook interface
 *
 * @name: module name
 * @params: parameters type list
 */
#define RTE_HOOK_DECLARE(name, params...) \
    __RTE_HOOK_DECLARE(name, void, params)

/* Define hook instance */
#define RTE_HOOK_INSTANCE(name, _attr) \
    name##_hook_t __rte_##name##_application_hook _attr;

/* Set hook function */
#define RTE_HOOK_SET(name, hook) \
    name##_set_application_hook(hook)

/* Get hook function */
#define RTE_HOOK_GET(name) \
    name##_get_application_hook()

/* Call hook function */
#define RTE_HOOK(name, ...) \
do { \
    if (__rte_##name##_application_hook) \
        __rte_##name##_application_hook(__VA_ARGS__); \
} while (0)

#define __RTE_HOOK_DECLARE(name, rettype, params...) \
    typedef rettype (*name##_hook_t)(params); \
    extern name##_hook_t __rte_##name##_application_hook; \
    static __rte_inline name##_hook_t name##_get_application_hook(void) { \
        return __rte_##name##_application_hook; \
    } \
    static __rte_inline name##_hook_t name##_set_application_hook(name##_hook_t fn) { \
        name##_hook_t old_fn; \
        old_fn = __rte_##name##_application_hook; \
        __rte_##name##_application_hook = fn; \
        return old_fn; \
    }

/*
 * Static hook-function definition
 */

/* Overwrite hook */
#define RTE_OVERHOOK(name, param...) name(param)

/* Default hook */
#define RTE_WEAKHOOK(name, param...)  __rte_weak name(param)

#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_HOOK_H_ */
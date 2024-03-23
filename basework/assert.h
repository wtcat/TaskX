/*
 * Copyright 2023 wtcat
 */

#ifndef BASEWORK_ASSERT_H_
#define BASEWORK_ASSERT_H_

#ifdef __cplusplus
extern "C"{
#endif

#ifndef CONFIG_RTE_ASSERT_DISABLE
void __assert_failed(const char *file, int line, const char *func, const char *failedexpr);
# define RTE_ASSERT(_e) \
    ((_e) ? ( void ) 0 : __assert_failed(__FILE__, __LINE__, __func__, #_e))
#else
# define RTE_ASSERT(...) (void) 0
#endif /* CONFIG_RTE_ASSERT_DISABLE */

#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_ASSERT_H_ */

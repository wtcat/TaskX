/*
 * Copyright 2022 wtcat
 */
#ifndef BASEWORK_PRINTER_H_
#define BASEWORK_PRINTER_H_

#include <stddef.h>
#include <stdarg.h>

#include "base/compiler.h"

#ifdef __cplusplus
extern "C"{
#endif

struct printer {
    void *context;
    int (*format)(void *context, const char *fmt, va_list ap);
};

/*
 * virt_format - Virtual format output
 * @printer: printer object
 * @fmt: format string
 * return the number of bytes that has been output
 */
static inline int virt_format(struct printer *printer, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int len = printer->format(printer->context, fmt, ap);
    va_end(ap);
    return len;
}

/*
 * printf_format_init - Initialize a stdio(printf) printer
 * @pr: printer object
 */
void printf_format_init(struct printer *pr);

#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_PRINTER_H_ */

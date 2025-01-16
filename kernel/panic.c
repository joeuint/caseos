#include <stdarg.h>

#include "print.h"

/*
 * Panic with formatting. Behaves like all -f c functions
 */
void panicf(const char* format, ...) {
    va_list vargs;
    va_start(vargs, format);

    printk("Panic: %s\n", format, vargs);

    va_end(vargs);

    __asm__ volatile("wfi");
}

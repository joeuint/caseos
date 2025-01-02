#include <stdarg.h>

/*
 * Panic with formatting. Behaves like all -f c functions
 */
void panicf(const char* format, ...) {
    va_list vargs;

    asm volatile("wfi");
}

#include <stdarg.h>
#include <limits.h>
#include <stddef.h>

#include "uart.h"

void printk(const char* format, ...) {
    uart_print(format);
}

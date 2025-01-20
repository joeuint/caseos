#include <stdarg.h>
#include <limits.h>
#include <stddef.h>

#include "uart.h"

void printk(const char* format, ...) {
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] != '%') {
            uart_putch(format[i]);
            continue;
        }

        i++;

        if (format[i] == '\0') {
            break;
        }

        if (format[i] == '%') {
            uart_putch('%');
            continue;
        }

        if (format[i] == 'd') {
            int val = va_arg(args, int);
            
            // Perfect amount of space for INT_MAX
            // Excl. the sign, it's printed serparately
            char result[11];

            if (val < 0) {
                uart_putch('-');
                val = -val;
            }
            
            int j = 0;
            do {
                result[j++] = '0' + (val % 10);
            } while (val /= 10);
            
            // Print it backwards.
            // This is what uart_print does anyway,
            // but backwards
            for (j--; j >= 0; j--) {
                uart_putch(result[j]);
            }

            continue;
        }

        if (format[i] == 's') {
            uart_print(va_arg(args, const char*));

            continue;
        }

    }

    va_end(args);

    // this is logging, newlines are default
    uart_putch('\n');
}

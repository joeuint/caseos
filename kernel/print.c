#include <stdarg.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "uart.h"

/*
 * @brief Convert a numeric type to base 10 string (incl. sign)
 *
 * Please allocate enough space for the result. If in doubt, 20 bytes
 * will always work (64 bit uint).
 *
 * To print the result, print each character in the buffer in reverse order, 
 * decrement return value, and repeat until less than 0.
 *
 * For example:
 * @code{.c}
 * char result[21];
 * int j = print_numeric(val, result, 21);
 * for (j--; j >= 0; j--) {
 *    putch(result[j]); // or whatever you use to print 1 character
 * }
 * @endcode
 *
 * @param val The absolute value of the number.
 * @param res The buffer to store the result.
 * @param n The size of the buffer.
 *
 * @return The number of characters written to the buffer.
 */
int print_numeric(uint64_t val, char* res, int n) {
    int j = 0;
    do {
        res[j++] = '0' + (val % 10);
    } while ((val /= 10) && (j < n)); // Thanks order of operations! Very cool!
    return j;
}

/*
 * @brief Convert a numeric type to base 16 string (incl. sign)
 *
 * Please allocate enough space for the result. If in doubt, 16 bytes
 * will always work (64 bit uint).
 *
 * To print the result, print each character in the buffer in reverse order, 
 * decrement return value, and repeat until less than 0.
 *
 * Also note that this function does not include the "0x" prefix
 *
 * For example:
 * @code{.c}
 * fputs(stdout, "0x"); // or whatever you use to print strings
 * char result[16];
 * int j = print_hex(val, result, 16);
 * for (j--; j >= 0; j--) {
 *    putch(result[j]); // or whatever you use to print 1 character
 * }
 * @endcode
 *
 * @param val The absolute value of the number.
 * @param res The buffer to store the result.
 * @param n The size of the buffer.
 *
 * @return The number of characters written to the buffer.
 */
int print_hex(uint64_t val, char* res, int n) {
    int j = 0;
    do {
        res[j++] = "0123456789abcdef"[val % 16];
    } while ((val /= 16) && (j < n));
    return j;
}


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
        
        // Signed Integer Types
        if (format[i + 1] != '\0' && format[i + 2] != '\0' && format[i] == 'l'
                && format[i + 1] == 'l' && format[i + 2] == 'd') {
            i += 2;

            long long val = va_arg(args, long long);
            
            char result[21];

            if (val < 0) {
                uart_putch('-');
                val = -val;
            }
            
            int j = print_numeric(val, result, 21);
            
            for (j--; j >= 0; j--) {
                uart_putch(result[j]);
            }

            i++;

            continue;
        }

        if (format[i + 1] != '\0' && format[i] == 'l' && format[i + 1] == 'd') {
            i++;

            long val = va_arg(args, long);
            
            char result[21];

            if (val < 0) {
                uart_putch('-');
                val = -val;
            }
            
            int j = print_numeric(val, result, 21);
            
            for (j--; j >= 0; j--) {
                uart_putch(result[j]);
            }

            i++;

            continue;
        }

        if (format[i] == 'd') {
            int val = va_arg(args, int);
            
            char result[21];

            if (val < 0) {
                uart_putch('-');
                val = -val;
            }
            
            int j = print_numeric(val, result, 21);
            
            for (j--; j >= 0; j--) {
                uart_putch(result[j]);
            }

            continue;
        }

        // Unsigned Integer Types
        if (format[i + 1] != '\0' && format[i + 2] != '\0' && format[i] == 'l'
                && format[i + 1] == 'l' && format[i + 2] == 'u') {
            i += 2;

            unsigned long long val = va_arg(args, unsigned long long);
            
            char result[21];
            
            int j = print_numeric(val, result, 21);
            
            for (j--; j >= 0; j--) {
                uart_putch(result[j]);
            }

            continue;
        }

        if (format[i + 1] != '\0' && format[i] == 'l' && format[i + 1] == 'u') {
            i++;

            unsigned long val = va_arg(args, unsigned long);
            
            char result[21];
            
            int j = print_numeric(val, result, 21);
            
            for (j--; j >= 0; j--) {
                uart_putch(result[j]);
            }

            continue;
        }

        if (format[i] == 'u') {
            unsigned int val = va_arg(args, unsigned int);
            
            char result[21];
            
            int j = print_numeric(val, result, 21);
            
            for (j--; j >= 0; j--) {
                uart_putch(result[j]);
            }

            continue;
        }

        // Pointer
        if (format[i] == 'p') {
            uintptr_t val = va_arg(args, uintptr_t);
            // TODO: This is probably overkill
            // Document and fix accordingly later
            char result[16];

            int j = print_hex(val, result, 16);

            // print hex prefix
            uart_print("0x");

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

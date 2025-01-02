#include <stddef.h>

#include "uart.h"

unsigned char * uart = (unsigned char*)UART_ADDR;

void uart_init(void) {
    *UART_REGISTER(IER_OFFSET) = 0x00; // Disable interupts
}

void uart_putch(char c) {
    *UART_REGISTER(0) = c;
    return;
}

void uart_print(const char * str) {
    while(*str != '\0') {
		uart_putch(*str);
		str++;
	}
	return;
}

void uart_debug() {

}

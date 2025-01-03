#include <stddef.h>
#include <string.h>

#include "uart.h"

#define to_nibble(n) ('0' + (n) + ((n) < 10 ? 0 : 'a' - '0' - 10))

unsigned char * uart = (unsigned char*)UART_ADDR;

void char_as_hex(char* buf, char c) { // Unsafe AF. Please never use
}

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

void uart_debug(void) {
    char c = uart_getch();

    if (c == 0)
        return;

    char p1 = to_nibble(c % 16);

    c /= 16;

    char p2 = to_nibble(c % 16);

    char buf[3];
    
    buf[0] = p2;
    buf[1] = p1;
    buf[2] = '\0';

    uart_print(buf);

    uart_putch('\n');
}



char uart_getch() {
    if (*UART_REGISTER(LSR_OFFSET) & 0x01) {
        char ch = *UART_REGISTER(RBR_OFFSET);

        return ch;
    }

    return 0;
}

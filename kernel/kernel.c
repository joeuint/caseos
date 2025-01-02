#include <stdint.h>
#include <stddef.h>

#include "uart.h"
 
void kmain(void) {
	uart_print("Hello world!\r\n");
	while(1) {
        if (*UART_REGISTER(LSR_OFFSET) & 0x01) {
            uart_putch((unsigned char)*UART_REGISTER(RBR_OFFSET));
        }
	}
}

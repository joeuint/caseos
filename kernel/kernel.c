#include <stdint.h>
#include <stddef.h>

#include "uart.h"


void kmain(void) {
	uart_print("Hello world!\n");
	while(1) {
        uart_debug();
	}
}

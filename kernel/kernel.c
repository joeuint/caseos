#include <stdint.h>
#include <stddef.h>

#include "uart/uart.h"
 
void kmain(void) {
	uart_print("Hello world!\r\n");
	while(1) {
        // empty for now
	}
	return;
}

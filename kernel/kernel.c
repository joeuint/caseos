#include <stdint.h>
#include <stddef.h>

#include "uart.h"
#include "block.h"

void kmain(void) {
    init_block();

	uart_print("Hello world!\n");
	while(1) {
        uart_debug();
	}
}

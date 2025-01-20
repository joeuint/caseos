#include <stdint.h>
#include <stddef.h>

#include "uart.h"
#include "block.h"
#include "print.h"

void kmain(void) {
    init_block();

	printk("Hello world!");
	while(1) {
        uart_debug();
	}
}

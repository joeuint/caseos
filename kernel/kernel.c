#include <stdint.h>
#include <stddef.h>

#include "uart.h"
#include "block.h"
#include "print.h"

// Printed twice
// Once before init and once after
void print_notice() {
    printk("(C) 2025 Joseph Umana et al.");
    printk("This software is licensed under the GNU General Public License v2.0 or later.");
    printk("Some exceptions apply. See COPYING for full details.");
    printk("This software comes with ABSOLUTELY NO WARRANTY.");
}

void kmain(void) {
    print_notice();

    init_block();

    print_notice();
    printk("Hello world!");
	while(1) {
        uart_debug();
	}
}

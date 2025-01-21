#include <stdint.h>
#include <stddef.h>

#include "uart.h"
#include "block.h"
#include "print.h"
#include "alloc.h"
#include "panic.h"

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

    printk("kmain: initializing kernel heap");
    init_memory();
    printk("kmain: finished initializing kernel heap");

    init_block();

    char* str = kalloc();
    str[0] = 'H';
    str[1] = 'e';
    str[2] = 'l';
    str[3] = 'l';
    str[4] = 'o';
    str[5] = '\0';

    printk("%s", str);

    if (kfree_s(str))
        panicf("Failed to free testing string");
    
    print_notice();
    printk("Hello world!");
	while(1) {
        uart_debug();
	}
}

#pragma once

#include <stddef.h>

#define UART_ADDR 0x10000000
#define UART_REGISTER(reg) ((volatile unsigned char *)(UART_ADDR + (reg)))

// UART data registers
#define RBR_OFFSET 0x00
#define IER_OFFSET 0x01
#define LSR_OFFSET 0x05

// UART LSR BITS
#define BACKSPACE 0x08

void uart_init(void);

/* 
 * Writes a char to uart
 */
void uart_putch(char c);

/*
 * Writes a NUL terminated string to uart
 */
void uart_print(const char * str);

/*
 * Prints the hex code of the key pressed.
 *
 * *DO NOT USE EXCEPT FOR TEMP DEBUGGING*
 */
void uart_debug(void) 
    __attribute__((warning("Debugging function in use")));


char uart_getch();

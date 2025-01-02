#pragma once

#include <stddef.h>

#define UART_ADDR 0x10000000
#define UART_REGISTER(reg) ((volatile unsigned char *)(UART_ADDR + (reg)))

// UART data registers
#define RBR_OFFSET 0x00
#define IER_OFFSET 0x01
#define LSR_OFFSET 0x05

// UART LSR BITS

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
 * Prints each register of uart. May be removed at anytime
 *
 * *DO NOT USE EXCEPT FOR TEMP DEBUGGING*
 */
void uart_debugprint(void);

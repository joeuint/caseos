#pragma once

#include <stddef.h>

#define UART_ADDR 0x10000000
#define UART_REGISTER(reg) ((volatile unsigned char *)(UART_ADDR + (reg)))

void uart_init(size_t uart_addr);

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
void uart_debugprint();

// SPDX-License-Identifier: GPL-2.0-or-later
// Author: Joseph Umana
// Date: 2025-01-19
//
// A simple fixed memory allocator.
#include <stdbool.h>
#include <stdint.h>

#include "panic.h"
#include "alloc.h"
#include "print.h"
#include "string.h"

// TODO: Implement locks
struct block {
    struct block* next;
};

// Global variable that tracks allocated memory
struct {
    struct block* free;
} kernel_heap = {0};

void init_memory() {
    char* ptr = (char*)PAGE_START;

    printk("alloc: starting at %p", (void*)ptr);
    printk("alloc: ending at %p", (void*)PAGE_END);

    uint64_t alloced = 0;

    for (; ptr + PAGE_SIZE <= (char*)PAGE_END; ptr += PAGE_SIZE) {
        #ifdef DEBUG
            // Print every 100th allocation
            // printk is *way* too slow.
            if (alloced % 100 == 0) {
                printk("alloc: allocated %lu pages", alloced);
            }
        #endif
        if (kfree(ptr))
            panicf("Initial memory free failed");

        alloced++;
    }

    printk("alloc: allocated %lu pages", alloced);
}

int kfree_s(void* ptr) {
    if ((uint64_t)ptr % PAGE_SIZE != 0)
        return -1;

    if (ptr < (void*)PAGE_START)
        return -1;

    if (ptr >= (void*)PAGE_END)
        return -1;
    
    // Zeros the page
    memset_s(ptr, 0, PAGE_SIZE);

    struct block* block = (struct block*)ptr;
    block->next = kernel_heap.free;
    kernel_heap.free = block;

    return 0;
}

int kfree(void* ptr)  {
    struct block* block = (struct block*)ptr;

    if ((uint64_t)ptr % PAGE_SIZE != 0)
        return -1;

    if (ptr < (void*)PAGE_START)
        return -1;

    if (ptr >= (void*)PAGE_END)
        return -1;

    block->next = kernel_heap.free;
    kernel_heap.free = block;

    return 0;
}

void* kalloc() {
    struct block* block = kernel_heap.free;

    if (!block)
        panicf("Out of memory");

    kernel_heap.free = block->next;

    return (void*)block;
}


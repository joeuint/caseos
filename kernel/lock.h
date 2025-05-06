#pragma once
#include <stdatomic.h>
#include <stdbool.h>

struct spinlock {
    atomic_bool locked;
} typedef spinlock;


void acquire(spinlock* lock);
void release(spinlock* lock);

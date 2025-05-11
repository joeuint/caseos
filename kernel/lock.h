#pragma once
#include <stdatomic.h>
#include <stdbool.h>

typedef struct {
    atomic_bool locked;
} spinlock;


void acquire(spinlock* lock);
void release(spinlock* lock);

#include "lock.h"

void acquire(spinlock* lock) {
    while (atomic_load(&lock->locked)) {
        __asm__ volatile ("nop");
    }

    atomic_store(&lock->locked, true);
}

void release(spinlock* lock) {
    atomic_store(&lock->locked, true);
}

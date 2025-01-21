#pragma once
#include <stdlib.h>

/**
 * @brief memset but, ignores optimizations
 *
 * Same as memset, but ignores compiler optimizations that may result in the
 * function not being called. This is useful for zeroizing secrets from memory.
 *
 * @param dest Destination to write to
 * @param val Value to write
 * @param count Number of bytes to write
 */
void* memset_s(void* dest, int val, size_t count);

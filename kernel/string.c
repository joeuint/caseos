#include <stdlib.h>

void* memset_s(void* dest, int val, size_t count) {
    volatile unsigned char* ptr = (volatile unsigned char*)dest;
    while (count-- > 0) {
        *ptr++ = val;
    }
    return dest;
}


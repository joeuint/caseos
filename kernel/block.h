#pragma once
#include <stdint.h>

void init_block();
void virtio_blk_write(volatile uint8_t* data, volatile uint64_t sector);

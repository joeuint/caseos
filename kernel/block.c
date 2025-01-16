#include <stdint.h>
#include <stdlib.h>
#include "panic.h"
#include "print.h"

#define BLOCK_ADDRESS 0x10001000

#define BLOCK_REG(x) ((volatile uint32_t *)(BLOCK_ADDRESS + (x)))

#define VIRTIO_MAGIC 0x74726976
#define VIRTIO_VERSION 0x01

#define VIRTIO_MAGIC_OFFSET 0x00
#define VIRTIO_VERSION_OFFSET 0x04
#define VIRTIO_DEVICE_ID_OFFSET 0x08


void init_block() {
    volatile uint32_t * reg;
    reg = BLOCK_REG(VIRTIO_MAGIC_OFFSET);

    if (*reg != VIRTIO_MAGIC) {
        panicf("Magic value not found\n");
        return;
    }

    printk("virtio: Magic value found\n");

    reg = BLOCK_REG(VIRTIO_VERSION_OFFSET);
    
    // TODO: We should probably implement version 2
    // at some point.
    if (*reg != VIRTIO_VERSION) {
        panicf("Version mismatch");
        return;
    }

    printk("virtio: Version value\n");

    reg = BLOCK_REG(VIRTIO_DEVICE_ID_OFFSET);

    if (*reg == 0x0) {
        panicf("Device ID is 0x0\n");
        return;
    }

    printk("virtio: Correct device ID\n");

    printk("virtio: Initialization complete\n");
}

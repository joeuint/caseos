// SPDX-License-Identifier: GPL-2.0-or-later
// Author: Joseph Umana
// Date: 2025-01-16
// 
// A simple virtio block device driver.
// See https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf#8f

#include <stdint.h>
#include <stdbool.h>

#include "panic.h"
#include "print.h"

#define BLOCK_ADDRESS 0x10001000

#define BLOCK_REG(x) ((volatile uint32_t *)(BLOCK_ADDRESS + (x)))

#define VIRTIO_MAGIC 0x74726976
#define VIRTIO_VERSION 0x02

#define VIRTIO_MAGIC_OFFSET 0x00
#define VIRTIO_VERSION_OFFSET 0x04
#define VIRTIO_DEVICE_ID_OFFSET 0x08

#define VIRTIO_STATUS_OFFSET 0x70
#define VIRTIO_FEATURE_DEVICE_FEATURES_OFFSET 0x10
#define VIRTIO_DRIVER_FEATURES_OFFSET 0x20

#define RESET_VIRTIO() *BLOCK_REG(VIRTIO_STATUS_OFFSET) = 0

// Status bits
#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEATURES_OK 8

struct virtio_blk_device {
    
};

__attribute__((warning("Bad feature negotiation. Pls fix ASAP.")))
void negotiate_features() {
    volatile uint32_t * reg;
    reg = BLOCK_REG(VIRTIO_FEATURE_DEVICE_FEATURES_OFFSET);
    
    // Okay. This is probably not good. This should *Really* be changed
    // for a regular thing
    // But i want this to work for now.
    // TODO: Also handle read only
    *BLOCK_REG(VIRTIO_DRIVER_FEATURES_OFFSET) = *reg;
}

void init_block() {
    volatile uint32_t * reg;
    reg = BLOCK_REG(VIRTIO_MAGIC_OFFSET);

    if (*reg != VIRTIO_MAGIC) {
        panicf("Magic value not found");
        return;
    }

    printk("virtio: Magic value found");

    reg = BLOCK_REG(VIRTIO_VERSION_OFFSET);
    
    if (*reg != VIRTIO_VERSION) {
        panicf("Version mismatch");
        return;
    }

    printk("virtio: Version OK");

    reg = BLOCK_REG(VIRTIO_DEVICE_ID_OFFSET);
    
    // Blank device id should be ignored
    if (*reg == 0x00)
        return;

    printk("virtio: Starting block init");

    // According to docs, we must reset by sending a 0
    // to the status register.
    printk("virtio: Resetting device");
    RESET_VIRTIO();

    // Now we set the ACKNOWLEDGE status bit
    printk("virtio: ACK device");
    *BLOCK_REG(VIRTIO_STATUS_OFFSET) |= VIRTIO_STATUS_ACKNOWLEDGE;

    // Check if block device
    if (*reg != 0x02) {
        printk("virtio: Invalid device id");
        return;
    }

    printk("virtio: BLK device DRIVER OK");

    // Set the DRIVER status bit
    *BLOCK_REG(VIRTIO_STATUS_OFFSET) |= VIRTIO_STATUS_DRIVER;

    printk("virtio: Negotiating features");

    // Feature negotiation
    negotiate_features();

    *BLOCK_REG(VIRTIO_DRIVER_FEATURES_OFFSET) |= VIRTIO_STATUS_FEATURES_OK;

    // Check if still OK
    bool OK = *BLOCK_REG(VIRTIO_STATUS_OFFSET) & VIRTIO_STATUS_FEATURES_OK;

    if (OK)
        panicf("virtio: Feature subset not supported");

    printk("virtio: Features OK");

    // Driver OK
    // Yay!
    *BLOCK_REG(VIRTIO_STATUS_OFFSET) |= VIRTIO_STATUS_DRIVER_OK;

    printk("virtio: Driver OK");
}

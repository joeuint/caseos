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
#include "alloc.h"
#include "string.h"

#include "virtio.h"

void virtio_blk_write(struct virtq* queue) {
    struct virtio_blk_req* req = (struct virtio_blk_req*)kalloc();

    uint8_t* data = kalloc();
    memset_s(data, 69, 512);

    // Memset_s basically casts back to a volatile type so we can ignore
    // the compiler warning by explicitly casting to (void*)
    req->sector = 0;
    req->reserved = 0;
    req->type = VIRTIO_BLK_T_OUT;

    queue->desc[0].addr = (uintptr_t)req;
    queue->desc[0].len = sizeof(struct virtio_blk_req);
    queue->desc[0].flags = VIRTQ_DESC_F_NEXT;
    queue->desc[0].next = 1;

    queue->desc[1].addr = (uintptr_t)data;
    queue->desc[1].len = 512;
    queue->desc[1].flags = VIRTQ_DESC_F_NEXT;
    queue->desc[1].next = 2;

    unsigned char status = 0xff;
    queue->desc[2].addr = (uintptr_t)&status;
    queue->desc[2].len = sizeof(status);
    queue->desc[2].flags = VIRTQ_DESC_F_WRITE;
    queue->desc[2].next = 0;

    queue->avail->ring[queue->avail->idx % VIRTIO_BLK_QUEUE_SIZE] = 0;
    virtio_wmb();
    queue->avail->idx++;

    virtio_wmb();

    *BLOCK_REG(VIRTIO_QUEUE_NOTIFY_OFFSET) = 0;

    while (status == 0xff) {
        __asm__ volatile ("nop");
    }

    if (kfree(data))
        panicf("failed to free");

    printk("%u", status);
}

struct virtq* queue_init() {
    volatile uint32_t num_max;

    *BLOCK_REG(VIRTIO_QUEUE_SEL_OFFSET) = 0;

    if (*BLOCK_REG(VIRTIO_QUEUE_READY_OFFSET) != 0) {
        panicf("virtio: Queue not ready");
        return NULL;
    }

    num_max = *BLOCK_REG(VIRTIO_QUEUE_NUM_MAX_OFFSET);

    if (num_max < 1) {
        panicf("virtio: Queue num max < 1");
        return NULL;
    }

    printk("virtio: queue num max %d", num_max);


    // Allocate the queue
    struct virtq * queue = (struct virtq *)kalloc();

    // Zero it out
    // You know what, we should probably just use memset but it does not exist
    // yet. So I guess we will just use memset_s because im too lazy to copy it
    // and make a normal memset. It should do the same thing though.
    // 
    // Bye,
    // Joseph 2025-01-21
    // TOOD: Replace with memset() when it exists
    memset_s(queue, 0, sizeof(struct virtq));

    // Allocate each ring
    // TODO: Look at storing these directly in the struct?
    // The issue is that virtq would be bigger and may require an extra
    // page or two.
    queue->desc = kalloc();
    queue->avail = kalloc();
    queue->used = kalloc();
    if (!queue->desc || !queue->avail || !queue->used)
        panicf("Failed to alloc descs");
    
    // should be zeroed or QEMU will have an aneurysm lol
    // especially queue->avail.
    memset_s(queue->desc, 0, PAGE_SIZE);
    memset_s(queue->avail, 0, PAGE_SIZE);
    memset_s(queue->used, 0, PAGE_SIZE);

    *BLOCK_REG(VIRTIO_QUEUE_NUM_OFFSET) = VIRTIO_BLK_QUEUE_SIZE;

    *BLOCK_REG(VIRTIO_QUEUE_DESC_LOW) = (uint64_t)queue->desc;
    *BLOCK_REG(VIRTIO_QUEUE_DESC_HIGH) = (uint64_t)queue->desc >> 32;

    *BLOCK_REG(VIRTIO_QUEUE_DRIVER_LOW) = (uint64_t)queue->avail;
    *BLOCK_REG(VIRTIO_QUEUE_DRIVER_HIGH) = (uint64_t)queue->avail >> 32;

    *BLOCK_REG(VIRTIO_QUEUE_DEVICE_LOW) = (uint64_t)queue->used;
    *BLOCK_REG(VIRTIO_QUEUE_DEVICE_HIGH) = (uint64_t)queue->used >> 32;

    // Finally queue ready
    virtio_wmb();

    *BLOCK_REG(VIRTIO_QUEUE_READY_OFFSET) = 0x01;

    return queue;
}

__attribute__((warning("Bad feature negotiation. Pls fix ASAP.")))
void negotiate_features() {
    volatile uint32_t features;
    features = *BLOCK_REG(VIRTIO_DEVICE_FEATURES_OFFSET);

    features &= ~(1 << VIRTIO_BLK_F_RO);
    features &= ~(1 << VIRTIO_BLK_F_SCSI);
    features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
    features &= ~(1 << VIRTIO_BLK_F_MQ);
    features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
    features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
    features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);

    printk("virtio: Device Features: %d", features);

    printk("virtio: Proposed Features: %d", features);
    
    // TODO: This is a pretty bad way to negotiate features
    // Fix it. Exclude them like how xv6 does it.
    // kernel/virtio_disk.c line 87-95
    *BLOCK_REG(VIRTIO_DRIVER_FEATURES_OFFSET) = features;
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

    printk("virtio: Version %d OK", *reg);

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
    virtio_wmb();
    *BLOCK_REG(VIRTIO_STATUS_OFFSET) |= VIRTIO_STATUS_ACKNOWLEDGE;

    // Check if block device
    if (*reg != 0x02) {
        printk("virtio: Invalid device id");
        return;
    }

    printk("virtio: BLK device DRIVER OK");

    // Set the DRIVER status bit
    virtio_wmb();
    *BLOCK_REG(VIRTIO_STATUS_OFFSET) |= VIRTIO_STATUS_DRIVER;

    printk("virtio: Negotiating features");

    // Feature negotiation
    negotiate_features();
    virtio_wmb();

    *BLOCK_REG(VIRTIO_STATUS_OFFSET) |= VIRTIO_STATUS_FEATURES_OK;

    // Check if still OK
    bool OK = *BLOCK_REG(VIRTIO_STATUS_OFFSET) & VIRTIO_STATUS_FEATURES_OK;

    if (!OK)
        panicf("virtio: Feature subset not supported");

    printk("virtio: Features OK");

    // Init the queue
    struct virtq* queue = queue_init();

    // Driver OK
    // Yay! :D
    virtio_wmb();
    *BLOCK_REG(VIRTIO_STATUS_OFFSET) |= VIRTIO_STATUS_DRIVER_OK;

    printk("virtio: Driver OK");

    // Now we can read the config
    volatile struct virtio_blk_config * config = (struct virtio_blk_config *)
        BLOCK_REG(VIRTIO_CONFIG_OFFSET);

    le32 capacity = config->capacity;

    printk("virtio: Capacity: %d sectors", capacity);

    virtio_blk_write(queue);
}

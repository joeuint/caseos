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

#define BLOCK_ADDRESS 0x10001000

// Expected Values
#define VIRTIO_MAGIC 0x74726976
#define VIRTIO_VERSION 0x02

// Property offsets
#define VIRTIO_MAGIC_OFFSET 0x00
#define VIRTIO_VERSION_OFFSET 0x04
#define VIRTIO_DEVICE_ID_OFFSET 0x08
#define VIRTIO_DEVICE_FEATURES_OFFSET 0x10
#define VIRTIO_DRIVER_FEATURES_OFFSET 0x20
#define VIRTIO_QUEUE_SEL_OFFSET 0x30
#define VIRTIO_QUEUE_NUM_MAX_OFFSET 0x34
#define VIRTIO_QUEUE_NUM_OFFSET 0x38
#define VIRTIO_QUEUE_READY_OFFSET 0x44
#define VIRTIO_QUEUE_NOTIFY_OFFSET 0x50
#define VIRTIO_STATUS_OFFSET 0x70

#define VIRTIO_QUEUE_DESC_LOW 0x80
#define VIRTIO_QUEUE_DESC_HIGH 0x84

#define VIRTIO_QUEUE_DRIVER_LOW 0x90
#define VIRTIO_QUEUE_DRIVER_HIGH 0x94

#define VIRTIO_QUEUE_DEVICE_LOW 0xA0
#define VIRTIO_QUEUE_DEVICE_HIGH 0xA4

#define VIRTIO_CONFIG_OFFSET 0x100

// Macros
#define RESET_VIRTIO() *BLOCK_REG(VIRTIO_STATUS_OFFSET) = 0
#define BLOCK_REG(x) ((volatile uint32_t *)(BLOCK_ADDRESS + (x)))

// Status bits
#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEATURES_OK 8

// Block Commands
#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1
#define VIRTIO_BLK_T_FLUSH 4
#define VIRTIO_BLK_T_GET_ID 8
#define VIRTIO_BLK_T_GET_LIFETIME 10
#define VIRTIO_BLK_T_DISCARD 11
#define VIRTIO_BLK_T_WRITE_ZEROES 13
#define VIRTIO_BLK_T_SECURE_ERASE 14

// Descriptor Flags
#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_DESC_F_INDIRECT 4

// Typedefs 
typedef volatile uint64_t le64;
typedef volatile uint32_t le32;
typedef volatile uint16_t le16;
typedef volatile uint8_t le8;

typedef volatile uint8_t u8;
typedef volatile uint16_t u16;
typedef volatile uint32_t u32;
typedef volatile uint64_t u64;

// BLK Feature bits
#define VIRTIO_BLK_F_RO              5	/* Disk is read-only */
#define VIRTIO_BLK_F_SCSI            7	/* Supports scsi command passthru */
#define VIRTIO_BLK_F_CONFIG_WCE     11	/* Writeback mode available in config */
#define VIRTIO_BLK_F_MQ             12	/* support more than one vq */
#define VIRTIO_F_ANY_LAYOUT         27
#define VIRTIO_RING_F_INDIRECT_DESC 28
#define VIRTIO_RING_F_EVENT_IDX     29

// BLK queue size
//
// Changed down to four. A page is about 4KiB. With 8, it would go over a page
#define VIRTIO_BLK_QUEUE_SIZE 8

// BLK config struct
// see https://docs.oasis-open.org/virtio/virtio/v1.2/virtio-v1.2.pdf#5a
struct virtio_blk_config {
    le64 capacity;
    le32 size_max;
    le32 seg_max;
    struct virtio_blk_geometry {
        le16 cylinders;
        u8 heads;
        u8 sectors;
    } geometry;
    le32 blk_size;
    struct virtio_blk_topology {
        // # of logical blocks per physical block (log2)
        u8 physical_block_exp;
        // offset of first aligned logical block
        u8 alignment_offset;
        // suggested minimum I/O size in blocks
        le16 min_io_size;
        // optimal (suggested maximum) I/O size in blocks
        le32 opt_io_size;
    } topology;
    u8 writeback;
    u8 unused0;
    u16 num_queues;
    le32 max_discard_sectors;
    le32 max_discard_seg;
    le32 discard_sector_alignment;
    le32 max_write_zeroes_sectors;
    le32 max_write_zeroes_seg;
    u8 write_zeroes_may_unmap;
    u8 unused1[3];
    le32 max_secure_erase_sectors;
    le32 max_secure_erase_seg;
    le32 secure_erase_sector_alignment;
};

// Virtio BLK request struct
// See (find it yourself)
struct virtio_blk_req {
    le32 type;
    le32 reserved;
    le64 sector;
} __attribute__((packed));

struct virtq_desc {
    le64 addr;
    le32 len;
    le16 flags;
    le16 next;
};

struct virtq_avail {
    le16 flags;
    le16 idx;
    le16 ring[];
};

struct virtq_used_elem {
    le32 id;
    le32 len;
};

struct virtq_used {
    le16 flags;
    le16 idx;
    struct virtq_used_elem ring[];
};

struct virtq {
    unsigned int num;
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;
};

static inline void virtio_wmb(void) {
    __asm__ __volatile__("fence w, w" ::: "memory");
}

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

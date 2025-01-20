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
#define VIRTIO_STATUS_OFFSET 0x70
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
    u8 data[512]; // We r/w 512 bytes at a time
    u8 status;
};

struct virtq {
    unsigned int num;
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;
};

void virtio_blk_write() {
    // just read the first sector
    
}

void queue_init() {
    // just in case
    *BLOCK_REG(VIRTIO_QUEUE_SEL_OFFSET) = 0;

    // we are using 1 queue only

}

__attribute__((warning("Bad feature negotiation. Pls fix ASAP.")))
void negotiate_features() {
    volatile uint32_t features;
    features = *BLOCK_REG(VIRTIO_DEVICE_FEATURES_OFFSET);

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

    *BLOCK_REG(VIRTIO_STATUS_OFFSET) |= VIRTIO_STATUS_FEATURES_OK;

    // Check if still OK
    bool OK = *BLOCK_REG(VIRTIO_STATUS_OFFSET) & VIRTIO_STATUS_FEATURES_OK;

    if (!OK)
        panicf("virtio: Feature subset not supported");

    printk("virtio: Features OK");

    // Driver OK
    // Yay!
    *BLOCK_REG(VIRTIO_STATUS_OFFSET) |= VIRTIO_STATUS_DRIVER_OK;

    printk("virtio: Driver OK");

    // Now we can read the config
    volatile struct virtio_blk_config * config = (struct virtio_blk_config *)
        BLOCK_REG(VIRTIO_CONFIG_OFFSET);

    le32 capacity = config->capacity;

    printk("virtio: Capacity: %d sectors", capacity);

    // Init the queue
    queue_init();
}

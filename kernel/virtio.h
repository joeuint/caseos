#pragma once
#include <stdint.h>

// Addresses
// TODO: Automate with Device Tree
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
    volatile uint64_t capacity;
    volatile uint32_t size_max;
    volatile uint32_t seg_max;
    struct virtio_blk_geometry {
        volatile uint16_t cylinders;
        uint8_t heads;
        uint8_t sectors;
    } geometry;
    volatile uint32_t blk_size;
    struct virtio_blk_topology {
        // # of logical blocks per physical block (log2)
        uint8_t physical_block_exp;
        // offset of first aligned logical block
        uint8_t alignment_offset;
        // suggested minimum I/O size in blocks
        volatile uint16_t min_io_size;
        // optimal (suggested maximum) I/O size in blocks
        volatile uint32_t opt_io_size;
    } topology;
    uint8_t writeback;
    uint8_t unused0;
    u16 num_queues;
    volatile uint32_t max_discard_sectors;
    volatile uint32_t max_discard_seg;
    volatile uint32_t discard_sector_alignment;
    volatile uint32_t max_write_zeroes_sectors;
    volatile uint32_t max_write_zeroes_seg;
    uint8_t write_zeroes_may_unmap;
    uint8_t unused1[3];
    volatile uint32_t max_secure_erase_sectors;
    volatile uint32_t max_secure_erase_seg;
    volatile uint32_t secure_erase_sector_alignment;
};

// Virtio BLK request struct
// See (find it yourself)
struct virtio_blk_req {
    volatile uint32_t type;
    volatile uint32_t reserved;
    volatile uint64_t sector;
} __attribute__((packed));

struct virtq_desc {
    volatile uint64_t addr;
    volatile uint32_t len;
    volatile uint16_t flags;
    volatile uint16_t next;
};

struct virtq_avail {
    volatile uint16_t flags;
    volatile uint16_t idx;
    volatile uint16_t ring[];
};

struct virtq_used_elem {
    volatile uint32_t id;
    volatile uint32_t len;
};

struct virtq_used {
    volatile uint16_t flags;
    volatile uint16_t idx;
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


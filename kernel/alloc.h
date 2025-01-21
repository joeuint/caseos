#pragma once

extern char* _end;

#define KERNEL_BASE (void*)0x80000000

// 4 KiB is standard for page sizes
#define PAGE_SIZE 4096

// Right now, our board has 128MB of ram
// 128 * 1024 = 131072
// 131072 / 4 = 32768
// We can have up to 32768 pages
// Let's just use like 32,000 pages to be safe.
#define NUM_PAGES 32000

#define PAGE_ROUNDUP(x) ((x + PAGE_SIZE - 1) & -PAGE_SIZE)

#define PAGE_START PAGE_ROUNDUP((uint64_t)&_end)

#define PAGE_END (PAGE_START + (NUM_PAGES * PAGE_SIZE))

void init_memory();

void* kalloc();
int kfree(void* ptr) __attribute__((warn_unused_result));
int kfree_s(void* ptr) __attribute__((warn_unused_result));

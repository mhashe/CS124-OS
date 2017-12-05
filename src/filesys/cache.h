/*! \file cache.h
 *
 */

#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include <debug.h>
#include <stdint.h>
#include <kernel/hash.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"


/* Number of sectors in buffer cache. */
#define CACHE_SIZE 64
#define CACHE_SECTOR_EMPTY -1

/* Sleep time for read ahead and write behind.*/
#define CACHE_KERNEL_SLEEP 250

enum lock_mode {
    UNLOCK,
    READ_LOCK,
    WRITE_LOCK
};

struct cache_entry {
    int sector;
    bool access;
    bool dirty;
    char data[BLOCK_SECTOR_SIZE];

    /* Implement read/write lock. */
    struct lock cache_entry_lock;

    struct condition readers;
    struct condition writers;

    /* Count of waiting processes. */
    uint8_t reader_active; /* Threads currently reading. */
    uint8_t reader_waiting; /* Threads waiting to read. */
    /* Writer_active should only ever be one or zero. */
    uint8_t writer_waiting; /* Threads waiting to write. */

    enum lock_mode mode;
};

void cache_init(void);
void cache_kernel_thread_init(void);
void cache_read(block_sector_t sector, void * buffer, off_t size, off_t offset);
void cache_write(block_sector_t sector, const void * buffer, off_t size, off_t offset);
void flush_cache(void);

#endif /* vm/cache.h */


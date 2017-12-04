/*! \file cache.h
 *
 */

#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include <debug.h>
#include <stdint.h>
#include <kernel/hash.h>
#include "threads/synch.h"


/* Number of sectors in buffer cache. */
#define CACHE_SIZE 64
#define CACHE_SECTOR_EMPTY -1

void cache_init(void);

enum lock_mode {
    UNLOCKED,
    READ_LOCKED,
    WRITE_LOCKED
};

struct cache_entry {
    int sector;
    bool dirty;
    char data[BLOCK_SECTOR_SIZE];

    /* Implement read/write lock. */
    struct lock cache_lock;

    struct condition readers;
    struct condition writers;

    uint8_t reader_count;
    uint8_t writer_count;

    enum lock_mode mode;
};

void cache_read(struct block * fs_device, block_sector_t sector, void * buffer);
void cache_write(block_sector_t sector, const void * buffer);

#endif /* vm/cache.h */


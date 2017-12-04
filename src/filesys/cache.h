/*! \file cache.h
 *
 */

#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include <debug.h>
#include <stdint.h>
#include <kernel/hash.h>


/* Number of sectors in buffer cache. */
#define CACHE_SIZE 64
#define CACHE_SECTOR_EMPTY -1

void cache_init(void);

struct cache_entry {
    int sector;
    bool dirty;
    char data[BLOCK_SECTOR_SIZE];
};

void cache_read(struct block * fs_device, block_sector_t sector, void * buffer);
void cache_write(block_sector_t sector, const void * buffer);

#endif /* vm/cache.h */


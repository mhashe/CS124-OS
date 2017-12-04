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

void cache_init(void);

struct cache_entry {
    int sector;
    char data[BLOCK_SECTOR_SIZE];
};

bool cache_read(block_sector_t sector, void * buffer);
bool cache_write(block_sector_t sector, void * buffer);

#endif /* vm/cache.h */


#include <debug.h>
#include <stddef.h>
#include <stdio.h>
#include <kernel/hash.h>
#include <string.h>
// #include <stdint.h>

#include "devices/block.h"
#include "filesys/filesys.h"
#include "cache.h"

/* Cache of data */
static struct cache_entry sector_cache[CACHE_SIZE];

static struct cache_entry * sector_to_cache(block_sector_t sector);
static struct cache_entry * get_free_cache(block_sector_t sector);
static struct cache_entry * cache_evict(void);

/* Initialize. */
void cache_init(void) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        sector_cache[i].sector = CACHE_SECTOR_EMPTY;
    }
}


/* Returns a pointer to the sector's cache entry in the cache. Returns NULL if 
sector is not in the cache. */
static struct cache_entry * sector_to_cache(block_sector_t sector) {
    int target_sector = (int) sector;

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (sector_cache[i].sector == target_sector) {
            return &sector_cache[i];
        }
    }

    return NULL;
}

/* Reads cache data at "cache" into buffer. Returns false if sector is not 
in the cache. */
void cache_read(struct block * fs_device, block_sector_t sector, void * buffer) {
    struct cache_entry * cache = sector_to_cache(sector);

    /* If data is not in the cache, get free cache entry and load into it. */
    if (!cache) {
        cache = get_free_cache(sector);
        block_read(fs_device, sector, cache->data);
    }

    memcpy(buffer, (void *) cache->data, (size_t) BLOCK_SECTOR_SIZE);

}

/* Reads from buffer into cache data at "cache". Returns false if sector is 
not in the cache. */
void cache_write(block_sector_t sector, const void * buffer) {
    struct cache_entry * cache = sector_to_cache(sector);

    /* If data is not in the cache, get free cache entry. */
    if (!cache) {
        cache = get_free_cache(sector);
    }

    memcpy((void *) cache->data, buffer, (size_t) BLOCK_SECTOR_SIZE);
}

static struct cache_entry * get_free_cache(block_sector_t sector) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (sector_cache[i].sector != CACHE_SECTOR_EMPTY) {
            sector_cache[i].sector = sector;
            return &sector_cache[i];
        }
    }

    return cache_evict();
}


/* Comment */
static struct cache_entry * cache_evict(void) {
    ASSERT(0); // not implemented

    return NULL;
}



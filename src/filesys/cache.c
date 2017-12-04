#include <debug.h>
#include <stddef.h>
#include <stdio.h>
#include <kernel/hash.h>
#include <string.h>
// #include <stdint.h>

#include "devices/block.h"
#include "cache.h"

/* Cache of data */
static struct cache_entry sector_cache[CACHE_SIZE];

static char * sector_to_cache_data(block_sector_t sector);

/* Initialize. */
void cache_init(void) {

}


/* Returns NULL if sector is not in the cache. Else, it returns a pointer to 
the sector's data in the cache. */
static char * sector_to_cache_data(block_sector_t sector) {
    int target_sector = (int) sector;

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (sector_cache[i].sector == target_sector) {
            return sector_cache[i].data;
        }
    }

    return NULL;
}

/* Reads cache data at "cache" into buffer. Returns false if sector is not 
in the cache. */
bool cache_read(block_sector_t sector, void * buffer) {
    char * cache_data = sector_to_cache_data(sector);

    if (!cache_data) {
        return false;
    }

    memcpy(buffer, (void *) cache_data, (size_t) BLOCK_SECTOR_SIZE);
    return true;
}

/* Reads from buffer into cache data at "cache". Returns false if sector is 
not in the cache. */
bool cache_write(block_sector_t sector, void * buffer) {
    char * cache_data = sector_to_cache_data(sector);

    if (!cache_data) {
        return false;
    }

    memcpy((void *) cache_data, buffer, (size_t) BLOCK_SECTOR_SIZE);
    return true;
}






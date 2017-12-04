#include <debug.h>
#include <stddef.h>
#include <stdio.h>
#include <kernel/hash.h>
#include <string.h>
// #include <stdint.h>

#include "devices/block.h"
#include "filesys/filesys.h"
#include "cache.h"
#include "threads/synch.h"

struct lock global_fs_lock;

/* Cache of data */
static struct cache_entry sector_cache[CACHE_SIZE];

static struct cache_entry * sector_to_cache(block_sector_t sector);
static struct cache_entry * get_free_cache(block_sector_t sector);
static struct cache_entry * cache_evict(block_sector_t sector);

/* Initialize. */
void cache_init(void) {
    lock_init(&global_fs_lock);

    for (int i = 0; i < CACHE_SIZE; i++) {
        sector_cache[i].sector = CACHE_SECTOR_EMPTY;

        lock_init(&sector_cache[i].cache_lock);
        cond_init(&sector_cache[i].readers);
        cond_init(&sector_cache[i].writers);

        sector_cache[i].mode = UNLOCKED;

        sector_cache[i].reader_count = 0;
        sector_cache[i].writer_count = 0;
    }
}

// /* Acquire lock for an entry as a reader. */
// void read_acquire(struct cache_entry *cache) {
//     cache->readers++;

//     while (1) {
//         /* Some other reader got the lock. */
//         if (cache->mode == READ_LOCKED && cache->writer_count == 0) {
//             break;
//         }

//          We got control of the lock. 
//         if (lock_try_acquire(&cache->cache_lock)) {
//             cache->mode = READ_LOCKED;
//             break;
//         }

//         /* Else, wait for thread to relinquish lock. */
//         cond_wait(&cache->readers, &cache->cache_lock);
//     }
//     cache->readers--;
// }

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

/* Reads cache data at "cache" into buffer. */
void cache_read(struct block * fs_device, block_sector_t sector, void * buffer) {
    /* Bool; we only need to load data if it isn't already loaded. */
    bool loaded = false;
    struct cache_entry *cache = NULL;

    while (1) {
        /* Acquire global lock and cache entry. */
        lock_acquire(&global_fs_lock);
        cache = sector_to_cache(sector);
        lock_release(&global_fs_lock);

        if (!cache) {
            lock_acquire(&global_fs_lock);
            cache = get_free_cache(sector);
            loaded = true;
        } else {
            lock_acquire(&cache->cache_lock);
            loaded = false;
        }

        /* Should've switched locks. */
        ASSERT(!lock_held_by_current_thread(&global_fs_lock));
        ASSERT(lock_held_by_current_thread(&cache->cache_lock));
        
        /* Ensure that this is still the correct sector. */
        if (cache->sector == (int) sector) {
            /* We gud. */
            break;
        } else {
            /* We not gud. */
            lock_release(&cache->cache_lock);
        }

        /* If data is not in the cache, get free cache entry and load into it. */
    }

    /* Really shouldn't be null. */
    ASSERT(cache);

    if (loaded) {
        block_read(fs_device, sector, cache->data); 
    }
    memcpy(buffer, (void *) &cache->data, (size_t) BLOCK_SECTOR_SIZE);

    /* We done, we release. */
    lock_release(&cache->cache_lock);
}

/* Reads from buffer into cache data at "cache". 
   Write sector SECTOR to CACHE from BUFFER, which must contain
   BLOCK_SECTOR_SIZE bytes.*/
void cache_write(block_sector_t sector, const void * buffer) {
    /* Bool; we only need to load data if it isn't already loaded. */
    struct cache_entry *cache = NULL;

    while (1) {
        /* Acquire global lock and cache entry. */
        lock_acquire(&global_fs_lock);
        cache = sector_to_cache(sector);
        lock_release(&global_fs_lock);

        if (!cache) {
            lock_acquire(&global_fs_lock);
            cache = get_free_cache(sector);
        } else {
            lock_acquire(&cache->cache_lock);
        }

        /* Should've switched locks. */
        ASSERT(!lock_held_by_current_thread(&global_fs_lock));
        ASSERT(lock_held_by_current_thread(&cache->cache_lock));
        
        /* Ensure that this is still the correct sector. */
        if (cache->sector == (int) sector) {
            /* We gud. */
            break;
        } else {
            /* We not gud. */
            lock_release(&cache->cache_lock);
        }

        /* If data is not in the cache, get free cache entry and load into it. */
    }

    /* Really shouldn't be null. */
    ASSERT(cache);

    memcpy((void *) &cache->data, buffer, (size_t) BLOCK_SECTOR_SIZE);

    /* We done, we release. */
    lock_release(&cache->cache_lock);
}

static struct cache_entry * get_free_cache(block_sector_t sector) {
    ASSERT(lock_held_by_current_thread(&global_fs_lock));
    // TODO assert that we hold global lock
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (sector_cache[i].sector == CACHE_SECTOR_EMPTY) {
            sector_cache[i].sector = sector;

            /* If this is actually free, no one holds this lock. */
            ASSERT(lock_try_acquire(&sector_cache[i].cache_lock));
            lock_release(&global_fs_lock);
            memset(&sector_cache[i].data, 0, BLOCK_SECTOR_SIZE);
            return &sector_cache[i];
        }
    }
    
    return cache_evict(sector);
}


/* Comment */
static struct cache_entry * cache_evict(block_sector_t sector) {
    // TODO: something better
    // TODO: only if dirty
    ASSERT(lock_held_by_current_thread(&global_fs_lock));
    int victim = 0;

    struct cache_entry *cache = &sector_cache[victim];

    /* Switch locks. */
    lock_release(&global_fs_lock);
    lock_acquire(&cache->cache_lock);

    block_write(fs_device, cache->sector, &cache->data);

    cache->sector = sector;
    cache->dirty = false;
    memset(&cache->data, 0, BLOCK_SECTOR_SIZE);

    return cache;
}



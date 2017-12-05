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
static struct cache_entry * get_free_cache(block_sector_t sector, enum lock_mode mode);
static struct cache_entry * cache_evict(block_sector_t sector, enum lock_mode mode);

/* Initialize. */
void cache_init(void) {
    lock_init(&global_fs_lock);

    for (int i = 0; i < CACHE_SIZE; i++) {
        sector_cache[i].sector = CACHE_SECTOR_EMPTY;

        lock_init(&sector_cache[i].cache_lock);
        cond_init(&sector_cache[i].readers);
        cond_init(&sector_cache[i].writers);

        sector_cache[i].mode = UNLOCK;

        sector_cache[i].reader_active = 0;
        sector_cache[i].reader_waiting = 0;
        sector_cache[i].writer_waiting = 0;
    }
}

// /* Acquire lock for an entry as a reader. */
// void read_acquire(struct cache_entry *cache) {
//     cache->readers++;

//     while (1) {
//         /* Some other reader got the lock. */
//         if (cache->mode == READ_LOCKED && cache->writer_waiting == 0) {
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
void cache_read(struct block * fs_device UNUSED, block_sector_t sector, void * buffer) {
    struct cache_entry *cache = NULL;

    while (1) {
        /* Acquire global lock and cache entry. */
        lock_acquire(&global_fs_lock);
        cache = sector_to_cache(sector);
        lock_release(&global_fs_lock);

        if (!cache) {
            lock_acquire(&global_fs_lock);
            cache = get_free_cache(sector, READ_LOCK);
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
    }

    /* Really shouldn't be null. */
    ASSERT(cache);

    if (cache->mode == UNLOCK) {
        /* Don't need to wait. */
        cache->mode = READ_LOCK;
    } else if (cache->mode == READ_LOCK && cache->writer_waiting == 0) {
        /* Just keep reading. */
    } else {
        /* Need to wait; either
           1) WRITE_LOCK'ed, let the writer finish.
           2) Writer waiting, let him get a turn. */
        cache->reader_waiting++;
        cond_wait(&cache->readers, &cache->cache_lock);
        cache->reader_waiting--;

        /* Passing around the lock between waiters shouldn't change the 
           cache sector. */
        ASSERT(cache->sector == (int) sector);
    }

    /* Begin reading. */
    cache->reader_active++;

    ASSERT(cache->mode == READ_LOCK);
    memcpy(buffer, (void *) &cache->data, (size_t) BLOCK_SECTOR_SIZE);
    cache->reader_active--;

    /* If we're done reading, reset state. */
    if (cache->reader_active == 0 && cache->writer_waiting > 0) {
        /* No more readers, but there is a writer. */
        cache->mode = WRITE_LOCK;
        cond_signal(&cache->writers, &cache->cache_lock);
    } else if (cache->reader_active == 0 && cache->writer_waiting == 0) {
        /* Just set to unlock. */
        cache->mode = UNLOCK;
    } /* else: More readers, let them take care of cleanup. */

    /* We done, we release. */
    lock_release(&cache->cache_lock);
}

/* Reads from buffer into cache data at "cache". 
   Write sector SECTOR to CACHE from BUFFER, which must contain
   BLOCK_SECTOR_SIZE bytes.*/
void cache_write(block_sector_t sector, const void * buffer) {
    struct cache_entry *cache = NULL;
    // TODO don't need to read from disk when we load from disk because we
    // necessarily overwrite the whole sector


    while (1) {
        /* Acquire global lock and cache entry. */
        lock_acquire(&global_fs_lock);
        cache = sector_to_cache(sector);
        lock_release(&global_fs_lock);

        if (!cache) {
            lock_acquire(&global_fs_lock);
            cache = get_free_cache(sector, WRITE_LOCK);
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
    }

    /* Really shouldn't be null. */
    ASSERT(cache);

    if (cache->mode == UNLOCK) {
        cache->mode = WRITE_LOCK;
    } else {
        /* We need to wait - signal this to other threads with writer count. */
        cache->writer_waiting++;
        cond_wait(&cache->writers, &cache->cache_lock);
        cache->writer_waiting--;
    }

    ASSERT(cache->mode == WRITE_LOCK);
    ASSERT(cache->reader_active == 0);

    memcpy((void *) &cache->data, buffer, (size_t) BLOCK_SECTOR_SIZE);

    /* Once we're done, signal the next threads. */
    if (cache->reader_waiting > 0) {
        /* Grant file to readers.*/
        cache->mode = READ_LOCK;
        cond_broadcast(&cache->readers, &cache->cache_lock);
    } else if (cache->reader_waiting == 0 && cache->writer_waiting > 0) {
        /* Signal some writer. */
        cond_signal(&cache->writers, &cache->cache_lock);
    } else {
        /* No one waiting. */
        cache->mode = UNLOCK;
    }

    /* We done, we release. */
    lock_release(&cache->cache_lock);
}

static struct cache_entry *get_free_cache(block_sector_t sector, enum lock_mode mode) {
    ASSERT(lock_held_by_current_thread(&global_fs_lock));

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (sector_cache[i].sector == CACHE_SECTOR_EMPTY) {
            sector_cache[i].sector = sector;

            /* If this is actually free, no one holds this lock. */
            ASSERT(lock_try_acquire(&sector_cache[i].cache_lock));
            ASSERT(sector_cache[i].mode == UNLOCK);
            ASSERT(sector_cache[i].reader_active == 0);
            ASSERT(sector_cache[i].writer_waiting == 0);

            /* Lock is acquired, mark is as being r/w. */
            // sector_cache[i].mode = mode;

            /* Relinquish control of cache table. */
            lock_release(&global_fs_lock);

            /* Read in new memory. */
            memset(&sector_cache[i].data, 0, BLOCK_SECTOR_SIZE);
            block_read(fs_device, sector, &sector_cache[i].data); 

            return &sector_cache[i];
        }
    }

    return cache_evict(sector, mode);
}


/* Comment */
static struct cache_entry *cache_evict(block_sector_t sector, enum lock_mode mode) {
    // TODO: something better
    // TODO: only if dirty
    ASSERT(lock_held_by_current_thread(&global_fs_lock));
    int victim = 0;

    struct cache_entry *cache = &sector_cache[victim];

    /* Switch locks. */
    lock_release(&global_fs_lock);

    /* For now at least, we just leave all currently waiting threads waiting.
       The idea is that when they wake up at some point, they acquire the lock,
       realize that this is no longer the data that they want, and then
       relinquish it. */
    lock_acquire(&cache->cache_lock);

    /* We relock the cache table here to ensure that processes cannot 
       concurrently load the same sector into cache memory twice. */
    lock_acquire(&global_fs_lock);

    struct cache_entry *loaded_cache = sector_to_cache(sector);
    if (loaded_cache) {
        /* If it's already loaded, we only need to lock that cache entry. */
        lock_release(&global_fs_lock);
        lock_release(&cache->cache_lock);
        lock_acquire(&loaded_cache->cache_lock);

        return loaded_cache;
    } else {
        /* Keep track of old sector, for writing to disk. */
        int old_sector = cache->sector;

        /* Still need to load it; mark it here so that everyone
           blocks on it. */
        cache->sector = sector;
        cache->dirty = false;

        /* Now that everyone knows where the sector will be loaded, we can
           release global. Anyone trying to access this (half-loaded) sector
           will block until we release the lock later. */
        lock_release(&global_fs_lock);

        /* Read in new memory, write out old. */
        block_write(fs_device, old_sector, &cache->data);
        memset(&cache->data, 0, BLOCK_SECTOR_SIZE);
        block_read(fs_device, sector, cache->data); 
        return cache;
    }
}


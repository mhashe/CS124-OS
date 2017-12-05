#include <debug.h>
#include <stddef.h>
#include <stdio.h>
#include <kernel/hash.h>
#include <string.h>
// #include <stdint.h>

#include "devices/block.h"
#include "filesys/filesys.h" /* fs_device */
#include "cache.h"
#include "threads/synch.h"

struct lock cache_table_lock;

/* Victim. */
static int victim;

/* Cache of data */
static struct cache_entry sector_cache[CACHE_SIZE];

/* Helper functions. */
static struct cache_entry *sector_to_cache(block_sector_t sector);
static struct cache_entry *get_free_cache(block_sector_t sector, bool writing);
static struct cache_entry *cache_evict(block_sector_t sector, bool writing);

/* Initialize. */
void cache_init(void) {
    victim = 0;

    lock_init(&cache_table_lock);

    for (int i = 0; i < CACHE_SIZE; i++) {
        sector_cache[i].sector = CACHE_SECTOR_EMPTY;

        sector_cache[i].access = false;
        sector_cache[i].dirty = false;
        memset(&sector_cache[i].data, 0, BLOCK_SECTOR_SIZE);

        lock_init(&sector_cache[i].cache_entry_lock);

        cond_init(&sector_cache[i].readers);
        cond_init(&sector_cache[i].writers);

        sector_cache[i].reader_active = 0;
        sector_cache[i].reader_waiting = 0;
        sector_cache[i].writer_waiting = 0;
        
        sector_cache[i].mode = UNLOCK;
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

/* Reads cache data at "cache" into buffer. */
void cache_read(block_sector_t sector, void * buffer) {
    struct cache_entry *cache = NULL;

    /* Looping is done to avoid situations where we obtain a cache,
       switch to a thread that changes our cache, and then switch
       back- here, we ensure that our cache still contains the
       correct sector. */
    while (1) {
        /* Acquire global lock and cache entry. */
        lock_acquire(&cache_table_lock);
        cache = sector_to_cache(sector);
        lock_release(&cache_table_lock);

        if (!cache) {
            /* Sector is not currently in cache- switch it in. */
            lock_acquire(&cache_table_lock);
            cache = get_free_cache(sector, false);
        } else {
            /* Lock cache until we finish reading from it. */
            lock_acquire(&cache->cache_entry_lock);
        }

        /* Should've switched locks. */
        ASSERT(!lock_held_by_current_thread(&cache_table_lock));
        ASSERT(lock_held_by_current_thread(&cache->cache_entry_lock));
        
        /* Ensure that this is still the correct sector. */
        if (cache->sector == (int) sector) {
            /* We gud. */
            break;
        } else {
            /* We not gud. */
            lock_release(&cache->cache_entry_lock);
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
        cond_wait(&cache->readers, &cache->cache_entry_lock);
        cache->reader_waiting--;

        /* Passing around the lock between waiters shouldn't change the 
           cache sector. */
        ASSERT(cache->sector == (int) sector);
    }

    /* Begin reading. */
    cache->reader_active++;
    ASSERT(cache->mode == READ_LOCK);

    /* Carry out actual read. */
    memcpy(buffer, (void *) &cache->data, (size_t) BLOCK_SECTOR_SIZE);
    cache->access = true;
    cache->reader_active--;

    /* If we're done reading, reset state. */
    if (cache->reader_active == 0 && cache->writer_waiting > 0) {
        /* No more readers, but there is a writer. */
        cache->mode = WRITE_LOCK;
        cond_signal(&cache->writers, &cache->cache_entry_lock);
    } else if (cache->reader_active == 0 && cache->writer_waiting == 0) {
        /* Just set to unlock. */
        cache->mode = UNLOCK;
    } /* else: More readers, let them take care of cleanup. */

    /* We done, we release. */
    lock_release(&cache->cache_entry_lock);
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
        lock_acquire(&cache_table_lock);
        cache = sector_to_cache(sector);
        lock_release(&cache_table_lock);

        if (!cache) {
            lock_acquire(&cache_table_lock);
            cache = get_free_cache(sector, true);
        } else {
            lock_acquire(&cache->cache_entry_lock);
        }

        /* Should've switched locks. */
        ASSERT(!lock_held_by_current_thread(&cache_table_lock));
        ASSERT(lock_held_by_current_thread(&cache->cache_entry_lock));
        
        /* Ensure that this is still the correct sector. */
        if (cache->sector == (int) sector) {
            /* We gud. */
            break;
        } else {
            /* We not gud. */
            lock_release(&cache->cache_entry_lock);
        }
    }

    /* Really shouldn't be null. */
    ASSERT(cache);

    if (cache->mode == UNLOCK) {
        cache->mode = WRITE_LOCK;
    } else {
        /* We need to wait - signal this to other threads with writer count. */
        cache->writer_waiting++;
        cond_wait(&cache->writers, &cache->cache_entry_lock);
        cache->writer_waiting--;
    }

    ASSERT(cache->mode == WRITE_LOCK);
    ASSERT(cache->reader_active == 0);

    /* Carry out actual write operation. */
    memcpy((void *) &cache->data, buffer, (size_t) BLOCK_SECTOR_SIZE);
    cache->access = true;
    cache->dirty = true;

    /* Once we're done, signal the next threads. */
    if (cache->reader_waiting > 0) {
        /* Grant file to readers.*/
        cache->mode = READ_LOCK;
        cond_broadcast(&cache->readers, &cache->cache_entry_lock);
    } else if (cache->reader_waiting == 0 && cache->writer_waiting > 0) {
        /* Signal some writer. */
        cond_signal(&cache->writers, &cache->cache_entry_lock);
    } else {
        /* No one waiting. */
        cache->mode = UNLOCK;
    }

    /* We done, we release. */
    lock_release(&cache->cache_entry_lock);
}

static struct cache_entry *get_free_cache(block_sector_t sector, bool writing) {
    ASSERT(lock_held_by_current_thread(&cache_table_lock));

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (sector_cache[i].sector == CACHE_SECTOR_EMPTY) {
            sector_cache[i].sector = sector;

            /* If this is actually free, no one holds this lock. */
            ASSERT(lock_try_acquire(&sector_cache[i].cache_entry_lock));
            ASSERT(sector_cache[i].mode == UNLOCK);
            ASSERT(sector_cache[i].reader_active == 0);
            ASSERT(sector_cache[i].writer_waiting == 0);

            /* Relinquish control of cache table. */
            lock_release(&cache_table_lock);

            /* Read in new memory, unless we're immediately going to 
               overwrite it. */
            // TODO : review memset policy
            // memset(&sector_cache[i].data, 0, BLOCK_SECTOR_SIZE);
            if (!writing) {
                block_read(fs_device, sector, &sector_cache[i].data); 
            }

            return &sector_cache[i];
        }
    }

    return cache_evict(sector, writing);
}


/* Comment */
static struct cache_entry *cache_evict(block_sector_t sector, bool writing) {
    // TODO: something better
    // TODO: only if dirty
    ASSERT(lock_held_by_current_thread(&cache_table_lock));

    /* Choose victim, set for next time. TODO : better policy. */
    int cur_vic = victim;
    victim = (victim + 1) % CACHE_SIZE;

    struct cache_entry *cache = &sector_cache[cur_vic];

    /* Switch locks. */
    lock_release(&cache_table_lock);

    /* For now at least, we just leave all currently waiting threads waiting.
       The idea is that when they wake up at some point, they acquire the lock,
       realize that this is no longer the data that they want, and then
       relinquish it. */
    lock_acquire(&cache->cache_entry_lock);

    /* We relock the cache table here to ensure that processes cannot 
       concurrently load the same sector into cache memory twice. */
    lock_acquire(&cache_table_lock);

    struct cache_entry *loaded_cache = sector_to_cache(sector);
    if (loaded_cache) {
        /* If it's already loaded, we only need to lock that cache entry. */
        lock_release(&cache_table_lock);
        lock_release(&cache->cache_entry_lock);
        lock_acquire(&loaded_cache->cache_entry_lock);

        return loaded_cache;
    } else {
        /* Keep track of old sector, for writing to disk. */
        int old_sector = cache->sector;

        /* Still need to load it; mark it here so that everyone
           blocks on it. */
        cache->sector = sector;

        /* Now that everyone knows where the sector will be loaded, we can
           release global. Anyone trying to access this (half-loaded) sector
           will block until we release the lock later. */
        lock_release(&cache_table_lock);

        /* Read in new memory, write out old (if it's dirty). */
        if (cache->dirty) {
            block_write(fs_device, old_sector, &cache->data); 
        }
        cache->dirty = false;
        // TODO : review memset policy
        // memset(&cache->data, 0, BLOCK_SECTOR_SIZE);
        if (!writing) {
            block_read(fs_device, sector, cache->data); 
        }
        return cache;
    }
}


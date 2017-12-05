#include "cache.h"

#include <debug.h>
#include <kernel/hash.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "devices/block.h"
#include "devices/timer.h"
#include "filesys/filesys.h" /* fs_device */
#include "filesys/off_t.h"
#include "threads/synch.h"
#include "threads/thread.h"

struct lock cache_table_lock;

/* Victim. */
static int victim;

/* Cache of data */
static struct cache_entry sector_cache[CACHE_SIZE];

/* Read-ahead queue, clock-like index. */
static int read_ahead_buffer[CACHE_SIZE];
volatile int read_ahead_head;
volatile int read_ahead_tail;

/* Helper functions. */
static struct cache_entry *sector_to_cache(block_sector_t sector);
static struct cache_entry *get_free_cache(block_sector_t sector, bool writing);
static struct cache_entry *cache_evict(block_sector_t sector, bool writing);

static void read_ahead(void *arg_ UNUSED);
static void write_behind(void *arg_ UNUSED);



/* Initialize. */
void cache_init(void) {
    victim = 0;
    read_ahead_head = 0;
    read_ahead_tail = 0;

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

        /* Also, the read_ahead_buffer. */
        read_ahead_buffer[i] = CACHE_SECTOR_EMPTY;
    }
}

void cache_kernel_thread_init(void) {
    thread_create("cache-read-ahead",   PRI_DEFAULT, read_ahead, NULL);
    thread_create("cache-write-behind", PRI_DEFAULT, write_behind, NULL);
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
void cache_read(block_sector_t sector, void * buffer, off_t size, off_t offset) {
    ASSERT(size + offset <= BLOCK_SECTOR_SIZE);
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
    memcpy(buffer, (void *) (&cache->data) + offset, (size_t) size);
    cache->access = true;
    cache->reader_active--;

    /* Line up read ahead. */
    read_ahead_buffer[read_ahead_tail] = (sector + 1) % block_size(fs_device);
    read_ahead_tail = (read_ahead_tail + 1) % CACHE_SIZE;

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
void cache_write(block_sector_t sector, const void * buffer, off_t size, off_t offset) {
    ASSERT(size + offset <= BLOCK_SECTOR_SIZE);
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
    memcpy((void *) &cache->data + offset, buffer, (size_t) size);
    cache->access = true;
    cache->dirty = true;

    /* Line up read ahead. */
    read_ahead_buffer[read_ahead_tail] = (sector + 1) % block_size(fs_device);
    read_ahead_tail = (read_ahead_tail + 1) % CACHE_SIZE;

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

/* Called when filesystem is closed. We want to write all dirty block to disk.*/
void flush_cache(void) {
    struct cache_entry *cache = NULL;

    for (int i = 0; i < CACHE_SIZE; i++) {
        cache = &sector_cache[i];

        if (cache->sector == CACHE_SECTOR_EMPTY) {
            continue;
        }

        if (cache->dirty) {
            ASSERT(lock_try_acquire(&cache->cache_entry_lock))
            block_write(fs_device, cache->sector, &cache->data);
            cache->dirty = false;
            lock_release(&cache->cache_entry_lock);
        }

    }
}

/* Reads data into cache, but not into buffer. */
static void read_ahead(void *arg_ UNUSED) {
    struct cache_entry *cache = NULL;
    
    while (1) {
        while (read_ahead_head == read_ahead_tail) {
            /* Nothing to do, sleep. */
            timer_msleep(CACHE_KERNEL_SLEEP);
        }

        /* Something to read ahead. */
        int rah = read_ahead_head;


        /* Acquire global lock and cache entry. */
        lock_acquire(&cache_table_lock);
        cache = sector_to_cache(read_ahead_buffer[rah]);
        lock_release(&cache_table_lock);

        /* Sector is not currently in cache- switch it in. */
        if (!cache) {
            lock_acquire(&cache_table_lock);
            cache = get_free_cache(rah, false);

            /* Really shouldn't be null. */
            ASSERT(cache);

            /* Should've switched locks. */
            ASSERT(!lock_held_by_current_thread(&cache_table_lock));
            ASSERT(lock_held_by_current_thread(&cache->cache_entry_lock));
            
            /* We done, we release. */
            lock_release(&cache->cache_entry_lock);
        }

        /* Move to next entry in read ahead buffer. */
        read_ahead_head = (read_ahead_head + 1) % CACHE_SIZE;
        cache = NULL;
        timer_msleep(CACHE_KERNEL_SLEEP);
    }
}

static void write_behind(void *arg_ UNUSED) {
    struct cache_entry *cache = NULL;

    int i = 0;
    while (1) {
        cache = &sector_cache[i];

        /* Cache is filled from the front; if we ever hit
           empty entries, everything past that is also empty. */
        if (cache->sector == CACHE_SECTOR_EMPTY) {
            i = 0;
        }

        if (cache->dirty) {
            if (lock_try_acquire(&cache->cache_entry_lock)) {
                block_write(fs_device, cache->sector, &cache->data);
                cache->dirty = false;
                lock_release(&cache->cache_entry_lock);
            }
        }

        /* Move to next entry in cache. */
        i = (i + 1) % CACHE_SIZE;
        cache = NULL;
        timer_msleep(CACHE_KERNEL_SLEEP);
    }
}


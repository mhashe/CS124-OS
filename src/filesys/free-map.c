#include "filesys/free-map.h"
#include <bitmap.h>
#include <debug.h>
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"

static struct file *free_map_file;   /*!< Free map file. */
static struct bitmap *free_map;      /*!< Free map, one bit per sector. */

/*! Initializes the free map. */
void free_map_init(void) {
    free_map = bitmap_create(block_size(fs_device));
    if (free_map == NULL)
        PANIC("bitmap creation failed--file system device is too large");
    bitmap_mark(free_map, FREE_MAP_SECTOR);
    bitmap_mark(free_map, ROOT_DIR_SECTOR);
}

/*! Allocates CNT consecutive sectors from the free map and stores the first
    into *SECTORP.
    Returns true if successful, false if not enough consecutive sectors were
    available or if the free_map file could not be written. */
bool free_map_allocate(size_t cnt, block_sector_t *sectorp) {
    block_sector_t sector = bitmap_scan_and_flip(free_map, 0, cnt, false);
    if (sector != BITMAP_ERROR && free_map_file != NULL &&
        !bitmap_write(free_map, free_map_file)) {
        bitmap_set_multiple(free_map, sector, cnt, false); 
        sector = BITMAP_ERROR;
    }
    if (sector != BITMAP_ERROR)
        *sectorp = sector;
    return sector != BITMAP_ERROR;
}

/*! Allocates CNT sectors from the free map and stores the first
    into *SECTORP.
    Returns true if successful, false if not enough sectors were
    available or if the free_map file could not be written. */

// TODO: create a free_map_allocate() that doesn't allocate continguous values in the bitmap
// instead, it finds total number free. if total free are enough, then set each 
// individually to not free. or just use bitmap_test() all the way.

// static void set_inode_sector(struct inode_disk *data, off_t pos, 
//                                 block_sector_t sector) {
//     if (pos < NUM_DIRECT) {
//         data->direct[pos] = sector;
//         return;
//     }
//     if (pos < (NUM_ENTRIES_IN_INDIRECT * NUM_INDIRECT) + NUM_DIRECT) {
//         pos -= NUM_DIRECT;
//         data->indirect[pos % NUM_ENTRIES_IN_INDIRECT]->[NUM_DIRECT] = sector;
//         return;
//     }
//     data->double_indr

// }


bool free_map_append_to_inode(size_t cnt, struct inode_disk *data) {
    // TODO: Fix cnt!

    /* If that are not available free sectors for allocation, return false. */
    if (bitmap_count(free_map, 0, cnt, false) < cnt) {
        return false;
    }

    // TODO: obviously this entire process of allocating is something that 
    // needs to be locked down

    /* Else, we allocate the sectors and fill them into the inode_disk data. */
    size_t num_sectors = bitmap_size(free_map);

    int cur_dir, cur_ind, cur_double_ind;
    block_sector_t *cur_dir_buf, cur_ind_buf;

    // TODO: Clean this up??? This is very ugly.
    int max_double_ind = (NUM_ENTRIES_IN_INDIRECT * NUM_INDIRECT) + NUM_DIRECT;
    if (data->length < NUM_DIRECT) {
        cur_double_ind = -1;
        cur_ind = -1;
        cur_dir = data->length;
        cur_ind_buf = NULL;
        cur_double_ind_buf = NULL;
    } else if (pos < max_double_ind) {
        cur_double_ind = -1;
        cur_ind = (data->length - NUM_DIRECT) / NUM_ENTRIES_IN_INDIRECT;
        cur_dir = (data->length - NUM_DIRECT) % NUM_ENTRIES_IN_INDIRECT;
        cur_dir_buf = malloc(BLOCK_SECTOR_SIZE);
        if (!cur_dir_buf) {
            return false;
        }
        cache_read(data->indirect[cur_ind], cur_dir_buf);
        cur_ind_buf = NULL;
    } else {
        cur_double_ind = 0;
        cur_ind = (data->length - max_double_ind) % (NUM_ENTRIES_IN_INDIRECT * NUM_INDIRECT);
        cur_dir = (data->length - max_double_ind) % NUM_ENTRIES_IN_INDIRECT;
        cur_ind_buf = malloc(BLOCK_SECTOR_SIZE);
        if (!cur_ind_buf) {
            return false;
        }
        cache_read(data->double_indirect, cur_ind_buf);
        cur_dir_buf = malloc(BLOCK_SECTOR_SIZE);
        if (!cur_dir_buf) {
            free(cur_ind_buf);
            return false;
        }
        cache_read(cur_ind_buf[cur_ind], cur_dir_buf);
    }

    int current_indirect = -1;
    int current_direct;
    size_t num_left = cnt;      /* Number of sectors left to be allocated. */

    // TODO: bitmap_test is slow b/c it does a lot of checks. this may be able 
    // to be done computationally faster by writing our own bitmap function.
    for (size_t i = 0; i < num_sectors; i++) {
        /* We found a sector that is free, so give it to the inode. */
        if (!bitmap_test(free_map, i)) {
            bitmap_mark(free_map, i);
            num_left--;

            if (cur_ind != -1) {
                data->direct[cur_dir] = i;
            } else if (cur_double_ind != -1) {
                cur_dir_buf[cur_dir] = i;
            }

            cur_dir++;

            // Make sure cur_dir_buf is pointing to something that makes sense
            if (cur_ind == -1) {
                if (cur_dir > NUM_DIRECT) {
                    cur_ind = 0;
                    cur_dir = 0;
                    cur_dir_buf = malloc(BLOCK_SECTOR_SIZE);
                    if (!cur_dir_buf) {
                        return false;
                    }
                    cache_read(data->indirect[cur_ind], cur_dir_buf);
                }
            } else {
                if (cur_dir == NUM_ENTRIES_IN_INDIRECT) {
                    if (cur_ind == NUM_INDIRECT && cur_double_ind == 0) {
                        cache_write(data->indirect[cur_ind - 1], cur_dir_buf);
                        cur_dir = 0;
                        cur_ind = 0;
                        cur_double_ind = 1;
                        cache_read(data->double_indirect, cur_ind_buf);
                        cur_dir_buf = malloc(BLOCK_SECTOR_SIZE);
                        if (!cur_dir_buf) {
                            free(cur_ind_buf);
                            return false;
                        }
                        cache_read(cur_ind_buf[cur_ind], cur_dir_buf);
                    } else {
                        
                        cur_dir = 0;
                        cur_ind++;
                        cache_read(data->indirect[cur_ind], cur_dir_buf);
                    }
                }
            }
        }
    }

    /* Make sure that we were able to allocate all blocks that we needed to. */
    ASSERT(num_alloced == cnt);

    data->length += cnt;

    /* Finish persisting to disk our changes and free temporary buffers. */
    if (cur_double_ind) {
        cache_write(data->double_indirect, cur_ind_buf);
        cache_write(cur_ind_buf[cur_ind], cur_dir_buf);
        free(cur_ind_buf);
        free(cur_dir_buf);
    }

    if (cur_dir_buf) {
        cache_write(data->indirect[cur_ind], cur_dir_buf);
        free(cur_dir_buf);
    }

    return true;
}


/*! Makes CNT sectors starting at SECTOR available for use. */
void free_map_release(block_sector_t sector, size_t cnt) {
    ASSERT(bitmap_all(free_map, sector, cnt));
    bitmap_set_multiple(free_map, sector, cnt, false);
    bitmap_write(free_map, free_map_file);
}

/*! Opens the free map file and reads it from disk. */
void free_map_open(void) {
    free_map_file = file_open(inode_open(FREE_MAP_SECTOR));
    if (free_map_file == NULL)
        PANIC("can't open free map");
    if (!bitmap_read(free_map, free_map_file))
        PANIC("can't read free map");
}

/*! Writes the free map to disk and closes the free map file. */
void free_map_close(void) {
    file_close(free_map_file);
}

/*! Creates a new free map file on disk and writes the free map to it. */
void free_map_create(void) {
    /* Create inode. */
    if (!inode_create(FREE_MAP_SECTOR, bitmap_file_size(free_map)))
        PANIC("free map creation failed");

    /* Write bitmap to file. */
    free_map_file = file_open(inode_open(FREE_MAP_SECTOR));
    if (free_map_file == NULL)
        PANIC("can't open free map");
    if (!bitmap_write(free_map, free_map_file))
        PANIC("can't write free map");
}

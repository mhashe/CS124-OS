#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include <bitmap.h>
#include "filesys/filesys.h"
#include "filesys/cache.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"
#include "threads/synch.h"

#include <stdio.h>

/*! Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

/*! On-disk inode.
    Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk {
    volatile off_t length;           /*!< File size in bytes. */
    bool is_directory;               /*!< If inode represents a directory. */

    block_sector_t double_indirect;  /*!< Multilevel indirection. */

    unsigned magic;                  /*!< Magic number. */
    uint32_t unused[124];            /*!< Not used. */
};

/*! Returns the number of sectors to allocate for an inode SIZE
    bytes long. */
static inline size_t bytes_to_sectors(off_t size) {
    return DIV_ROUND_UP(size, BLOCK_SECTOR_SIZE);
}


/*! In-memory inode. */
struct inode {
    struct list_elem elem;              /*!< Element in inode list. */
    block_sector_t sector;              /*!< Sector number of disk location. */
    int open_cnt;                       /*!< Number of openers. */
    bool removed;                       /*!< True if deleted, false otherwise.*/
    int deny_write_cnt;                 /*!< 0: writes ok, >0: deny writes. */
    struct lock extension_lock;         /*!< Lock to extend file. */
    struct inode_disk data;             /*!< Inode content. */

    int file_count;                     /*!< Count of files / subsdirectories. */
};


static void indices_from_offset(off_t pos, size_t *dir_idx, size_t *ind_idx);
static bool inode_extend_file(struct inode_disk *data, size_t cnt);
static void print_inode_allocation(struct inode_disk *data);

/*! List of open inodes, so that opening a single inode twice
    returns the same `struct inode'. */
static struct list open_inodes;



void inode_check(struct inode *inode) {
    printf("CHECK: %d, %d\n", inode->data.is_directory, inode->data.magic == INODE_MAGIC);
}

bool inode_is_directory(struct inode *inode) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);
    return inode->data.is_directory;
}

block_sector_t inode_get_sector(struct inode *inode) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);
    return inode->sector;
}

/* Given a certain position in a file, figure out the index in the single
   indirect block and the direct block. Store these indexes in pointers. */
static void indices_from_offset(off_t pos, size_t *dir_idx, size_t *ind_idx) {
    *dir_idx = pos / BLOCK_SECTOR_SIZE;
    *ind_idx = *dir_idx / NUM_ENTRIES_IN_INDIRECT;
    *dir_idx %= NUM_ENTRIES_IN_INDIRECT;
}


static bool inode_extend_file(struct inode_disk *data, size_t cnt) {
    ASSERT(data != NULL);
    ASSERT(data->magic == INODE_MAGIC);

    /* Get the correct count of sectors we need. */

    /* Calculate initial and final directed and indirected sector indices 
    after appending cnt bytes. */
    size_t dir_idx_f, ind_idx_f, dir_idx, ind_idx;

    /* Subtract 1 because indices are 0-indexed (byte 1 is at index 0). */
    indices_from_offset(data->length + cnt - 1, &dir_idx_f, &ind_idx_f);
    /* Note that if data->length is 0, dir_idx is 0, which is correct. */
    indices_from_offset(data->length - 1, &dir_idx, &ind_idx);

    size_t file_sectors = ((ind_idx_f - ind_idx) * NUM_ENTRIES_IN_INDIRECT + 
        (dir_idx_f - dir_idx));

    /* Add number of sectors needed for the growth of the multi-level inode. */
    size_t new_sectors = file_sectors + (ind_idx_f - ind_idx);

    /* If there are not available free sectors for allocation, return false. */
    size_t num_sectors = bitmap_size(free_map);
    if (bitmap_count(free_map, 0, num_sectors, false) < new_sectors) {
        return false;
    }

    /* Set the new file length accordingly. */
    data->length += cnt;

    /* If we don't need to allocate any new sectors, our job is finished. */
    if (new_sectors == 0) {
        return true;
    }

    /* Else, we allocate the sectors and fill them into the inode_disk data. */
    size_t num_found = 0;
    
    block_sector_t available_sectors[new_sectors];

    /* Find the sectors we need for allocation and save them in an array. */
    size_t i;
    for (i = 0; i < num_sectors; i++) {
         /* We found a sector that is free, so give it to the inode. */
        if (!bitmap_test(free_map, i)) {
            bitmap_mark(free_map, i);

            available_sectors[num_found] = i;
            num_found++;
            if (num_found == new_sectors) {
                break;
            }
        }
    }
    
    /* If we were not able to allocate all blocks that we needed or we fail 
    to persist our changes in free_map to its file, undo our changes and 
    return false. */
    if ((num_found != new_sectors) || 
        (free_map_file != NULL && !bitmap_write(free_map, free_map_file))) {

        for (i = 0; i < num_found; i++) {
            bitmap_reset(free_map, available_sectors[i]);
        }
        return false;
    }

    /* Allocate temporary buffers to hold the indexed sector data. */
    block_sector_t *dir = malloc(BLOCK_SECTOR_SIZE);
    if (dir == NULL) {
        return false;
    }
    block_sector_t *ind = malloc(BLOCK_SECTOR_SIZE);
    if (ind == NULL) {
        free(dir);
        return false;
    }

    /* Allocate temporary buffer to hold zeros for appended file data. */
    void *zeros = calloc(1, BLOCK_SECTOR_SIZE);
    if (zeros == NULL) {
        free(dir);
        free(ind);
        return false;
    }

    cache_read(data->double_indirect, ind, BLOCK_SECTOR_SIZE, 0);
    cache_read(ind[ind_idx], dir, BLOCK_SECTOR_SIZE, 0);

    /* Write all new entries into the indirected sectors. */
    i = 0;
    while (i < new_sectors) {
        dir_idx++;

        /* If we have consumed of all the current indirected sector, allocate 
        and start using a new one. */
        if (dir_idx == NUM_ENTRIES_IN_INDIRECT) {
            cache_write(ind[ind_idx], dir, BLOCK_SECTOR_SIZE, 0);
            dir_idx = 0;
            ind_idx++;
            ind[ind_idx] = available_sectors[i];
            i++;
            memset(dir, 0, BLOCK_SECTOR_SIZE);
        }

        /* Give the file a new sector for use. */
        dir[dir_idx] = available_sectors[i];

        /* Set newly appended sector to be all zeros. */
        cache_write(available_sectors[i], zeros, BLOCK_SECTOR_SIZE, 0);

        i++;
    }

    /* Finish persisting to disk our changes and free temporary buffers. */
    cache_write(ind[ind_idx], dir, BLOCK_SECTOR_SIZE, 0);
    cache_write(data->double_indirect, ind, BLOCK_SECTOR_SIZE, 0);
    free(dir);
    free(ind);
    free(zeros);

    return true;
}


/*! Initializes the inode module. */
void inode_init(void) {
    list_init(&open_inodes);
}

/*! Returns the block device sector that contains byte offset POS
    within INODE.
    Returns -1 if INODE does not contain data for a byte at offset
    POS. */
static block_sector_t byte_to_sector(const struct inode *inode, off_t pos) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);
    
    /* Get indirection indices that point to the correct sector. */
    size_t dir_idx, ind_idx;
    indices_from_offset(pos, &dir_idx, &ind_idx);
    block_sector_t sector;

    /* Temporary buffer to story indirection tables. */
    block_sector_t *buffer = malloc(sizeof(block_sector_t));
    
    /* Find the single-indirect table from the double-indirect, if it exists. */
    cache_read(inode->data.double_indirect, buffer, sizeof(block_sector_t), 
        sizeof(block_sector_t) * ind_idx);
    sector = *buffer;

    /* It doesn't exist, so return -1. */
    if (!sector) {
        free(buffer);
        return -1;
    }

    /* Find the single-direct sector from the single-indirect, if it exists. */
    cache_read(sector, buffer, sizeof(block_sector_t), 
        sizeof(block_sector_t) * dir_idx);
    sector = *buffer;
    free(buffer);

    /* It doesn't exist, so return -1. */
    if (!sector) {
        return -1;
    }

    return sector;
}


/* Essentially a function for debugging purposes. Prints all sectors of a file 
that the inode links to. */
static void print_inode_allocation(struct inode_disk *data) {
    ASSERT(data != NULL);
    ASSERT(data->magic == INODE_MAGIC);

    int i;

    block_sector_t *ind = malloc(BLOCK_SECTOR_SIZE);
    ASSERT(ind);
    block_sector_t *dir = malloc(BLOCK_SECTOR_SIZE);
    ASSERT(dir);

    cache_read(data->double_indirect, ind, BLOCK_SECTOR_SIZE, 0);
    while (*ind) {
        printf("%d:", *ind);
        cache_read(*ind, dir, BLOCK_SECTOR_SIZE, 0);
        for (i = 0; i < NUM_ENTRIES_IN_INDIRECT; i++) {
            if (!*(dir + i)) {
                break;
            }
            printf(" %d", *(dir + i));
        }
        printf("\n");
        if (i != NUM_ENTRIES_IN_INDIRECT) {
            break;
        }
        ind++;
    }
}


/*! Initializes an inode with LENGTH bytes of data and
    writes the new inode to sector SECTOR on the file system
    device.
    Returns true if successful.
    Returns false if memory or disk allocation fails. */
bool inode_create(block_sector_t sector, off_t length, bool is_directory) {
    struct inode_disk *disk_inode = NULL;
    bool success = false;

    ASSERT(length >= 0);

    /* If this assertion fails, the inode structure is not exactly
       one sector in size, and you should fix that. */
    ASSERT(sizeof *disk_inode == BLOCK_SECTOR_SIZE);

    disk_inode = calloc(1, sizeof *disk_inode);
    if (disk_inode != NULL) {
        disk_inode->length = 0;
        disk_inode->magic = INODE_MAGIC;
        disk_inode->is_directory = is_directory;

        /* Create multilevel indirection into inode. */
        block_sector_t *zeros = calloc(1, BLOCK_SECTOR_SIZE);
        block_sector_t ind_sector, dir_sector;

        if (zeros != NULL) {
            /* Allocate the first double-indirect and indirect tables (0, 0). */
            if (free_map_allocate_single(&disk_inode->double_indirect)) {
                if (free_map_allocate_single(&ind_sector)) {
                    if (free_map_allocate_single(&dir_sector)) {
                        success = true;
                    } else {
                        free_map_release_single(disk_inode->double_indirect);
                        free_map_release_single(ind_sector);
                    }
                } else {
                    free_map_release_single(disk_inode->double_indirect);
                }
            }
            /* Write the sector values to these tables corresponding to the 
            allocations. */
            if (success) {
                /* Write the tables to disk, with the appropriate sector 
                of the indirect table in the double-indirect table. */
                cache_write(dir_sector, zeros, BLOCK_SECTOR_SIZE, 0);
                *zeros = dir_sector;
                cache_write(ind_sector, zeros, BLOCK_SECTOR_SIZE, 0);
                *zeros = ind_sector;
                cache_write(disk_inode->double_indirect, zeros, 
                    BLOCK_SECTOR_SIZE, 0);
            }
            free(zeros);

            /* Now allocate the space for the file contents (beyond (0, 0)). */
            if (success && !inode_extend_file(disk_inode, length)) {
                /* If the allocation fails, release sectors used by inode. */
                free_map_release_single(ind_sector);
                free_map_release_single(disk_inode->double_indirect);
                success = false;
            } else {
                /* Else, all allocations succeeded, so write inode to disk. */
                cache_write(sector, disk_inode, BLOCK_SECTOR_SIZE, 0);
            }
        }
        free(disk_inode);
    }

    return success;
}

/*! Reads an inode from SECTOR
    and returns a `struct inode' that contains it.
    Returns a null pointer if memory allocation fails. */
struct inode * inode_open(block_sector_t sector) {
    struct list_elem *e; 
    struct inode *inode;

    /* Check whether this inode is already open. */
    for (e = list_begin(&open_inodes); e != list_end(&open_inodes);
         e = list_next(e)) {
        inode = list_entry(e, struct inode, elem);
        if (inode->sector == sector) {
            inode_reopen(inode);
            return inode; 
        }
    }

    /* Allocate memory. */
    inode = malloc(sizeof *inode);
    if (inode == NULL)
        return NULL;

    /* Initialize. */
    list_push_front(&open_inodes, &inode->elem);
    inode->sector = sector;
    inode->open_cnt = 1;
    inode->deny_write_cnt = 0;
    inode->removed = false;
    inode->file_count = 0;
    lock_init(&inode->extension_lock);
    cache_read(inode->sector, &inode->data, BLOCK_SECTOR_SIZE, 0);
    ASSERT(inode->data.magic == INODE_MAGIC);
    return inode;
}

/*! Reopens and returns INODE. */
struct inode * inode_reopen(struct inode *inode) {
    if (inode != NULL) {
        ASSERT(inode->data.magic == INODE_MAGIC);
        inode->open_cnt++;
    }
    return inode;
}

/*! Returns INODE's inode number. */
block_sector_t inode_get_inumber(const struct inode *inode) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);

    return inode->sector;
}

/*! Closes INODE and writes it to disk.
    If this was the last reference to INODE, frees its memory.
    If INODE was also a removed inode, frees its blocks. */
void inode_close(struct inode *inode) {

    int i;
    /* Ignore null pointer. */
    if (inode == NULL)
        return;

    ASSERT(inode->data.magic == INODE_MAGIC);

    /* Release resources if this was the last opener. */
    if (--inode->open_cnt == 0) {
        /* Remove from inode list and release lock. */
        list_remove(&inode->elem);
 
        /* Persist changes to inode to disk. */
        cache_write(inode->sector, &inode->data, BLOCK_SECTOR_SIZE, 0);

        /* Deallocate blocks if removed. */
        if (inode->removed) {
            block_sector_t *ind = malloc(BLOCK_SECTOR_SIZE);
            ASSERT(ind);
            block_sector_t *dir = malloc(BLOCK_SECTOR_SIZE);
            ASSERT(dir);

            cache_read(inode->data.double_indirect, ind, BLOCK_SECTOR_SIZE, 0);
            while (*ind) {
                cache_read(*ind, dir, BLOCK_SECTOR_SIZE, 0);
                for (i = 0; i < NUM_ENTRIES_IN_INDIRECT; i++) {
                    if (*(dir + i)) {
                        break;
                    }
                    /* Free the direct (file contents) sector. */
                    free_map_release_single(*(dir + i));
                }
                /* Free the single indirect sector. */
                free_map_release_single(*ind);
                if (i != NUM_ENTRIES_IN_INDIRECT) {
                    break;
                }
                ind++;
            }

            /* Free the double indirect sector. */
            free_map_release_single(inode->data.double_indirect);

            /* Free inode itself. */
            free_map_release_single(inode->sector);
        }

        free(inode); 
    }
}

/*! Marks INODE to be deleted when it is closed by the last caller who
    has it open. */
void inode_remove(struct inode *inode) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);

    inode->removed = true;
}

/* Tells if an inode has been removed. */
bool inode_is_removed(struct inode *inode) {
    ASSERT(inode != NULL);

    return inode->removed;
}

/*! Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t inode_read_at(struct inode *inode, void *buffer_, off_t size, off_t offset) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);

    uint8_t *buffer = buffer_;
    off_t bytes_read = 0;

    while (size > 0) {
        /* Disk sector to read, starting byte offset within sector. */
        block_sector_t sector_idx = byte_to_sector (inode, offset);

        int sector_ofs = offset % BLOCK_SECTOR_SIZE;

        /* Bytes left in inode, bytes left in sector, lesser of the two. */
        off_t inode_left = inode_length(inode) - offset;
        int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
        int min_left = inode_left < sector_left ? inode_left : sector_left;

        /* Number of bytes to actually copy out of this sector. */
        int chunk_size = size < min_left ? size : min_left;
        if (chunk_size <= 0)
            break;

        cache_read(sector_idx, buffer + bytes_read, chunk_size, sector_ofs);
      
        /* Advance. */
        size -= chunk_size;
        offset += chunk_size;
        bytes_read += chunk_size;
    }
    return bytes_read;
}

/*! Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
    Returns the number of bytes actually written, which may be
    less than SIZE if end of file is reached or an error occurs. */
off_t inode_write_at(struct inode *inode, const void *buffer_, off_t size, 
        off_t offset) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);

    const uint8_t *buffer = buffer_;
    off_t bytes_written = 0;

    if (inode->deny_write_cnt)
        return 0;

    /* If we are writing beyond the file’s length (in sectors), we might be
    extending the file. */
    off_t write_position = offset + size;

   /* Note that this condition doesn’t guarentee a new sector is needed,
    but in case it isn’t inode_extend_file(). */
    if (write_position >= inode->data.length) {
        /* Get the lock and then check again that we are extending. */
        lock_acquire(&inode->extension_lock);

        if (write_position >= inode->data.length) {
            /* We are, so extend the file. */
            inode_extend_file(&inode->data, write_position-inode->data.length);
        }
        lock_release(&inode->extension_lock);
    }

    while (size > 0) {
        /* Sector to write, starting byte offset within sector. */
        block_sector_t sector_idx = byte_to_sector(inode, offset);

        int sector_ofs = offset % BLOCK_SECTOR_SIZE;

        /* Bytes left in inode, bytes left in sector, lesser of the two. */
        off_t inode_left = inode_length(inode) - offset;
        int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
        int min_left = inode_left < sector_left ? inode_left : sector_left;

        /* Number of bytes to actually write into this sector. */
        int chunk_size = size < min_left ? size : min_left;
        if (chunk_size <= 0)
            break;


        cache_write(sector_idx, buffer + bytes_written, chunk_size, sector_ofs);

        /* Advance. */
        size -= chunk_size;
        offset += chunk_size;
        bytes_written += chunk_size;
    }

    return bytes_written;
}

/*! Disables writes to INODE.
    May be called at most once per inode opener. */
void inode_deny_write (struct inode *inode) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);

    inode->deny_write_cnt++;
    ASSERT(inode->deny_write_cnt <= inode->open_cnt);
}

/*! Re-enables writes to INODE.
    Must be called once by each inode opener who has called
    inode_deny_write() on the inode, before closing the inode. */
void inode_allow_write (struct inode *inode) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);

    ASSERT(inode->deny_write_cnt > 0);
    ASSERT(inode->deny_write_cnt <= inode->open_cnt);
    inode->deny_write_cnt--;
}

/*! Returns the length, in bytes, of INODE's data. */
off_t inode_length(const struct inode *inode) {
    ASSERT(inode != NULL);
    ASSERT(inode->data.magic == INODE_MAGIC);

    return inode->data.length;
}

void inode_incr_count(struct inode *inode) {
    ASSERT(inode != NULL);

    inode->file_count++;
}

void inode_decr_count(struct inode *inode) {
    ASSERT(inode != NULL);

    inode->file_count--;

    /* Just to make sure rest of code functions properly. */
    ASSERT(inode->file_count >= 0);
}

int inode_num_files(struct inode *inode) {
    ASSERT(inode != NULL);
    ASSERT(inode->file_count >= 0);

    return inode->file_count;
}


#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/bitmap.h>
#include "vm/swap.h"
#include "devices/block.h"
#include "threads/synch.h"

static struct bitmap *swap_slots;  /* 1 if allocated/in-use, 0 if available. */
static int swap_num_slots;    /* Number of slots in swap_slots */
static struct block *swap_block;   /* Swap block device */

inline static block_sector_t swap_slot_to_sector(swapslot_t swap_slot);

/* Initialize bitmap used to check which swap-slots are available. */
void swap_init(void) {
    swap_block = block_get_role(BLOCK_SWAP);
    // TODO: handle case that swap block doesn't exist
    swap_num_slots = block_size(swap_block) / SECTORS_PER_PAGE;
    swap_slots = bitmap_create(swap_num_slots);

    lock_init(&swap_lock);

    // TODO: delete this print statement
    printf("Number of swap slots available at start: %d\n", swap_num_slots);
}


/* Writes PGSIZE of data at addr into swap_slot. */
void swap_write(swapslot_t swap_slot, void *addr) {
    if (addr == 0) {
        PANIC("WHOOPS!");
    }
    lock_acquire(&swap_lock);
    block_sector_t first_sec = swap_slot_to_sector(swap_slot);

    for (int s = 0; s < SECTORS_PER_PAGE; s++) {
        // TODO: this assumes that BLOCK_SECTOR_SIZE < PGSIZE (which is true)
        // TODO: this also assumes that PGSIZE is a multiple of BLOCK_SECTOR_SIZE
        block_write(swap_block, first_sec + s, addr + (BLOCK_SECTOR_SIZE * s));
    }
    lock_release(&swap_lock);
}


/* Reads PGSIZE of data at swap_slot into addr. */
void swap_read(swapslot_t swap_slot, void *addr) {
    if (addr == 0) {
        PANIC("WHOOPS!");
    }
    lock_acquire(&swap_lock);
    block_sector_t first_sec = swap_slot_to_sector(swap_slot);

    for (int s = 0; s < SECTORS_PER_PAGE; s++) {
        // TODO: this assumes that BLOCK_SECTOR_SIZE < PGSIZE (which is true)
        // TODO: this also assumes that PGSIZE is a multiple of BLOCK_SECTOR_SIZE
        block_read(swap_block, first_sec + s, addr + (BLOCK_SECTOR_SIZE * s));
    }
    lock_release(&swap_lock);
}


/* Returns an available swap slot and returns its swap number. If none are 
available, panic. */
swapslot_t swap_alloc(void) {
    lock_acquire(&swap_lock);
    size_t swap_slot = bitmap_scan_and_flip(swap_slots, 0, 1, false);
    // printf("SWAP: ALLOC %d\n", swap_slot);
    
    if (swap_slot == BITMAP_ERROR) {
        PANIC("Out of swap slots!");
    }
    
    lock_release(&swap_lock);
    return swap_slot;
}


/* Asserts that swap slot has been allocated and frees it. */
void swap_free(swapslot_t swap_slot) {
    // printf("SWAP: FREE %d\n", swap_slot);
    lock_acquire(&swap_lock);
    ASSERT(bitmap_test(swap_slots, swap_slot));
    bitmap_set(swap_slots, swap_slot, false);
    lock_release(&swap_lock);
}


/* Asserts that swap slot is a valid allocated index in swap_num_slots and 
returns the sector in block device corresponding to the swap_slot. */
inline static block_sector_t swap_slot_to_sector(swapslot_t swap_slot) {
    ASSERT(bitmap_test(swap_slots, swap_slot));

    return swap_slot * SECTORS_PER_PAGE;
}


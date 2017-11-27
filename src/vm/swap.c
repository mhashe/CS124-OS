#include <debug.h>
#include <kernel/bitmap.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "devices/block.h"
#include "vm/swap.h"

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

}


/* Writes PGSIZE of data at addr into swap_slot. */
void swap_write(swapslot_t swap_slot, void *addr) {
    block_sector_t first_sec = swap_slot_to_sector(swap_slot);

    for (int s = 0; s < SECTORS_PER_PAGE; s++) {
        // TODO: this assumes that BLOCK_SECTOR_SIZE < PGSIZE (which is true)
        // TODO: this also assumes that PGSIZE is a multiple of BLOCK_SECTOR_SIZE
        block_write(swap_block, first_sec + s, addr + (BLOCK_SECTOR_SIZE * s));
    }
}


/* Reads PGSIZE of data at swap_slot into addr. */
void swap_read(swapslot_t swap_slot, void *addr) {
    block_sector_t first_sec = swap_slot_to_sector(swap_slot);

    for (int s = 0; s < SECTORS_PER_PAGE; s++) {
        // TODO: this assumes that BLOCK_SECTOR_SIZE < PGSIZE (which is true)
        // TODO: this also assumes that PGSIZE is a multiple of BLOCK_SECTOR_SIZE
        block_read(swap_block, first_sec + s, addr + (BLOCK_SECTOR_SIZE * s));
    }
}


/* Returns an available swap slot and returns its swap number. If none are 
available, panic. */
swapslot_t swap_alloc(void) {
    size_t swap_slot = bitmap_scan_and_flip(swap_slots, 0, 1, false);
    
    if (swap_slot == BITMAP_ERROR) {
        PANIC("Out of swap slots!");
    }
    
    return swap_slot;
}


/* Asserts that swap slot has been allocated and frees it. */
void swap_free(swapslot_t swap_slot) {
    ASSERT(bitmap_test(swap_slots, swap_slot));
    bitmap_set(swap_slots, swap_slot, false);
}


/* Asserts that swap slot is a valid allocated index in swap_num_slots and 
returns the sector in block device corresponding to the swap_slot. */
inline static block_sector_t swap_slot_to_sector(swapslot_t swap_slot) {
    ASSERT(bitmap_test(swap_slots, swap_slot));

    return swap_slot * SECTORS_PER_PAGE;
}


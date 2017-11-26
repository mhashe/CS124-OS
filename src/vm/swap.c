#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/bitmap.h>
// #include "vm/page.h"
// #include "vm/frame.h"
// #include "threads/palloc.h"
// #include "threads/malloc.h"
// #include "threads/vaddr.h"
// #include "threads/pte.h"
// #include "threads/thread.h"
// #include "filesys/filesys.h"  /* For filesys ops. */
// #include "filesys/file.h"     /* For file ops. */
// #include "userprog/syscall.h"
// #include "userprog/pagedir.h"
#include "devices/block.h"

/* 1 if allocated/in-use, 0 if available. */
static struct bitmap *swap_slots;
static struct block *swap_block;
static uint32_t swap_num_slots;


/* Initialize bitmap used to check which swap-slots are available. */
void swap_init(void) {
    swap_block = block_get_role(BLOCK_SWAP);
    // TODO: handle case that block doesn't exist
    swap_num_slots = block_size(swap_block) / SECTORS_PER_PAGE;
    swap_slots = bitmap_create(swap_num_slots);

    // TODO: delete this print statement
    printf("Number of swap slots available at start: %d\n", swap_num_slots);
}

/* Writes data at addr into swap_slot. */
void swap_write(size_t swap_slot, void *addr) {
    int s;

    /* Assert the swap slot is a valid index and that it is allocated. */
    ASSERT((swap_slot <= (swap_num_slots - SECTORS_PER_PAGE)) 
        && (swap_slot >= 0));
    for (s = 0; s < SECTORS_PER_PAGE; s++) {
        ASSERT(bitmap_test(swap_slots, swap_slot));
    }
    // TODO: if assert is expensive, perhaps just change a local var and once when done?
    // TODO: assert the addr is a valid address? or is that overkill?

    /* Write page at addr to swap slot. */
    for (s = 0; s < SECTORS_PER_PAGE; s++) {
        // TODO: this assumes that BLOCK_SECTOR_SIZE < PGSIZE (which is true)
        block_write(swap_block, swap_slot + s, addr + (BLOCK_SECTOR_SIZE * s));
    }
}


/* Reads data at swap_slot into addr. */
void swap_read(size_t swap_slot, void *addr) {
    // TODO
}


/* Returns an available swap slot and returns its swap number. If none are 
available, panic. */
size_t swap_alloc(void) {
    size_t swap_slot = bitmap_scan_and_flip(swap_slots, 0, 1, false);
    
    if (swap_slot == BITMAP_ERROR) {
        // TODO: panic the kernel b/c we ran out of slots
    }
    
    return swap_slot;
}


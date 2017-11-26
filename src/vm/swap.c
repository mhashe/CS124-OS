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

/* 1 if used, 0 if available. */
static struct bitmap *swap_slots;


/* Initialize bitmap used to check which swap-slots are available. */
void swap_init(void) {
    swap_slots = bitmap_create(size_t size);
}

/* Writes data at addr into swap_slot. */
void swap_write(size_t swap_slot, void *addr) {
    // TODO

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


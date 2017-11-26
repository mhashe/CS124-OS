#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

static struct bitmap *swap_available;

/* Initialize bitmap used to check which swap-slots are available. */
void swap_init(void) {
    // TODO
}

/* Writes data at addr into swap_slot. */
void swap_write(int swap_slot, void *addr) {
    // TODO
}

/* Reads data at swap_slot into addr. */
void swap_read(int swap_slot, void *addr) {
    // TODO
}


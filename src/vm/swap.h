/*! \file swap.h
 *
 */

#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <debug.h>
#include <stdint.h>
#include "threads/vaddr.h"
#include "devices/block.h"
// #include "userprog/syscall.h"
// #include "threads/pte.h"

/* Set sectors per page to be pgsize/block_sector_size rounded up. */
#define SECTORS_PER_PAGE (PGSIZE + BLOCK_SECTOR_SIZE - 1) / BLOCK_SECTOR_SIZE

void swap_init(void);

void swap_write(size_t swap_slot, void *addr);

void swap_read(size_t swap_slot, void *addr);

size_t swap_alloc(void);


#endif /* vm/swap.h */


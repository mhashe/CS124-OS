/*! \file swap.h
 *
 */

#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <debug.h>
#include <stdint.h>
#include "threads/vaddr.h"
#include "devices/block.h"

/* Set sectors per page to be pgsize/block_sector_size rounded up. */
#define SECTORS_PER_PAGE ((PGSIZE + BLOCK_SECTOR_SIZE - 1) / BLOCK_SECTOR_SIZE)

typedef size_t swapslot_t;

void swap_init(void);

void swap_write(swapslot_t swap_slot, void *addr);

void swap_read(swapslot_t swap_slot, void *addr);

swapslot_t swap_alloc(void);

void swap_free(swapslot_t swap_slot);


#endif /* vm/swap.h */


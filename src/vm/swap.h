/*! \file swap.h
 *
 */

#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <debug.h>
#include <stdint.h>
// #include "userprog/syscall.h"
// #include "threads/pte.h"

void swap_init(void);

void swap_write(size_t swap_slot, void *addr);

void swap_read(size_t swap_slot, void *addr);

size_t swap_alloc(void);


#endif /* vm/swap.h */


#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <list.h>

#include "vm/page.h"
// #include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/pte.h"
#include "threads/thread.h"
#include "threads/malloc.h"

/* Returns 0 on success, -1 on failure. */
int sup_load_file(uint32_t * vaddr, struct file_des *fd, int offset) {
    struct list *spd = &thread_current()->sup_pagedir;
    
    // Make sure that this vaddr is empty
    struct sup_entry* spe = sup_get_entry(vaddr, spd);
    if (spe != NULL) {
        return -1;
    }
    // TODO: If this address is not a valid address for other reasons, return -1

    spe = malloc(sizeof(struct sup_entry));
    list_push_back(spd, &spe->elem);
    spe->fd = fd;
    spe->file_ofs = offset;

    return 0;
}

struct sup_entry* sup_get_entry(uint32_t * vaddr, struct list *spd) {
    struct list_elem *spe_elem = list_begin(spd);
    struct sup_entry *spe;

    if (spe_elem != list_end(spd)) {
        struct list_elem *e;
        for (e = list_next(spe_elem); e != list_end(spd); e = list_next(e)) {
            spe = list_entry(e, struct sup_entry, elem);
            if (spe->vaddr == vaddr) {
                return spe;
            }
        }
    }

    return NULL;
}




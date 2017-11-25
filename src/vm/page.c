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
#include "vm/frame.h"
#include "userprog/syscall.h"

/* Allocates entire file in as many pages as needed in supplementary page 
table. To be called in mmap. Returns 0 on success, -1 on failure. */
int sup_alloc_file(uint32_t * vaddr, struct file_des *fd) {
    struct list *spd = &thread_current()->sup_pagedir;
    
    int offset = pg_ofs(vaddr);

    // Make sure that this vaddr is empty
    int num_pages = (offset + filesize(fd)) / PGSIZE;
    num_pages += ((num_pages % PGSIZE) != 0);
    vaddr = pg_round_down(vaddr);
    
    for (int page = 0; page < num_pages; page ++) {
        uint32_t *addr = (vaddr + (PGSIZE * page));
        struct sup_entry* spe = sup_get_entry(addr, spd);

        // TODO: If this address is not a valid address for other reasons, return -1

        if (spe != NULL) {
            return -1;
        }
    }
    
    for (int page = 0; page < num_pages; page ++) {
        uint32_t *addr = (vaddr + (PGSIZE * page));
        struct sup_entry* spe = sup_get_entry(addr, spd);

        spe =(struct sup_entry *) malloc(sizeof(struct sup_entry));
        list_push_back(spd, &spe->elem);
        spe->fd = fd;
        spe->file_ofs = offset + (PGSIZE * page);
    }

    return 0;
}


// TODO: comment
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


/* Loads part of file needed at vaddr page. Returns 0 on success, -1 on 
failure. */
int sup_load_file(uint32_t *vaddr, bool user) {
    struct sup_entry * spe = sup_get_entry(pg_round_down(vaddr), 
        &thread_current()->sup_pagedir);

    if (spe == NULL) {
        return -1;
    }

    int frame_no = get_frame(vaddr, user);

    // TODO: load part of file into frame
    // Is it possible to read in a certain offset in a file? 
    // read(spe->fd, (addr of frame), PG_SIZE, spe->file_ofs)

    return 0;
}



#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vm/page.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/pte.h"
#include "threads/thread.h"

struct sup_entry *** sup_pagedir_create(void) {
    struct sup_entry ***sup_pagedir;
    struct sup_entry **sup_t;

    sup_pagedir = (struct sup_entry ***) palloc_get_page(PAL_ASSERT | PAL_ZERO);
    sup_t = NULL;
    for (size_t page = 0; page < init_ram_pages; page++) {
        char *vaddr = ptov(page * PGSIZE);
        size_t pde_idx = pd_no(vaddr);
        size_t pte_idx = pt_no(vaddr);

        // TODO: is 0 the same as null? counting on that here...
        if (sup_pagedir[pde_idx] == NULL) {
            sup_t = (struct sup_entry **) palloc_get_page(PAL_ASSERT | PAL_ZERO);
            sup_pagedir[pde_idx] = (struct sup_entry **) sup_t;
        }

        sup_t[pte_idx] = NULL;
    }

    return sup_pagedir;
}


void sup_load_file(uint32_t * vaddr, struct file_des *fd, int offset) {
    struct thread *cur = thread_current();
    struct sup_entry **sup_t = *(cur->sup_pagedir + pd_no(vaddr));
    struct sup_entry *se = *(sup_t + pt_no(vaddr));
}





#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vm/page.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/pte.h"


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



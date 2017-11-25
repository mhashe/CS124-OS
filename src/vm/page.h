/*! \file page.h
 *
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdint.h>
#include <list.h>


struct sup_entry {
    uint32_t *vaddr;
    struct file_des *fd;     /* pointer to file to be opened */
    int file_ofs;            /* file loaded into page at fd's offset */

    struct list_elem elem;
};

int sup_load_file(uint32_t * vaddr, struct file_des *fd, int offset);
struct sup_entry* sup_get_entry(uint32_t * vaddr, struct list *spd);

#endif /* vm/page.h */


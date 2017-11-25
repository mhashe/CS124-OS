/*! \file page.h
 *
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdint.h>

struct sup_entry {
    struct file_des *fd;     /* pointer to file to be opened */
    int file_ofs;            /* file loaded into page at offset */
};

struct sup_entry *** sup_pagedir_create(void);

#endif /* vm/page.h */


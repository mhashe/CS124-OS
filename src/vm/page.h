/*! \file page.h
 *
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdint.h>
#include "threads/thread.h"

struct supplement_pte {
    struct file_des *fd;     /* pointer to file to be opened */
    int file_ofs;            /* file loaded into page at offset */
};


#endif /* vm/page.h */


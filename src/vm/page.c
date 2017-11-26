#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vm/page.h"
#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "threads/pte.h"
#include "threads/thread.h"
#include "filesys/filesys.h"  /* For filesys ops. */
#include "filesys/file.h"     /* For file ops. */
#include "userprog/syscall.h"
#include "userprog/pagedir.h"

static inline struct sup_entry *sup_get_entry(void *vaddr, 
    struct sup_entry ***sup_pagedir);

static inline void sup_set_entry(void *vaddr, struct sup_entry *** sup_pagedir, 
    struct sup_entry *entry);

static inline void sup_remove_entry(void *upage, struct sup_entry *** 
    sup_pagedir);

/* Functions from syscall. TODO: how to not reimplement them here? */
static int filesize(int fd);
static struct file_des *file_from_fd(int fd);
static int sup_read(int fd, void* buffer, unsigned size, unsigned offset);
static int sup_write(int fd, void* buffer, unsigned size, unsigned offset);


/* Allocates and returns a pointer to an empty supplementary table. */
struct sup_entry *** sup_pagedir_create(void) {
    return (struct sup_entry ***) palloc_get_page(PAL_ASSERT | PAL_ZERO);
}


/* Allocate an all zero page and Returns 0 on success, -1 on failure. */
int sup_all_zeros(void * vaddr, bool user) {
    struct thread *cur = thread_current();
    struct sup_entry *spe;

    /* Allocate and get physical frame for data to be loaded into. */
    uint32_t frame_no = get_frame(user);
    void *kpage = ftov(frame_no);

    /* If the provided address is not page-aligned, return failure. */
    int offset = pg_ofs(vaddr);
    if (offset != 0) {
        free_frame(frame_no);
        return -1;
    }

    /* If the page specified by vaddr is already occupied, return failure. */
    if (sup_get_entry(vaddr, cur->sup_pagedir) != NULL) {
        free_frame(frame_no);
        return -1;
    }
 
    /* Linking frame to virtual address failed, so remove and deallocate the 
    page instantiated for it. */
    if (!pagedir_set_page(cur->pagedir, vaddr, kpage, true)) {
        free_frame(frame_no);
        return -1;
    }

    /* Create supplmentary entry corresponding to an initially all zero page. */
    spe = (struct sup_entry *) malloc(sizeof(struct sup_entry));
    spe->fd = -1;
    spe->file_ofs = 0;
    spe->page_end = PGSIZE;
    spe->writable = true;
    spe->loaded = true;
    spe->all_zero = true;
    spe->frame_no = frame_no;
    spe->mapid = MAP_FAILED;

    sup_set_entry(vaddr, cur->sup_pagedir, spe);

    return 0;
}

/* Allocates entire file in as many pages as needed in supplementary page 
table. The file is given by "int fd" and it is writable if "writeable". To 
be called in mmap. Returns entry on success, NULL on failure. */
int sup_alloc_file(void * vaddr, int fd, bool writable) {
    static mapid_t last_mapid = 0;

    /* The provided address must be page aligned. */
    int offset = pg_ofs(vaddr);
    if (offset != 0) {
        return MAP_FAILED;
    }

    /* Current thread's supplemental page directory. */
    struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;
    

    /* Calculate the number of pages required to allocate file. */
    int file_size = filesize(fd);
    int num_pages = file_size / PGSIZE;
    num_pages += ((file_size % PGSIZE) != 0);

    /* Start allocating page for file in supplemental table at page-align. */
    vaddr = pg_round_down(vaddr);

    /* Check if needed pages are available in the supplementary table. */
    for (int page = 0; page < num_pages; page++) {
        void* addr = (vaddr + (PGSIZE * page));
        struct sup_entry* spe = sup_get_entry(addr, sup_pagedir);

        // TODO: If this address is not a valid address for other reasons, return -1

        /* There are not enough free pages to allocate the file, so fail. */
        if (spe != NULL) {
            return MAP_FAILED;
        }
    }

    last_mapid++;


    /* Add the needed pages to the supplemental table with correct file. */
    for (int page = 0; page < num_pages; page ++) {
        void *addr = (vaddr + (PGSIZE * page));
        struct sup_entry* spe = sup_get_entry(addr, sup_pagedir);

        spe = (struct sup_entry *) malloc(sizeof(struct sup_entry));
        spe->fd = fd;
        spe->file_ofs = (unsigned) (PGSIZE * page);
        if (page == num_pages - 1) {
            spe->page_end = file_size % PGSIZE;
        } else {
            spe->page_end = PGSIZE;
        }
        spe->writable = writable;
        spe->loaded = false;
        spe->frame_no = (uint32_t) -1;
        spe->mapid = last_mapid;
        spe->all_zero = false;
        sup_set_entry(addr, sup_pagedir, spe);
    }

    return last_mapid;
}


/* Loads part of file needed at vaddr page. Returns 0 on success, -1 on 
failure. */
int sup_load_file(void *vaddr, bool user, bool write) {
    struct thread *cur = thread_current();
    void *upage = pg_round_down(vaddr);
    struct sup_entry * spe = sup_get_entry(upage, cur->sup_pagedir);

    /* If entry in supplementary page table does not exist, then failure. */
    if (spe == NULL) {
        return -1;
    }

    /* Unknown page fault since data has been loaded already. */
    if (spe->loaded) {
        return -1;
    }

    /* Invalid access if page fault was due to write attempt on r/only page. */
    if (write && (!spe->writable)) {
        return -1;
    }

    /* Allocate and get physical frame for data to be loaded into. */
    uint32_t frame_no = get_frame(user);
    void *kpage = ftov(frame_no);

    /* Load one page of the file at file_ofs into the frame. */
    if (sup_read(spe->fd, kpage, spe->page_end, spe->file_ofs) == -1) {
        free_frame(frame_no);
        return -1;
    }

    /* Linking frame to virtual address failed, so remove and deallocate the 
    page instantiated for it. */
    if (!pagedir_set_page(cur->pagedir, upage, kpage, spe->writable)) {
        free_frame(frame_no);
        return -1;
    }

    spe->frame_no = frame_no;
    spe->loaded = true;

    return 0;
}




/* Deallocate and remove file from supplementary page table. */
void sup_remove_map(mapid_t mapid) {
    struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;
    struct sup_entry *entry;
    // struct sup_entry* entry = sup_get_entry(upage, sup_pagedir);

    for (uint32_t i = 0; i < PGSIZE / sizeof(struct sup_entry **); i++) {
        if (!sup_pagedir[i]) {
            continue;
        }
        for (uint32_t j = 0; j < PGSIZE / sizeof(struct sup_entry *); j++) {
            entry = sup_pagedir[i][j];
            if (!entry || entry->mapid != mapid) {
                continue;
            }
            // pagedir_clear_page()
            if (entry->loaded) {
                sup_write(entry->fd, ftov(entry->frame_no), 
                    entry->page_end, entry->file_ofs);
                free_frame(entry->frame_no);
            }
            void *vaddr = index_to_vaddr(i, j);

            pagedir_clear_page(thread_current()->pagedir, vaddr);
            sup_remove_entry(vaddr, sup_pagedir);
        }
    }
}

/* Free all allocated pages and entries in the supplementary page table. 
TODO: Add this to process exit! */
void sup_free_table(struct sup_entry ***sup_pagedir) {
    for (uint32_t i = 0; i < PGSIZE / sizeof(struct sup_entry **); i++) {
        if (!sup_pagedir[i]) {
            continue;
        }
        for (uint32_t j = 0; j < PGSIZE / sizeof(struct sup_entry *); j++) {
            if (sup_pagedir[i][j]) {
                free(sup_pagedir[i][j]);
            }
        }
        palloc_free_page(sup_pagedir[i]);
    }
    palloc_free_page(sup_pagedir);

}


/* Removes supplemental entry from sup_pagedir at upage, which must be 
page-aligned. Assumes enty exists. */
static inline void sup_remove_entry(void *upage, struct sup_entry 
    *** sup_pagedir) {
    // TODO: make this computationally more efficient with local vars
    free(sup_pagedir[pd_no(upage)][pt_no(upage)]);
    sup_pagedir[pd_no(upage)][pt_no(upage)] = NULL;
    // TODO: free entire table if nothing is left?
}


/* Retreives supplemental entry from sup_pagedir at upage, which must be 
page-aligned. */
static inline struct sup_entry *sup_get_entry(void *upage, 
                                            struct sup_entry ***sup_pagedir) {
    uintptr_t pd = pd_no(upage);
    if (sup_pagedir[pd] == NULL) {
        return NULL;
    }
    return sup_pagedir[pd][pt_no(upage)];
}


/* Sets supplemental entry from sup_pagedir at upage to be entry. upage must 
be page-aligned. */
static inline void sup_set_entry(void *upage, struct sup_entry ***sup_pagedir, 
                                struct sup_entry *entry) {
    uintptr_t pd = pd_no(upage);
    if (sup_pagedir[pd] == NULL) {
        sup_pagedir[pd] = palloc_get_page(PAL_ASSERT | PAL_ZERO);
    }
    sup_pagedir[pd][pt_no(upage)] = entry;
}





/* Returns the size, in bytes, of the file open as fd. */
static int filesize(int fd) {
    int filesize;
    /* Special cases. */
    if (fd == STDIN_FILENO || fd == STDOUT_FILENO) {
        thread_exit();
    }

    /* Return file size. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        // lock_acquire(&filesys_io);                // LOCKS HAVE BEEN REMOVED
        filesize = file_length(file);
        // lock_release(&filesys_io);                // LOCKS HAVE BEEN REMOVED
    } else {
        /* Invalid file has no size. */
        thread_exit();
    }

    return filesize;
}

/* Each thread maintains a list of its file descriptors. Get the file object 
   associated with the file descriptor. */
static struct file_des *file_from_fd(int fd) {
    /* 0, 1 reserved for STDIN, STDOUT. */
    ASSERT(fd > STDIN_FILENO);
    ASSERT(fd > STDOUT_FILENO);

    /* Iterate over file descriptors, return if one matches. */
    struct list_elem *e;
    struct list *lst = &thread_current()->fds;
    struct file_des* fd_s;

    for (e = list_begin(lst); e != list_end(lst); e = list_next(e)) {
        /* File descriptor object. */
        fd_s = list_entry(e, struct file_des, elem);

        /* If a match, return. */
        if (fd_s->fd == fd) {
            return fd_s;
        }
    }

    /* Invalid file descriptor; terminate offending process. */
    thread_exit();
}

/* Reads size bytes from the file open as fd into buffer. Returns the number of
   bytes actually read (0 at end of file), or -1 if the file could not be read
   (due to a condition other than end of file). Fd 0 reads from the keyboard
   using input_getc(). */
static int sup_read(int fd, void* buffer, unsigned size, unsigned offset) {
    int bytes;

    ASSERT(fd > 1);
    ASSERT(is_kernel_vaddr(buffer));

    /* Return number of bytes read. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        // lock_acquire(&filesys_io);                // LOCKS HAVE BEEN REMOVED
        file_seek(file, offset);  
        bytes = file_read(file, buffer, size);
        // lock_release(&filesys_io);               // LOCKS HAVE BEEN REMOVED
    } else {
        /* Can't read invalid file. */
        bytes = -1;
    }

    return bytes;
}


/* Writes size bytes from buffer to the open file fd. Returns the number of 
   bytes actually written, which may be less than size if some bytes could not 
   be written. */
static int sup_write(int fd, void* buffer, unsigned size, unsigned offset) {
    int bytes;

    /* Return number of bytes read. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        // lock_acquire(&filesys_io);                // LOCKS HAVE BEEN REMOVED
        file_seek(file, offset);  
        bytes = file_write(file, buffer, size);
        // lock_release(&filesys_io);               // LOCKS HAVE BEEN REMOVED
    } else {
        /* Can't read invalid file. */
        bytes = -1;
    }

    return bytes;
}


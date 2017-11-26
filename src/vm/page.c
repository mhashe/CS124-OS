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

static inline struct sup_entry *sup_get_entry(void *vaddr, struct sup_entry ***sup_pagedir);

static inline void sup_set_entry(void *vaddr, struct sup_entry *** sup_pagedir, struct sup_entry *entry);

static inline void sup_remove_entry(void *upage, struct sup_entry *** 
    sup_pagedir);

/* Functions from syscall. TODO: how to not reimplement them here? */
static int filesize(int fd);
static struct file_des *file_from_fd(int fd);
static int read(int fd, void* buffer, unsigned size, unsigned offset);


/* Allocates and returns a pointer to an empty supplementary table. */
struct sup_entry *** sup_pagedir_create(void) {
    return (struct sup_entry ***) palloc_get_page(PAL_ASSERT | PAL_ZERO);
}


/* Allocates entire file in as many pages as needed in supplementary page 
table. The file is given by "int fd" and it is writable if "writeable". To 
be called in mmap. Returns 0 on success, -1 on failure. */
int sup_alloc_file(void * vaddr, int fd, bool writable) {
    /* Current thread's supplemental page directory. */
    struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;
    
    /* We allocate file at offset into the page provided by offset. */
    int offset = pg_ofs(vaddr);

    /* Calculate the number of pages required to allocate file. */
    int num_pages = (offset + filesize(fd)) / PGSIZE;
    num_pages += ((num_pages % PGSIZE) != 0);

    /* Start allocating page for file in supplemental table at page-align. */
    vaddr = pg_round_down(vaddr);

    /* Check if needed pages are available in the supplementary table. */
    for (int page = 0; page < num_pages; page++) {
        void* addr = (vaddr + (PGSIZE * page));
        struct sup_entry* spe = sup_get_entry(addr, sup_pagedir);

        // TODO: If this address is not a valid address for other reasons, return -1

        /* There is not enough free pages to allocate the file, so fail. */
        if (spe != NULL) {
            return -1;
        }
    }

    /* Add the needed pages to the supplemental table with correct file and 
    offset. */
    for (int page = 0; page < num_pages; page ++) {
        void *addr = (vaddr + (PGSIZE * page));
        struct sup_entry* spe = sup_get_entry(addr, sup_pagedir);

        spe = (struct sup_entry *) malloc(sizeof(struct sup_entry));
        spe->fd = fd;
        spe->file_ofs = (unsigned) (offset + (PGSIZE * page));
        spe->writable = writable;
        sup_set_entry(addr, sup_pagedir, spe);
    }

    return 0;
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
    void * kpage = ftov(frame_no);

    /* Load one page of the file at file_ofs into the frame. */
    if (read(spe->fd, kpage, (unsigned) PGSIZE, spe->file_ofs) == -1) {
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


/* Deallocate and remove file from supplementary page table. Return -1 if 
not successful. 0 if successful. */
int sup_remove_file(void *vaddr) {
    struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;

    void *upage = pg_round_down(vaddr);
    struct sup_entry* entry = sup_get_entry(upage, sup_pagedir);

    int file = entry->fd;
    int success = 0;

    // TODO: this assumes the same file has not been allocated multiple times 
    // by the same process!
    while (entry->fd == file) {
        if (entry->loaded) {
            free_frame(entry->frame_no);
        }
        sup_remove_entry(upage, sup_pagedir);
        
        upage += PGSIZE;
        // TODO: edge case: this is the last page in the entire directory
        
        entry = sup_get_entry(upage, sup_pagedir);
        if (entry == NULL) {
            success = -1;
            break;
        }
    }

    return success;
}

/* Free all allocated pages and entries in the supplementary page table. 
TODO: Add this to process exit! */
void sup_free_table(struct sup_entry ***sup_pagedir) {
    size_t pde_idx, pte_idx;
    char *vaddr;
    size_t last_pde = pd_no(ptov(0));

    // TODO: change this. not everything is allocated in the begenning. 
    // instead, only deallocate tables that are not null. within these tables,
    // only deallocate entries that are not null, as done below.
    for (size_t page = 0; page < init_ram_pages; page++) {
        vaddr = ptov(page * PGSIZE);
        pde_idx = pd_no(vaddr);
        pte_idx = pt_no(vaddr);

        if (last_pde != pde_idx) {
            palloc_free_page(sup_pagedir[last_pde]);
            last_pde = pde_idx;
        }

        struct sup_entry * entry = sup_pagedir[pde_idx][pte_idx];
        if (entry != NULL) {
            if (entry->loaded) {
                free_frame(entry->frame_no);
            }
            free(entry);
        }
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
static inline struct sup_entry *sup_get_entry(void *upage, struct sup_entry ***sup_pagedir) {
    uintptr_t pd = pd_no(upage);
    if (sup_pagedir[pd] == NULL) {
        return NULL;
    }
    return sup_pagedir[pd][pt_no(upage)];
}


/* Sets supplemental entry from sup_pagedir at upage to be entry. upage must 
be page-aligned. */
static inline void sup_set_entry(void *upage, struct sup_entry ***sup_pagedir, struct sup_entry *entry) {
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
static int read(int fd, void* buffer, unsigned size, unsigned offset) {
    int bytes;

    /* Verify arguments. */
    verify_pointer((uint32_t *) buffer);
    verify_pointer((uint32_t *) (buffer + size));

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

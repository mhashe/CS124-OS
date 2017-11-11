#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stdbool.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/thread.h"

#include "devices/shutdown.h" /* For halt. */
#include "filesys/filesys.h"  /* For filesys ops. */
#include "filesys/file.h"     /* For file ops. */
#include "threads/malloc.h"    /* For malloc. */
#include "filesys/off_t.h" /* For off_t. */
#include "filesys/file.h" /* For file_length, seek, tell. */
#include "threads/synch.h" /* For locks. */

//TODO : Close all fds in thread_exit.
//TODO : Possible remove once filecount goes to zero?

/* Handler function. */
static void syscall_handler(struct intr_frame *);

/* Helper functions. */
static uint32_t get_arg(struct intr_frame *f, int offset);
static struct file_des *file_from_fd(int fd);

/* Handlers for Project 4. */
static void     halt(struct intr_frame *f);
static void     exit(struct intr_frame *f);
static void     exec(struct intr_frame *f);
static void     wait(struct intr_frame *f);
static void   create(struct intr_frame *f);
static void   remove(struct intr_frame *f);
static void     open(struct intr_frame *f);
static void filesize(struct intr_frame *f);
static void     read(struct intr_frame *f);
static void    write(struct intr_frame *f);
static void     seek(struct intr_frame *f);
static void     tell(struct intr_frame *f);
static void    close(struct intr_frame *f);

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/*! Lock for performing filesystems operations. */
static struct lock filesys_io;

void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");

    lock_init(&filesys_io);
}

uint32_t* verify_pointer(uint32_t* p) {
    if (is_user_vaddr(p)) {
        /* Valid pointer, continue. */
        return p;
    } else {
        /* Invalid pointer, exit. */
        thread_exit();
    }
    // uint32_t* vp = (uint32_t*)pagedir_get_page(thread_current()->pagedir, p);
    // if (vp == NULL)
    //     return NULL;
}


static void syscall_handler(struct intr_frame *f) {
    uint32_t *stack = verify_pointer((uint32_t*)f->esp);
    // TODO Handle
    if (stack == NULL) 
        thread_exit();
    int syscall_num =  *(stack);
    // hex_dump(0, stack-128, 256, true);
    // printf("system call %d!\n", syscall_num);
    if (syscall_num != 9) {
        // printf("system call %d!\n", syscall_num);
    }
    
    switch(syscall_num) {
        case SYS_HALT :
            halt(f);
            break;

        case SYS_EXIT :
            exit(f);
            break;

        case SYS_EXEC :
            exec(f);
            break;

        case SYS_WAIT :
            wait(f);
            break;

        case SYS_CREATE :
            create(f);
            break;

        case SYS_REMOVE :
            remove(f);
            break;

        case SYS_OPEN :
            open(f);
            break;

        case SYS_FILESIZE :
            filesize(f);
            break;

        case SYS_READ :
            read(f);
            break;

        case SYS_WRITE :
            write(f);
            break;

        case SYS_SEEK :
            seek(f);
            break;

        case SYS_TELL :
            tell(f);
            break;

        case SYS_CLOSE :
            close(f);
            break;

        default :
            /* Invalid syscall. */
            thread_exit();
    }
}


static uint32_t get_arg(struct intr_frame *f, int offset) {
    /* We only handle syscalls with <= 3 arguments. */
    ASSERT(offset <= 3);
    ASSERT(offset >= 0);
    ASSERT(f);

    /* Obtain stack pointer. */
    uint32_t *stack = (uint32_t*)f->esp;
    // hex_dump(0, stack, 100, true);

    /* Move to offset. */
    return *(stack + offset);
}


static struct file_des *file_from_fd(int fd) {
    ASSERT(fd > STDOUT_FILENO);

    struct list_elem *e;
    struct list fds = thread_current()->fds;
    struct file_des* fd_s;

    for (e = list_begin (&fds); 
         e != list_end (&fds); e = list_next (e)) {
        fd_s = list_entry(e, struct file_des, elem);
        if (fd_s->fd == fd)
            return fd_s;
    }

    return NULL;
}


static void halt(struct intr_frame *f UNUSED) {
    /* Terminate Pintos. */
    shutdown_power_off();
}


static void exit(struct intr_frame *f) {
    /* Parse arguments. */
    int status = get_arg(f, 1);

    struct thread *parent = thread_get_from_tid(thread_current()->parent_tid);

    if (parent == NULL) {
        f->eax = status;
        thread_exit();
    }

    struct child *c = thread_get_child_elem(&parent->children, thread_current()->tid);

    ASSERT(c != NULL);

    c->exit_code = status;
    printf("%s: exit(%d)\n", thread_current()->name, status);

    /* Status code returned to kernel; TODO when writing wait. */
    f->eax = status;
    thread_exit();
}


static void exec(struct intr_frame *f) {
    //TODO
    /* Parse arguments. */
    const char* cmd_line = (const char*) get_arg(f, 1);

    /* Verify arguments. */
    verify_pointer((uint32_t *) cmd_line);
    

    f->eax = process_execute(cmd_line);

    process_wait(f->eax);

    // Temp
    // thread_exit();
}


static void wait(struct intr_frame *f) {
    /* Parse arguments. */
    pid_t pid = get_arg(f, 1);
    f->eax = process_wait(pid);
}


static void create(struct intr_frame *f) {
    /* Parse arguments, ensure valid pointer. */
    const char* file = (const char*) get_arg(f, 1);
    unsigned initial_size = get_arg(f,2);

    /* Verify arguments. */
    verify_pointer((uint32_t *) file);

    /* Create file, return boolean value. */
    f->eax = filesys_create(file, initial_size);
}


static void remove(struct intr_frame *f) {
    /* Parse arguments. */
    const char* file = (const char*) get_arg(f, 1);

    /* Verify arguments. */
    verify_pointer((uint32_t *) file);

    /* Remove file. */
    f->eax = filesys_remove(file);
}


static void open(struct intr_frame *f) {
    /* Parse arguments. */
    const char* file_name = (const char*) get_arg(f, 1);

    /* Count of file descriptors for error checking. */
    size_t count_start = list_size(&thread_current()->fds);

    if (file_name) {
        /* Try to open file. */
        lock_acquire(&filesys_io);
        struct file* file = filesys_open(file_name);
        lock_release(&filesys_io);

        if (file) {
            /* File opened! Create a file descriptor, add to list. */
            int fd = 2;

            /* If we already have fds, set fd = max(fds) + 1.
               This ensures unique file descriptors for a thread. */
            if (!list_empty(&thread_current()->fds)) {

                struct list_elem* last = list_rbegin(&thread_current()->fds);
                struct file_des* last_fd = list_entry(last, struct file_des, elem);
                fd = MAX(fd, last_fd->fd);
            }

            /* If either of these numbers are assigned, something went horribly wrong. */
            ASSERT(fd != STDIN_FILENO || fd != STDOUT_FILENO);

            /* Add new file descriptor object. */
            struct file_des* new_fd = malloc(sizeof(struct file_des));
            new_fd->fd = fd;
            new_fd->file = file;

            list_push_back(&thread_current()->fds, &new_fd->elem);

            /* Return file descriptor. */
            f->eax = fd;

            /* List should have increased in size. */
            ASSERT(list_size(&thread_current()->fds) > count_start);

        } else {
            /* Couldn't open file. */
            f->eax = -1;
        }
    } else {
        /* Invalid file name. */
        thread_exit();
    }

    /* List should have increased, or at least stayed the same size. */
    ASSERT(list_size(&thread_current()->fds) >= count_start);
}


static void filesize(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = get_arg(f, 1);

    /* Special cases. */
    if (fd == STDIN_FILENO || fd == STDOUT_FILENO) {
        thread_exit();
    }

    /* Return file size. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        lock_acquire(&filesys_io);
        f->eax = file_length(file);
        lock_release(&filesys_io);
    } else {
        /* Invalid file has no size. */
        thread_exit();
    }
}


static void read(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = get_arg(f, 1);
    void* buffer = (void *) get_arg(f, 2);
    unsigned size = get_arg(f, 3);

    /* Verify arguments. */
    verify_pointer((uint32_t *) buffer);

    /* Special cases. */
    if (fd == STDIN_FILENO) {
        // TODO
        f->eax = -1;
        return;
    } 
    if (fd == STDOUT_FILENO) {
        /* Can't read from stdout... */
        f->eax = -1;
        return;
    }

    /* Return number of bytes read. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        lock_acquire(&filesys_io);
        f->eax = file_read(file, buffer, size);
        lock_release(&filesys_io);
    } else {
        /* Can't read invalid file. */
        f->eax = -1;
    }
}


static void write(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = get_arg(f, 1);
    const void* buffer = (void *) get_arg(f, 2);
    uint32_t size = get_arg(f, 3);

    /* Verify arguments. */
    verify_pointer((uint32_t *) buffer);

    /* Special cases. */
    if (fd == STDIN_FILENO) {
        /* Can't write to stdin... */
        f-> eax = 0;
        return;
    }
    if (fd == STDOUT_FILENO) {
        /* Write to terminal. */
        // TODO - Break up larger buffers into multiple calls.
        putbuf(buffer, size);
        f->eax = size;
        return;
    }

    /* Return number of bytes written. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        lock_acquire(&filesys_io);
        f->eax = file_write(file, buffer, size);
        lock_release(&filesys_io);
    } else {
        /* Can't write to file. */
        f->eax = 0;
    }
}


static void seek(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = get_arg(f, 1);
    unsigned position = get_arg(f, 2);

    /* Special cases. */
    if (fd == STDIN_FILENO || fd == STDOUT_FILENO) {
        thread_exit();
    }

    /* Seek file. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        lock_acquire(&filesys_io);
        file_seek(file, position);
        lock_release(&filesys_io);
    } else {
        /* Can't seek invalid file. */
        thread_exit();
    }
}


static void tell(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = get_arg(f, 1);

    /* Special cases. */
    if (fd == STDIN_FILENO || fd == STDOUT_FILENO) {
        thread_exit();
    }

    /* Tell file. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        lock_acquire(&filesys_io);
        f->eax = file_tell(file);
        lock_release(&filesys_io);
    } else {
        /* Can't tell invalid file. */
        thread_exit();
    }
}


static void close(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = get_arg(f, 1);

    /* Special cases. */
    if (fd == STDIN_FILENO || fd == STDOUT_FILENO) {
        thread_exit();
    }

    /* Tell file. */
    struct file_des* file_des = file_from_fd(fd);
    if (file_des) {
        lock_acquire(&filesys_io);
        file_close(file_des->file);
        lock_release(&filesys_io);

        /* Free memory, remove from list. */
        list_remove(&file_des->elem);
        free(file_des);

    } else {
        /* Can't close invalid file. */
        thread_exit();
    }
}


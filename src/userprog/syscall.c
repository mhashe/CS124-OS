#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stdbool.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/thread.h"

#include "devices/shutdown.h" /* For halt. */

/* Handler function. */
static void syscall_handler(struct intr_frame *);

/* Helper functions. */
static void get_args(uint32_t arg1, uint32_t arg2, uint32_t arg3, 
    uint32_t num_args, struct intr_frame *f);

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


void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static uint32_t* verify_pointer(uint32_t* p) {
    if (!is_user_vaddr(p))
        return NULL;
    uint32_t* vp = (uint32_t*)pagedir_get_page(thread_current()->pagedir, p);
    if (vp == NULL)
        return NULL;
    return vp;

}


static void syscall_handler(struct intr_frame *f) {
    printf("system call!\n");
    uint32_t *caller_stack = verify_pointer((uint32_t*)f->esp);
    // TODO Handle
    if (caller_stack == NULL) 
        thread_exit();
    int syscall_num =  *(caller_stack);

    switch(syscall_num) {
        case SYS_HALT :
            halt(f);
        case SYS_EXIT :
            exit(f);
        case SYS_EXEC :
            exec(f);
        case SYS_WAIT :
            wait(f);
        case SYS_CREATE :
            create(f);
        case SYS_REMOVE :
            remove(f);
        case SYS_OPEN :
            open(f);
        case SYS_FILESIZE :
            filesize(f);
        case SYS_READ :
            read(f);
        case SYS_WRITE :
            write(f);
        case SYS_SEEK :
            seek(f);
        case SYS_TELL :
            tell(f);
        case SYS_CLOSE :
            close(f);
    }
}


static void get_args(uint32_t arg1, uint32_t arg2, uint32_t arg3, 
    uint32_t num_args, struct intr_frame *f) {

    /* We only handle syscalls with >= 0, <= 3 arguments. */
    ASSERT(num_args >= 0);
    ASSERT(num_args <= 4);
}


static void halt(struct intr_frame *f) {
    /* Terminate Pintos. */
    shutdown_power_off();
}


static void exit(struct intr_frame *f) {
    /* Status code set as first argument. */
    uint32_t status = ((uint32_t)f)+1;
    f->eax = status;
    thread_exit();
}


static void exec(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void wait(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void create(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void remove(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void open(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void filesize(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void read(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void write(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void seek(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void tell(struct intr_frame *f) {
    // Temp
    thread_exit();
}


static void close(struct intr_frame *f) {
    // Temp
    thread_exit();
}


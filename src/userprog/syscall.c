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

/* Handler function. */
static void syscall_handler(struct intr_frame *);

/* Helper functions. */
static uint32_t* get_arg(struct intr_frame *f, int offset);

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
    printf("system call1!\n");
    uint32_t *stack = verify_pointer((uint32_t*)f->esp);
    // TODO Handle
    if (stack == NULL) 
        thread_exit();
    int syscall_num =  *(stack);
    printf("%d!\n", syscall_num);

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


static uint32_t* get_arg(struct intr_frame *f, int offset) {
    /* We only handle syscalls with <= 3 arguments. */
    ASSERT(offset <= 3);
    ASSERT(offset >= 0);

    /* Obtain stack pointer. */
    uint32_t *stack = verify_pointer((uint32_t*)f->esp);

    /* Move to off set. */
    return stack + offset;
}


static void halt(struct intr_frame *f UNUSED) {
    /* Terminate Pintos. */
    shutdown_power_off();
}


static void exit(struct intr_frame *f) {
    /* Parse arguments. */
    int status = *(int *) get_arg(f, 1);

    /* Status code set as first argument. */
    f->eax = status;
    thread_exit();
}


static void exec(struct intr_frame *f) {
    /* Parse arguments. */
    const char* file = (const char*) get_arg(f, 1);
    

    f->eax = process_execute(file);

    process_wait(f->eax);

    // Temp
    thread_exit();
}


static void wait(struct intr_frame *f) {
    /* Parse arguments. */
    pid_t pid = *(pid_t *) get_arg(f, 1);

    // Temp
    (void)f;
    thread_exit();
}


static void create(struct intr_frame *f) {
    /* Parse arguments. */
    const char* file = (const char*) get_arg(f, 1);
    uint32_t initial_size = *(uint32_t *) get_arg(f,2);

    // Temp
    (void)f;
    thread_exit();
}


static void remove(struct intr_frame *f) {
    /* Parse arguments. */
    const char* file = (const char*) get_arg(f, 1);

    // Temp
    (void)f;
    thread_exit();
}


static void open(struct intr_frame *f) {
    /* Parse arguments. */
    const char* file = (const char*) get_arg(f, 1);
    // Temp
    (void)f;
    thread_exit();
}


static void filesize(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = *(int *) get_arg(f, 1);

    // Temp
    (void)f;
    thread_exit();
}


static void read(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = *(int *) get_arg(f, 1);
    void* buffer = (void *) get_arg(f, 2);
    uint32_t size = *(uint32_t *) get_arg(f,3);

    // Temp
    (void)f;
    thread_exit();
}


static void write(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = *(int *) get_arg(f, 1);
    const void* buffer = (void *) get_arg(f, 2);
    uint32_t size = *(uint32_t *) get_arg(f,3);

    // Temp
    (void)f;
    thread_exit();
}


static void seek(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = *(int *) get_arg(f, 1);
    uint32_t position = *(uint32_t *) get_arg(f, 2);

    // Temp
    (void)f;
    thread_exit();
}


static void tell(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = *(int *) get_arg(f, 1);

    // Temp
    (void)f;
    thread_exit();
}


static void close(struct intr_frame *f) {
    /* Parse arguments. */
    int fd = *(int *) get_arg(f, 1);

    // Temp
    (void)f;
    thread_exit();
}


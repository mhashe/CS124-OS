#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "devices/shutdown.h" /* For halt. */

static void syscall_handler(struct intr_frame *);

/* Handlers for Project 4. */
static void halt(struct intr_frame *f);
static void exit(struct intr_frame *f);
static void exec(struct intr_frame *f);
static void wait(struct intr_frame *f);
static void create(struct intr_frame *f);
static void remove(struct intr_frame *f);
static void open(struct intr_frame *f);
static void filesize(struct intr_frame *f);
static void read(struct intr_frame *f);
static void write(struct intr_frame *f);
static void seek(struct intr_frame *f);
static void tell(struct intr_frame *f);
static void close(struct intr_frame *f);

void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler(struct intr_frame *f) {
    printf("system call!\n");

    int syscall_num = SYS_HALT;

    switch(syscall_num) {
        case 'SYS_HALT' :
            halt(f);
        case 'SYS_EXIT' :
            exit(f);
        case 'SYS_EXEC' :
            exec(f);
        case 'SYS_WAIT' :
            wait(f);
        case 'SYS_CREATE' :
            create(f);
        case 'SYS_REMOVE' :
            remove(f);
        case 'SYS_OPEN' :
            open(f);
        case 'SYS_FILESIZE' :
            filesize(f);
        case 'SYS_READ' :
            read(f);
        case 'SYS_WRITE' :
            write(f);
        case 'SYS_SEEK' :
            seek(f);
        case 'SYS_TELL' :
            tell(f);
        case 'SYS_CLOSE' :
            close(f);
    }
}


static void halt(struct intr_frame *f) {
    /* Terminate Pintos. */
    shutdown_power_off();
}


static void exit(struct intr_frame *f) {
    // Temp
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


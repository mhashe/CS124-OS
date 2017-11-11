/*! \file thread.h
 *
 * Declarations for the kernel threading functionality in PintOS.
 */

#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <fixedp.h>
#include <stdint.h>

/*! States in a thread's life cycle. */
enum thread_status {
    THREAD_RUNNING,     /*!< Running thread. */
    THREAD_READY,       /*!< Not running but ready to run. */
    THREAD_BLOCKED,     /*!< Waiting for an event to trigger. */
    THREAD_DYING        /*!< About to be destroyed. */
};

/*! Thread identifier type.
    You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /*!< Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /*!< Lowest priority. */
#define PRI_DEFAULT 31                  /*!< Default priority. */
#define PRI_MAX 63                      /*!< Highest priority. */

/* Value of thread->ticks_until_wake when not asleep */
#define THREAD_AWAKE -1                     

/* Thread niceness values. */
#define NICE_MIN -20                    /*!< Lowest niceness. */
#define NICE_INIT 0                     /*!< Niceness of init thread. */
#define NICE_MAX 20                     /*!< Highest niceness. */

/* Multi-level feedback queue scheduling. */
/* Number of ticks between recalculations of priority. */
#define PRI_RECALC_PERIOD 4
/* Initial values of load_avg and recent_cpu in initial thread. */
#define LOAD_AVG_INIT 0
#define RECENT_CPU_INIT 0
/* Number of ticks in interval used for averaging load into load_avg. */
#define LOAD_AVG_PERIOD 6000
/* Coefficient of momentum in calculating load_avg. */
#define LOAD_AVG_MOMENTUM fixedp_divide( \
fixedp_from_int(LOAD_AVG_PERIOD - TIMER_FREQ), fixedp_from_int(LOAD_AVG_PERIOD))
/* Coefficient of decay in calculating load_avg. */
#define LOAD_AVG_DECAY    \
  fixedp_divide(fixedp_from_int(TIMER_FREQ), fixedp_from_int(LOAD_AVG_PERIOD))

/*! File descriptor. */
struct file_des {
    int fd;                /*!< Numeric file descriptor. */
    struct file* file;     /*!< File struct pointer. */

    struct list_elem elem; /*!< Pointers to previous, next file descriptors. */
};

/*! Child of a thread. */
struct child {
    tid_t tid;                /*!< tid of child. */
    int exit_code;            /*!< Exit code. */

    struct list_elem elem;    /*!< Pointers to previous, next child. */
};

/*! A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

\verbatim
        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
             |               tid               |
        0 kB +---------------------------------+
\endverbatim

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion.

   The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list.
*/
struct thread {
    /*! Owned by thread.c. */
    /**@{*/
    tid_t tid;                          /*!< Thread identifier. */
    enum thread_status status;          /*!< Thread state. */
    char name[16];                      /*!< Name (for debugging purposes). */
    uint8_t *stack;                     /*!< Saved stack pointer. */
    int priority;                       /*!< Priority. */
    struct list_elem allelem;           /*!< List elem for all threads list. */
    /**@}*/

    /* User-added stuff. */
    int64_t ticks_until_wake;           /*!< Ticks until done sleeping. */
    int nice;                           /*!< Nice value. */
    fixedp recent_cpu;                  /*!< Recent cpu time. */

    int priority_org;                   /*!< Stores original priority when
                                             when elevated. */
    struct list locks;                  /*!< Locks which thread owns. */
    struct lock *blocked_lock;          /*!< Lock which thread wants. */
    
    tid_t parent_tid;                   /*!< Parent thread id. */
    bool parent_waiting;                /*!< Parent's is waiting. */
    struct list children;               /*!< List of children. */
    int exit_code;                      /*!< Exit code. */

    /*! Shared between thread.c and synch.c. */
    /**@{*/
    struct list_elem elem;              /*!< List element. */
    /**@}*/

#ifdef USERPROG
    /*! Owned by userprog/process.c. */
    /**@{*/
    uint32_t *pagedir;                  /*!< Page directory. */
    struct list fds;                    /*!< File descriptors. */
    struct file *binary;                /*!< File thread was started from. */
    /**@{*/
#endif

    /*! Owned by thread.c. */
    /**@{*/
    unsigned magic;                     /* Detects stack overflow. */
    /**@}*/
};

/*! If false (default), use round-robin scheduler.
    If true, use multi-level feedback queue scheduler.
    Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init(void);
void thread_start(void);

void thread_tick(void);
void thread_print_stats(void);

typedef void thread_func(void *aux);
tid_t thread_create(const char *name, int priority, thread_func *, void *);

void thread_block(void);
void thread_unblock(struct thread *);

struct thread *thread_current (void);
tid_t thread_tid(void);
const char *thread_name(void);

void thread_exit(void) NO_RETURN;
void thread_yield(void);
void release_all_resources(void);

/*! Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func(struct thread *t, void *aux);

void thread_foreach(thread_action_func *, void *);

int thread_get_priority(void);
void thread_set_priority(int);

int thread_get_nice(void);
void thread_set_nice(int);
int thread_get_recent_cpu(void);
int thread_get_load_avg(void);

void sort_ready_list(void);

void print_ready_queue(void);
void print_all_priorities(void);

void recalculate_priority(struct thread *t);

bool thread_queue_compare(const struct list_elem *a,
                             const struct list_elem *b,
                             void *aux UNUSED);

void thread_defer_to_max_priority(void);

struct thread *thread_get_from_tid(tid_t tid);
struct child *thread_get_child_elem(struct list *lst, tid_t child_tid);
void thread_remove_child_elem(struct list *lst, tid_t child_tid);

#endif /* threads/thread.h */


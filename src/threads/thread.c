#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "../devices/timer.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/*! Random value for struct thread's `magic' member.
    Used to detect stack overflow.  See the big comment at the top
    of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/*! List of processes in THREAD_READY state, that is, processes
    that are ready to run but not actually running. */
static struct list ready_list;

/*! List of all processes.  Processes are added to this list
    when they are first scheduled and removed when they exit. */
static struct list all_list;

/*! Idle thread. */
static struct thread *idle_thread;

/*! Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/*! Lock used by allocate_tid(). */
static struct lock tid_lock;

/*! Stack frame for kernel_thread(). */
struct kernel_thread_frame {
    void *eip;                  /*!< Return address. */
    thread_func *function;      /*!< Function to call. */
    void *aux;                  /*!< Auxiliary data for function. */
};

/* Statistics. */
static long long idle_ticks;    /*!< # of timer ticks spent idle. */
static long long kernel_ticks;  /*!< # of timer ticks in kernel threads. */
static long long user_ticks;    /*!< # of timer ticks in user programs. */

/* Scheduling. */
#define TIME_SLICE 4            /*!< # of timer ticks to give each thread. */
static unsigned thread_ticks;   /*!< # of timer ticks since last yield. */

/* Multi-level feedback queue scheduling */
static fixedp load_avg;

/*! If false (default), use round-robin scheduler.
    If true, use multi-level feedback queue scheduler.
    Controlled by kernel command-line option "-o mlfqs". */
bool thread_mlfqs;

static void kernel_thread(thread_func *, void *aux);

static void idle(void *aux UNUSED);
static struct thread *running_thread(void);
static struct thread *next_thread_to_run(void);
static void init_thread(struct thread *, const char *name, int priority);
static bool is_thread(struct thread *) UNUSED;
static void *alloc_frame(struct thread *, size_t size);
static void schedule(void);
void thread_schedule_tail(struct thread *prev);
static tid_t allocate_tid(void);

bool thread_queue_compare(const struct list_elem *a,
                             const struct list_elem *b,
                             void *aux UNUSED);
void print_run_queue(void);
void print_all_priorities(void);
static struct thread * thread_get_ready_front(void);
static void thread_wake(struct thread *t, void *aux UNUSED);
static void thread_defer_to_max_priority(int old_priority);

/* Multi-level feedback queue scheduling */
static void thread_update_mlfqs_state(void);
static void thread_update_priority_in_mlfqs(struct thread *t, void *aux UNUSED);
static void thread_update_recent_cpu(struct thread *t, void *aux UNUSED);
static int thread_get_num_ready_and_run(void);


void print_run_queue(void) {
    struct list_elem *e;
    printf("READY_QUEUE: \n");
    for (e = list_begin (&ready_list); e != list_end (&ready_list); e = list_next (e)) {
        struct thread *t = list_entry(e, struct thread, elem);
        printf("%s(%d) ", t->name, t->priority);
    }
    printf("\n");
}

void print_all_priorities(void) {
    struct list_elem *e;
    for (e = list_begin (&ready_list); e != list_end (&ready_list); e = list_next (e)) {
        struct thread *t = list_entry(e, struct thread, elem);
        printf("%d ", t->priority);
    }
    printf("\n");
}

/* Compares the value of two list elements A and B, given auxiliary data AUX.
   Returns true if A is greater than B, or false if A is less than or equal to
   B. */
bool thread_queue_compare(const struct list_elem *a,
                             const struct list_elem *b,
                             void *aux UNUSED) {    
    struct thread *ta = list_entry(a, struct thread, elem);
    struct thread *tb = list_entry(b, struct thread, elem);
    return ta->priority > tb->priority;
}


/* Inserts a thread elem into a list of threads (ie. ready_list) in order 
 or priority such that front is the highest priority. */
inline void thread_insert_ordered(struct list *lst, struct list_elem *elem) {
    // printf("INSERT %s\n", list_entry(elem, struct thread, elem)->name);
    list_insert_ordered (lst, elem,
                         (list_less_func*) thread_queue_compare, NULL);
    // printf("!INSERT\n");
}

void sort_ready_list(void) {
    list_sort(&ready_list, (list_less_func*) thread_queue_compare, NULL);
}

/* Returns the thread at the front of the ready queue without popping it. */
static inline struct thread * thread_get_ready_front(void) {
    return list_entry(list_front(&ready_list), struct thread, elem);
}

/*! Recalculates priority of thread. */
void recalculate_priority(struct thread *t) {
    ASSERT(is_thread(t));
    struct list_elem *e, *f;

    int max = t->priority_org;

    for (e = list_begin(&(t->locks)); e != list_end(&(t->locks)); e = list_next(e)) {

        struct lock *l = list_entry(e, struct lock, elem);
        for (f = list_begin(&(l->semaphore.waiters)); 
             f != list_end(&(l->semaphore.waiters)); f = list_next(f)) {

            struct thread *t_wait = list_entry(f, struct thread, elem);

            max = MAX(max, t_wait->priority);

        }
    }
    t->priority = max;

    if (t->blocked_lock) {
        recalculate_priority(t->blocked_lock->holder);
    }

    ASSERT(max >= t->priority_org);

}

/*! Initializes the threading system by transforming the code
    that's currently running into a thread.  This can't work in
    general and it is possible in this case only because loader.S
    was careful to put the bottom of the stack at a page boundary.

    Also initializes the run queue and the tid lock.

    After calling this function, be sure to initialize the page allocator
    before trying to create any threads with thread_create().

    It is not safe to call thread_current() until this function finishes. */
void thread_init(void) {
    printf("PROG_START. mlfqs: %d\n", thread_mlfqs);
    ASSERT(intr_get_level() == INTR_OFF);

    lock_init(&tid_lock);
    list_init(&ready_list);
    list_init(&all_list);

    /* Set constants for mflq scheduler. load_avg is initially zero. */
    load_avg = fixedp_from_int(LOAD_AVG_INIT);

    /* Set up a thread structure for the running thread. */
    initial_thread = running_thread();
    init_thread(initial_thread, "main", PRI_DEFAULT);
    initial_thread->status = THREAD_RUNNING;
    initial_thread->tid = allocate_tid();

    /* The initial thread has a nice and recent_cpu values of zero. */
    if (thread_mlfqs) {
        initial_thread->nice = NICE_INIT;
        initial_thread->recent_cpu = fixedp_from_int(RECENT_CPU_INIT);
        thread_update_priority_in_mlfqs(initial_thread, NULL);
    }
}

/*! Starts preemptive thread scheduling by enabling interrupts.
    Also creates the idle thread. */
void thread_start(void) {
    // printf("START\n");
    /* Create the idle thread. */
    struct semaphore idle_started;
    sema_init(&idle_started, 0);
    thread_create("idle", PRI_MIN, idle, &idle_started);

    /* Start preemptive thread scheduling. */
    intr_enable();

    /* Wait for the idle thread to initialize idle_thread. */
    sema_down(&idle_started);
}

/*! Called by the timer interrupt handler at each timer tick.
    Thus, this function runs in an external interrupt context. */
void thread_tick(void) {
    // printf("TICK\n");
    struct thread *t = thread_current();

    /* Update statistics. */
    if (t == idle_thread)
        idle_ticks++;
#ifdef USERPROG
    else if (t->pagedir != NULL)
        user_ticks++;
#endif
    else
        kernel_ticks++;

    /* Update all priorities in mlfq mode here and prevent this from being 
    interrupted by disabling interrupts (include tick increment with priority 
    update so a kernel never receives ticks that do not match priority). */
    enum intr_level old_level;
    old_level = intr_disable();

    /* Enforce preemption. */ // TODO: Should we avoid a high priority thread 
    // from facing the scheduler unnecessarily?
    if (++thread_ticks >= TIME_SLICE)
        intr_yield_on_return();

    /* Recalculate (priority, recent_cpu, and load_avg) used in mlfqs mode. */
    if (thread_mlfqs) {
        thread_update_mlfqs_state();
    }

    /* Add threads to ready queue that have been blocked and are due to be 
    woken up. */
    thread_foreach((thread_action_func *) &thread_wake, NULL);

    intr_set_level(old_level);


    /* If any of newly ready threads have higher priority than current thread,
       run once this interrupt completes. */
    if (!list_empty(&ready_list)) {
        if (thread_get_ready_front()->priority > thread_current()->priority) {
            intr_yield_on_return();
        }
    }
}

static void thread_update_mlfqs_state(void) {
    /* Increment recent_cpu by 1 for running thread every interrupt. */
    if (thread_current() != idle_thread) {
        thread_current()->recent_cpu = fixedp_add_with_int(
            thread_current()->recent_cpu, 1);
    }

    /* Update load_avg and recent_cpu once per second. */
    if (timer_ticks() % TIMER_FREQ == 0) {
        // Calculate load average
        load_avg = fixedp_multiply(LOAD_AVG_MOMENTUM, load_avg);
        load_avg = fixedp_add(load_avg,
            fixedp_multiply_with_int(LOAD_AVG_DECAY, 
                thread_get_num_ready_and_run()));

        // Calculate recent cpu for all threads
        thread_foreach((thread_action_func *) &thread_update_recent_cpu, NULL);

        // DEBUG/PRINT:
        // printf("Load: %d\n", thread_get_load_avg());
        // printf("All priorities: "); print_all_priorities();
    }

    /* Update priority for all threads every four timer ticks. */
    if (timer_ticks() % PRI_RECALC_PERIOD == 0) {
        thread_foreach((thread_action_func *) 
            &thread_update_priority_in_mlfqs, NULL);
    }
}

/* If thread is blocked, checks if it has a timer interrupt. If it does, it 
decrements it. If it is due, it unblocks the thread. */
static void thread_wake(struct thread *t, void *aux UNUSED) {
    /* Check if the thread has a timer interrupt. */
    if (t->ticks_until_wake == THREAD_AWAKE) {
        return;
    }
    
    /* A thread should only have a time until awake if it is  asleep/blocked. */
    ASSERT(t->status == THREAD_BLOCKED);

    /* wake_thread() is called every tick, so decrement time until wakeup. */
    t->ticks_until_wake--;

    /* Wake up if it is time to wakeup (sleep time left is 0) */
    if (t->ticks_until_wake <= 0) {
        t->ticks_until_wake = THREAD_AWAKE;
        thread_unblock(t);
    }
}

/*! Prints thread statistics. */
void thread_print_stats(void) {
    printf("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
           idle_ticks, kernel_ticks, user_ticks);
}

/*! Creates a new kernel thread named NAME with the given initial PRIORITY,
    which executes FUNCTION passing AUX as the argument, and adds it to the
    ready queue.  Returns the thread identifier for the new thread, or
    TID_ERROR if creation fails.

    If thread_start() has been called, then the new thread may be scheduled
    before thread_create() returns.  It could even exit before thread_create()
    returns.  Contrariwise, the original thread may run for any amount of time
    before the new thread is scheduled.  Use a semaphore or some other form of
    synchronization if you need to ensure ordering.

    The code provided sets the new thread's `priority' member to PRIORITY, but
    no actual priority scheduling is implemented.  Priority scheduling is the
    goal of Problem 1-3. */
tid_t thread_create(const char *name, int priority, thread_func *function,
                    void *aux) {
    // printf("CREATE\n");
    struct thread *t;
    struct kernel_thread_frame *kf;
    struct switch_entry_frame *ef;
    struct switch_threads_frame *sf;
    tid_t tid;

    ASSERT(function != NULL);

    /* Allocate thread. */
    t = palloc_get_page(PAL_ZERO);
    if (t == NULL)
        return TID_ERROR;

    /* Initialize thread. */
    init_thread(t, name, priority);
    tid = t->tid = allocate_tid();

    /* A threaded created (outside of the init thread) inherits its nice and 
    recent_cpu values from its parent, and sets its priority from them. */
    if (thread_mlfqs) {
        t->nice = thread_get_nice();
        t->recent_cpu = thread_current()->recent_cpu;
        thread_update_priority_in_mlfqs(thread_current(), NULL);
    }

    /* Stack frame for kernel_thread(). */
    kf = alloc_frame(t, sizeof *kf);
    kf->eip = NULL;
    kf->function = function;
    kf->aux = aux;

    /* Stack frame for switch_entry(). */
    ef = alloc_frame(t, sizeof *ef);
    ef->eip = (void (*) (void)) kernel_thread;

    /* Stack frame for switch_threads(). */
    sf = alloc_frame(t, sizeof *sf);
    sf->eip = switch_entry;
    sf->ebp = 0;

    /* Add to run queue. */
    thread_unblock(t);

    /* If new thread has higher priority than current thread, yield to it. */
    if (priority > thread_current()->priority) {
        thread_yield();
    }

    return tid;
}

/*! Puts the current thread to sleep.  It will not be scheduled
    again until awoken by thread_unblock().

    This function must be called with interrupts turned off.  It is usually a
    better idea to use one of the synchronization primitives in synch.h. */
void thread_block(void) {
    // printf("%s:BLOCK %s\n", thread_current()->name, thread_current()->name);
    ASSERT(!intr_context());
    ASSERT(intr_get_level() == INTR_OFF);

    thread_current()->status = THREAD_BLOCKED;
    schedule();
}

/*! Transitions a blocked thread T to the ready-to-run state.  This is an
    error if T is not blocked.

    This function does not preempt the running thread.  This can be important:
    if the caller had disabled interrupts itself, it may expect that it can
    atomically unblock a thread and update other data. */
void thread_unblock(struct thread *t) {
    // printf("%s:UNBLOCK %s\n", thread_current()->name, t->name);
    enum intr_level old_level;

    ASSERT(is_thread(t));

    old_level = intr_disable();
    ASSERT(t->status == THREAD_BLOCKED);
    thread_insert_ordered(&ready_list, &t->elem);
    t->status = THREAD_READY;

    intr_set_level(old_level);
    // print_run_queue();
    // printf("%d\n", thread_get_num_ready());
    // printf("!UNBLOCK\n");
}

/*! Returns the name of the running thread. */
const char * thread_name(void) {
    return thread_current()->name;
}

/*! Returns the running thread.
    This is running_thread() plus a couple of sanity checks.
    See the big comment at the top of thread.h for details. */
struct thread * thread_current(void) {
    struct thread *t = running_thread();

    /* Make sure T is really a thread.
       If either of these assertions fire, then your thread may
       have overflowed its stack.  Each thread has less than 4 kB
       of stack, so a few big automatic arrays or moderate
       recursion can cause stack overflow. */
    ASSERT(is_thread(t));
    ASSERT(t->status == THREAD_RUNNING);

    return t;
}

/*! Returns the running thread's tid. */
tid_t thread_tid(void) {
    return thread_current()->tid;
}

/*! Deschedules the current thread and destroys it.  Never
    returns to the caller. */
void thread_exit(void) {
    // printf("EXIT\n");
    ASSERT(!intr_context());

#ifdef USERPROG
    process_exit();
#endif

    /* Remove thread from all threads list, set our status to dying,
       and schedule another process.  That process will destroy us
       when it calls thread_schedule_tail(). */
    intr_disable();
    list_remove(&thread_current()->allelem);
    thread_current()->status = THREAD_DYING;
    schedule();
    NOT_REACHED();
}

/*! Yields the CPU.  The current thread is not put to sleep and
    may be scheduled again immediately at the scheduler's whim. */
void thread_yield(void) {
    // printf("YIELD\n");
    struct thread *cur = thread_current();
    enum intr_level old_level;

    ASSERT(!intr_context());

    old_level = intr_disable();
    if (cur != idle_thread) 
        thread_insert_ordered(&ready_list, &cur->elem);
    cur->status = THREAD_READY;
    schedule();
    intr_set_level(old_level);
    // printf("!YIELD\n");
}

/*! Invoke function 'func' on all threads, passing along 'aux'.
    This function must be called with interrupts off. */
void thread_foreach(thread_action_func *func, void *aux) {
    struct list_elem *e;

    ASSERT(intr_get_level() == INTR_OFF);

    for (e = list_begin(&all_list); e != list_end(&all_list);
         e = list_next(e)) {
        struct thread *t = list_entry(e, struct thread, allelem);
        func(t, aux);
    }
}

/*! Sets the current thread's priority to NEW_PRIORITY. */
void thread_set_priority(int new_priority) {
    /* A thread in the mlfqs mode cannot set its priority directly. */
    if (thread_mlfqs)
        return;
    
    // printf("%s:SET %d\n", thread_current()->name, new_priority);

    /* Make sure that new priority is a valid priority. */
    ASSERT(PRI_MIN <= new_priority && new_priority <= PRI_MAX);

    /* Disable interrupts to prevent race conditions. */
    enum intr_level old_level;
    old_level = intr_disable();

    /* Lower base priority, recalculate overall priority. */
    int old_priority = thread_current()->priority;
    thread_current()->priority_org = new_priority;
    recalculate_priority(thread_current());

    thread_defer_to_max_priority(old_priority);

    // TODO: if user raises priority, do we need to trigger donations?

    intr_set_level(old_level);
}

/* Yields the cpu if the new priority of the current thread is lowered and 
is now less than the priority at the fron the of ready queue. This should 
not be interrupted, and the calling function should take care of that. */
static void thread_defer_to_max_priority(int old_priority) {
    int new_priority = thread_current()->priority;

    /* If new_priority has less priority than old_priority, then check if 
    ready queue has a has a higher priority thread than it and yield if so. */
    if ((new_priority < old_priority) && !list_empty(&ready_list)) {
        if (thread_get_ready_front()->priority > new_priority) {
            // TODO: does disabling interrupts prevent intr_context() from being true?
            if (intr_context()) {
                intr_yield_on_return();
            } else {
                thread_yield();
            }
        }
    }
}

/*! Returns the current thread's priority. */
int thread_get_priority(void) {
    enum intr_level old_level = intr_disable ();
    int tmp = thread_current()->priority;
    intr_set_level (old_level);
    return tmp;
}

/*! Sets the current thread's nice value to NICE. */
void thread_set_nice(int nice) {
    /* A thread not in the mlfqs mode should not set niceness. */
    if (!thread_mlfqs)
        return;

    /* Make sure that new niceness is a valid value. */
    ASSERT(NICE_MIN <= nice && nice <= NICE_MAX);

    /* Disable interrupts to prevent race conditions. */
    enum intr_level old_level;
    old_level = intr_disable();

    /* Set new niceness. */
    thread_current()->nice = nice;

    /* Set thread priority according to nice value. */
    int old_priority = thread_current()->priority;
    thread_update_priority_in_mlfqs(thread_current(), NULL);

    /* If the running thread no long has the highest priority, yield. */
    thread_defer_to_max_priority(old_priority);

    intr_set_level(old_level);
}

/*! Returns the current thread's nice value. */
int thread_get_nice(void) {
    enum intr_level old_level = intr_disable ();
    int tmp = thread_current()->nice;
    intr_set_level (old_level);
    return tmp;
}

/* Sets the priority of the thread using the thread's nice and recent_cpu 
values. This should not be interrupted, and the calling function should take 
care of that. */
static void thread_update_priority_in_mlfqs(struct thread *t, void *aux UNUSED) {
    if (t == idle_thread)
        return;

    /* Set priority = PRI_MAX - (recent_cpu / 4) - (nice * 2) */
    
    // Ceil the penalty since we want priority to be "rounded down"
    int cpu_penalty = fixedp_to_int_floored(
        fixedp_divide_by_int(t->recent_cpu, 4)) + 1;

    t->priority = PRI_MAX - cpu_penalty - (t->nice * 2);

    /* Priority should be bounded by PRI_MIN and PRI_MAX. */
    if (t->priority < PRI_MIN)
        t->priority = PRI_MIN;
    else if (t->priority > PRI_MAX)
        t->priority = PRI_MAX;
}

/*! Returns 100 times the system load average. */
int thread_get_load_avg(void) {
    return fixedp_to_int_nearest(fixedp_multiply_with_int(
        load_avg, 100));
}

/* Returns the number of threads either running or ready but not including 
idle. This should not be interrupted, and the calling function should take 
care of that. */
int thread_get_num_ready_and_run(void) {
    int count = 0;

    struct list_elem *e;
    for (e = list_begin (&ready_list); e != list_end (&ready_list); e = list_next (e)) {
        count++;
    }

    if (thread_current() != idle_thread) {
        count++;
    }

    return count;
}

/* Updates the recent_cpu value for thread t. This should not be interrupted, 
and the calling function should take care of that. */
static void thread_update_recent_cpu(struct thread *t, void *aux UNUSED) {
    // recent_cpu = (2*load_avg)/(2*load_avg + 1) * recent_cpu + nice
    fixedp twice_load_avg = fixedp_multiply_with_int(load_avg, 2);
    t->recent_cpu = fixedp_multiply(t->recent_cpu,
            fixedp_divide(twice_load_avg, 
                fixedp_add_with_int(twice_load_avg, 1)
            )
        );
    t->recent_cpu = fixedp_add_with_int(t->recent_cpu, t->nice);
}

/*! Returns 100 times the current thread's recent_cpu value. */
int thread_get_recent_cpu(void) {
    return fixedp_to_int_nearest(fixedp_multiply_with_int(
        thread_current()->recent_cpu, 100));
}

/*! Idle thread.  Executes when no other thread is ready to run.

    The idle thread is initially put on the ready list by thread_start().
    It will be scheduled once initially, at which point it initializes
    idle_thread, "up"s the semaphore passed to it to enable thread_start()
    to continue, and immediately blocks.  After that, the idle thread never
    appears in the ready list.  It is returned by next_thread_to_run() as a
    special case when the ready list is empty. */
static void idle(void *idle_started_ UNUSED) {
    struct semaphore *idle_started = idle_started_;
    idle_thread = thread_current();
    sema_up(idle_started);

    for (;;) {
        /* Let someone else run. */
        intr_disable();
        thread_block();

        /* Re-enable interrupts and wait for the next one.

           The `sti' instruction disables interrupts until the completion of
           the next instruction, so these two instructions are executed
           atomically.  This atomicity is important; otherwise, an interrupt
           could be handled between re-enabling interrupts and waiting for the
           next one to occur, wasting as much as one clock tick worth of time.

           See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
           7.11.1 "HLT Instruction". */
        asm volatile ("sti; hlt" : : : "memory");
    }
}

/*! Function used as the basis for a kernel thread. */
static void kernel_thread(thread_func *function, void *aux) {
    ASSERT(function != NULL);

    intr_enable();       /* The scheduler runs with interrupts off. */
    function(aux);       /* Execute the thread function. */
    thread_exit();       /* If function() returns, kill the thread. */
}

/*! Returns the running thread. */
struct thread * running_thread(void) {
    uint32_t *esp;

    /* Copy the CPU's stack pointer into `esp', and then round that
       down to the start of a page.  Because `struct thread' is
       always at the beginning of a page and the stack pointer is
       somewhere in the middle, this locates the curent thread. */
    asm ("mov %%esp, %0" : "=g" (esp));
    return pg_round_down(esp);
}

/*! Returns true if T appears to point to a valid thread. */
static bool is_thread(struct thread *t) {
    return t != NULL && t->magic == THREAD_MAGIC;
}

/*! Does basic initialization of T as a blocked thread named NAME. */
static void init_thread(struct thread *t, const char *name, int priority) {
    // printf("INIT %s\n", name);
    enum intr_level old_level;

    ASSERT(t != NULL);
    ASSERT(PRI_MIN <= priority && priority <= PRI_MAX);
    ASSERT(name != NULL);

    memset(t, 0, sizeof *t);
    t->status = THREAD_BLOCKED;
    strlcpy(t->name, name, sizeof t->name);
    t->stack = (uint8_t *) t + PGSIZE;

    /* Ignore the priority argument when thread_mlfqs is true. */
    if (!thread_mlfqs) {
        t->priority = priority;
        t->priority_org = priority;
        list_init(&t->locks);
        t->blocked_lock = NULL;
    }

    /* Initially, a thread does not need to be woken up at some time. */
    t->ticks_until_wake = THREAD_AWAKE;

    t->magic = THREAD_MAGIC;

    old_level = intr_disable();
    list_push_back(&all_list, &t->allelem);

    // printf("TP: %d\n", t->priority);
    intr_set_level(old_level);
}

/*! Allocates a SIZE-byte frame at the top of thread T's stack and
    returns a pointer to the frame's base. */
static void * alloc_frame(struct thread *t, size_t size) {
    /* Stack data is always allocated in word-size units. */
    ASSERT(is_thread(t));
    ASSERT(size % sizeof(uint32_t) == 0);

    t->stack -= size;
    return t->stack;
}

/*! Chooses and returns the next thread to be scheduled.  Should return a
    thread from the run queue, unless the run queue is empty.  (If the running
    thread can continue running, then it will be in the run queue.)  If the
    run queue is empty, return idle_thread. */
static struct thread * next_thread_to_run(void) {
    if (list_empty(&ready_list))
      return idle_thread;
    else
      return list_entry(list_pop_front(&ready_list), struct thread, elem);
}

/*! Completes a thread switch by activating the new thread's page tables, and,
    if the previous thread is dying, destroying it.

    At this function's invocation, we just switched from thread PREV, the new
    thread is already running, and interrupts are still disabled.  This
    function is normally invoked by thread_schedule() as its final action
    before returning, but the first time a thread is scheduled it is called by
    switch_entry() (see switch.S).

   It's not safe to call printf() until the thread switch is complete.  In
   practice that means that printf()s should be added at the end of the
   function.

   After this function and its caller returns, the thread switch is complete. */
void thread_schedule_tail(struct thread *prev) {
    struct thread *cur = running_thread();
  
    ASSERT(intr_get_level() == INTR_OFF);

    /* Mark us as running. */
    cur->status = THREAD_RUNNING;

    /* Start new time slice. */
    thread_ticks = 0;

#ifdef USERPROG
    /* Activate the new address space. */
    process_activate();
#endif

    /* If the thread we switched from is dying, destroy its struct thread.
       This must happen late so that thread_exit() doesn't pull out the rug
       under itself.  (We don't free initial_thread because its memory was
       not obtained via palloc().) */
    if (prev != NULL && prev->status == THREAD_DYING &&
        prev != initial_thread) {
        ASSERT(prev != cur);
        palloc_free_page(prev);
    }
}

/*! Schedules a new process.  At entry, interrupts must be off and the running
    process's state must have been changed from running to some other state.
    This function finds another thread to run and switches to it.

    It's not safe to call printf() until thread_schedule_tail() has
    completed. */
static void schedule(void) {
    struct thread *cur = running_thread();
    struct thread *next = next_thread_to_run();
    struct thread *prev = NULL;

    ASSERT(intr_get_level() == INTR_OFF);
    ASSERT(cur->status != THREAD_RUNNING);
    ASSERT(is_thread(next));

    if (cur != next)
        prev = switch_threads(cur, next);
    thread_schedule_tail(prev);
}

/*! Returns a tid to use for a new thread. */
static tid_t allocate_tid(void) {
    static tid_t next_tid = 1;
    tid_t tid;

    lock_acquire(&tid_lock);
    tid = next_tid++;
    lock_release(&tid_lock);

    return tid;
}

/*! Offset of `stack' member within `struct thread'.
    Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof(struct thread, stack);


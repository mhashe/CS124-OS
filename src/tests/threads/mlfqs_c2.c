/* Ensures that a high-priority thread really preempts.

   Based on a test originally submitted for Stanford's CS 140 in
   winter 1999 by by Matt Franklin
   <startled@leland.stanford.edu>, Greg Hutchins
   <gmh@leland.stanford.edu>, Yu Ping Hu <yph@cs.stanford.edu>.
   Modified by arens. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

static thread_func simple_thread_func;

void
test_mlfqs_c2 (void) 
{
  /* Requires. */
  ASSERT (thread_mlfqs);

  /* Make sure our priority is the default. */
  thread_set_priority(PRI_MAX); /* Temporary. */

  msg ("Create threads.");
  thread_set_nice(0);
  thread_create ("A", PRI_MIN, simple_thread_func, NULL);

  thread_set_nice(1);
  thread_create ("B", PRI_MIN, simple_thread_func, NULL);

  thread_set_nice(2);
  thread_create ("C", PRI_MIN, simple_thread_func, NULL);

  thread_set_nice(0);
  msg ("Threads created.");
}

static void 
simple_thread_func (void *aux UNUSED) 
{
  msg ("Thread %s started!", thread_name ());

  /* Print stats, sleep until yield. */
  // for (int i = 0; i < 5; i++) {
  msg ("Thread %s computing!", thread_name ());
  print_run_queue();
  print_all_priorities();
  msg ("\n");

  timer_sleep (4);
  // }

  msg ("Thread %s finished!", thread_name ());

}

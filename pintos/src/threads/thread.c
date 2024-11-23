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
#include "devices/timer.h" // 추가

#ifdef USERPROG
#include "userprog/process.h"
#include "threads/synch.h"   // 세마포어 사용을 위해 추가
//#include "devices/timer.h" // 추가
#endif

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/* List of processes in THREAD_READY state, that is, processes
   that are ready to run but not actually running. */
static struct list ready_list;
struct list sleep_list;
/* List of all processes.  Processes are added to this list
   when they are first scheduled and removed when they exit. */
static struct list all_list;

/* Idle thread. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;

/* Stack frame for kernel_thread(). */
struct kernel_thread_frame 
  {
    void *eip;                  /* Return address. */
    thread_func *function;      /* Function to call. */
    void *aux;                  /* Auxiliary data for function. */
  };

// mlfq priority 계산 위한 변수 
static fixed_point_t load_avg;


/* Statistics. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
static long long user_ticks;    /* # of timer ticks in user programs. */

/* Scheduling. */
#define TIME_SLICE 4            /* # of timer ticks to give each thread. */
static unsigned thread_ticks;   /* # of timer ticks since last yield. */




// 플젝3 ---------------------
#ifndef USERPROG
// project3 
bool thread_prior_aging;
#endif


#ifndef USERPROG
void thread_aging(void);
#endif

// 플젝3 ----------------------

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
bool thread_mlfqs;
scheduling_mode_t current_scheduling_mode = SCHEDULING_PRIORITY;  /* 기본값: 우선순위 기반 */

// 계산 함수
fixed_point_t fixed_point_operation(fixed_point_op_t op, fixed_point_t A, int B, fixed_point_t C);

// 매크로
#define FP_ADD(A, C) fixed_point_operation(FP_ADD_OP, (A), 0, (C))
#define FP_SUB(A, C) fixed_point_operation(FP_SUB_OP, (A), 0, (C))
#define FP_MUL_INT(A, B) fixed_point_operation(FP_MUL_INT_OP, (A), (B), 0)
#define FP_DIV_INT(A, B) fixed_point_operation(FP_DIV_INT_OP, (A), (B), 0)
#define FP_MUL(A, C) fixed_point_operation(FP_MUL_OP, (A), 0, (C))
#define FP_DIV(A, C) fixed_point_operation(FP_DIV_OP, (A), 0, (C))
#define FP_ADD_INT(A, B) fixed_point_operation(FP_ADD_INT_OP, (A), (B), 0)
#define FP_SUB_INT(A, B) fixed_point_operation(FP_SUB_INT_OP, (A), (B), 0)
#define FP_INT_PART(A) fixed_point_operation(FP_INT_PART_OP, (A), 0, 0)
#define FP_ROUND(A) fixed_point_operation(FP_ROUND_OP, (A), 0, 0)
#define FP_CONST(x) fixed_point_operation(FP_CONST_OP, (x), 0, 0)
// 플젝3 새로 추가 프로토타입
bool cmp_wake_up_time(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);
bool cmp_priority(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);
// 플젝3-mlfq 추가 프로토타입
static void calculate_priority(struct thread *t, void *aux UNUSED);
static void calculate_recent_cpu(struct thread *t, void *aux UNUSED);
static void calculate_load_avg(void);

static void kernel_thread (thread_func *, void *aux);

static void idle (void *aux UNUSED);
static struct thread *running_thread (void);
static struct thread *next_thread_to_run (void);
static void init_thread (struct thread *, const char *name, int priority);
static bool is_thread (struct thread *) UNUSED;
static void *alloc_frame (struct thread *, size_t size);
static void schedule (void);
void thread_schedule_tail (struct thread *prev);
static tid_t allocate_tid (void);
// 새로 추가 
struct thread *get_thread_by_tid(tid_t tid);
void test_max_priority(void);
int thread_get_load_avg (void);
int thread_get_recent_cpu (void);
// 플젝3 ----------------------
#ifndef USERPROG
/* 에이징을 통해 ready_list에 있는 모든 스레드의 우선순위를 증가시킴. */
void thread_aging(void) {
    struct list_elem *e;
    struct thread *t;

    /* ready_list를 순회 ->  각 스레드의 우선순위를 증가. */
    for (e = list_begin(&ready_list); e != list_end(&ready_list); e = list_next(e)) {
        t = list_entry(e, struct thread, elem);

        /* 우선순위가 최대값을 넘지 않도록 함 */
        if (t->priority < PRI_MAX) {
            t->priority += 1;
        }
    }
}
#endif



// 새로 추가 함수
/* why? Pintos는 모든 스레드를 all_list라는 리스트로 관리하기 떄문에,
특정 TID를 가진 스레드를 찾으려면 이 리스트를 순회해야한다. 
process_execute(), process_wait(), exec(), wait() 등의 함수에서 자식 스레드를 찾을 때 사용함
input : tid_t tid  - 찾고자 하는 스레드의 식별자



해당 tid를 가진 스레드의 포인터를 반환
*/
struct thread *get_thread_by_tid(tid_t tid)
{
  struct list_elem *e;

  /* 모든 스레드를 포함하는 all_list를 순회 */
  for (e = list_begin(&all_list); e != list_end(&all_list); e = list_next(e))
  {
    struct thread *t = list_entry(e, struct thread, allelem);
    if (t->tid == tid)
      return t;  // 해당 tid를 가진 스레드를 찾으면 반환
  }
  return NULL;  // 찾지 못하면 NULL 반환
}


/* Initializes the threading system by transforming the code
   that's currently running into a thread.  This can't work in
   general and it is possible in this case only because loader.S
   was careful to put the bottom of the stack at a page boundary.

   Also initializes the run queue and the tid lock.

   After calling this function, be sure to initialize the page
   allocator before trying to create any threads with
   thread_create().

   It is not safe to call thread_current() until this function
   finishes. */


void
thread_init (void) 
{
  ASSERT (intr_get_level () == INTR_OFF);

  lock_init (&tid_lock);
  list_init (&ready_list);
  list_init (&all_list);
  // 플젝3 추가- 알람 클럭 관련
  list_init (&sleep_list);  
  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", PRI_DEFAULT);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();
}

/* Starts preemptive thread scheduling by enabling interrupts.
   Also creates the idle thread. */
void
thread_start (void) 
{
  /* Create the idle thread. */
  struct semaphore idle_started;
  sema_init (&idle_started, 0);
  thread_create ("idle", PRI_MIN, idle, &idle_started);

  /* Start preemptive thread scheduling. */
  intr_enable ();

  /* Wait for the idle thread to initialize idle_thread. */
  sema_down (&idle_started);

  load_avg = FP_CONST(0);
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void
thread_tick (void) 
{
  struct thread *t = thread_current ();

  /* Update statistics. */
  if (t == idle_thread)
    idle_ticks++;
#ifdef USERPROG
  else if (t->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;

  /* Enforce preemption. */
  if (++thread_ticks >= TIME_SLICE)
    intr_yield_on_return ();

  if(current_scheduling_mode == SCHEDULING_MLFQS){


        /* Increase recent_cpu by 1 for the running thread */
        if (t != idle_thread)
            t->recent_cpu = FP_ADD(t->recent_cpu, FP_CONST(1));

        /* Every second, update load_avg and recent_cpu for all threads */
        if (timer_ticks() % TIMER_FREQ == 0)
        {
            calculate_load_avg();
            thread_foreach(calculate_recent_cpu, NULL);
        }

        /* Every 4 ticks, update priority for all threads */
        if (timer_ticks() % TIME_SLICE == 0)
        {
            thread_foreach(calculate_priority, NULL);
            /* Re-sort ready_list based on new priorities */
            list_sort(&ready_list, cmp_priority, NULL);
        }
}


  //project3 
   #ifndef USERPROG
//	thread_wake_up();
	if(thread_prior_aging==true){thread_aging();}
   #endif
   }

/* Prints thread statistics. */
void
thread_print_stats (void) 
{
  printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
          idle_ticks, kernel_ticks, user_ticks);
}

/* Creates a new kernel thread named NAME with the given initial
   PRIORITY, which executes FUNCTION passing AUX as the argument,
   and adds it to the ready queue.  Returns the thread identifier
   for the new thread, or TID_ERROR if creation fails.

   If thread_start() has been called, then the new thread may be
   scheduled before thread_create() returns.  It could even exit
   before thread_create() returns.  Contrariwise, the original
   thread may run for any amount of time before the new thread is
   scheduled.  Use a semaphore or some other form of
   synchronization if you need to ensure ordering.

   The code provided sets the new thread's `priority' member to
   PRIORITY, but no actual priority scheduling is implemented.
   Priority scheduling is the goal of Problem 1-3. */
tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux) 
{
  struct thread *t;
  struct kernel_thread_frame *kf;
  struct switch_entry_frame *ef;
  struct switch_threads_frame *sf;
  tid_t tid;

  ASSERT (function != NULL);

  /* Allocate thread. */
  t = palloc_get_page (PAL_ZERO);
  if (t == NULL)
    return TID_ERROR;

  /* Initialize thread. */
 
  init_thread (t, name, priority);
  tid = t->tid = allocate_tid ();

 #ifdef USERPROG
    /* 스레드 생성 시 종료되지 않은 상태로 초기화 */
  t->exited = false;
   t->wrong_exit = false;
  #endif


  /* Stack frame for kernel_thread(). */
  kf = alloc_frame (t, sizeof *kf);
  kf->eip = NULL;
  kf->function = function;
  kf->aux = aux;

  /* Stack frame for switch_entry(). */
  ef = alloc_frame (t, sizeof *ef);
  ef->eip = (void (*) (void)) kernel_thread;

  /* Stack frame for switch_threads(). */
  sf = alloc_frame (t, sizeof *sf);
  sf->eip = switch_entry;
  sf->ebp = 0;

  /* Add to run queue. */
  thread_unblock (t);



  // 선점 여부 확인 - 즉, 현재 만들어진것보다, 새로 만들어진게 더 우선순위 높으면
  // 현재꺼 그만둘수 있게끔 한다!!
  if (t->priority > thread_current()->priority)
    thread_yield();

  return tid;
}

/* Puts the current thread to sleep.  It will not be scheduled
   again until awoken by thread_unblock().

   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void
thread_block (void) 
{
  ASSERT (!intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  thread_current ()->status = THREAD_BLOCKED;
  schedule ();
}

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)

   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
void
thread_unblock (struct thread *t) 
{
  enum intr_level old_level;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);
 // list_push_back (&ready_list, &t->elem);

  list_insert_ordered(&ready_list, &t->elem, cmp_priority, NULL);
  t->status = THREAD_READY;
  intr_set_level (old_level);
}

/* Returns the name of the running thread. */
const char *
thread_name (void) 
{
  return thread_current ()->name;
}

/* Returns the running thread.
   This is running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
struct thread *
thread_current (void) 
{
  struct thread *t = running_thread ();
  
  /* Make sure T is really a thread.
     If either of these assertions fire, then your thread may
     have overflowed its stack.  Each thread has less than 4 kB
     of stack, so a few big automatic arrays or moderate
     recursion can cause stack overflow. */
  ASSERT (is_thread (t));
  ASSERT (t->status == THREAD_RUNNING);

  return t;
}

/* Returns the running thread's tid. */
tid_t
thread_tid (void) 
{
  return thread_current ()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void
thread_exit (void) 
{
  ASSERT (!intr_context ());

#ifdef USERPROG
  process_exit ();
#endif

  /* Remove thread from all threads list, set our status to dying,
     and schedule another process.  That process will destroy us
     when it calls thread_schedule_tail(). */
  intr_disable ();
  list_remove (&thread_current()->allelem);
  thread_current ()->status = THREAD_DYING;
  schedule ();
  NOT_REACHED ();
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void
thread_yield (void) 
{
  struct thread *cur = thread_current ();
  enum intr_level old_level;
  
  ASSERT (!intr_context ());

  old_level = intr_disable ();
  if (cur != idle_thread) 
  //  list_push_back (&ready_list, &cur->elem);
   list_insert_ordered(&ready_list, &cur->elem, cmp_priority, NULL);

  cur->status = THREAD_READY;
  schedule ();
  intr_set_level (old_level);
}

/* Invoke function 'func' on all threads, passing along 'aux'.
   This function must be called with interrupts off. */
void
thread_foreach (thread_action_func *func, void *aux)
{
  struct list_elem *e;

  ASSERT (intr_get_level () == INTR_OFF);

  for (e = list_begin (&all_list); e != list_end (&all_list);
       e = list_next (e))
    {
      struct thread *t = list_entry (e, struct thread, allelem);
      func (t, aux);
    }
}

/* Sets the current thread's priority to NEW_PRIORITY. */
void
thread_set_priority (int new_priority) 
{
//  thread_current ()->priority = new_priority;
if (current_scheduling_mode == SCHEDULING_MLFQS)return;
// 어차피, mlfq 모드일때는 ,테스트 케이스에서 thread_set_pirority 함수 호출안하는듯 
  else if (current_scheduling_mode == SCHEDULING_PRIORITY)
  {

  enum intr_level old_level = intr_disable();
  int old_priority = thread_current()->priority;
  thread_current()->priority = new_priority;

  if (new_priority < old_priority)
    test_max_priority();

  intr_set_level (old_level);}
}

/* Returns the current thread's priority. */
int
thread_get_priority (void) 
{
  return thread_current ()->priority;
}

/* Sets the current thread's nice value to NICE. */
void
thread_set_nice (int nice UNUSED) 
{
  /* Not yet implemented.*/
enum intr_level old_level = intr_disable ();

  thread_current()->nice = nice;

  /* Recalculate priority */
  calculate_priority(thread_current(), NULL);

  /* Re-sort ready_list */
  list_sort(&ready_list, cmp_priority, NULL);

  /* Yield if necessary */
  test_max_priority();

  intr_set_level(old_level);

}

/* Returns the current thread's nice value. */
int
thread_get_nice (void) 
{

  /* Not yet implemented. */
  return thread_current()->nice;

//  return 0;
}

/* Returns 100 times the system load average. */
//int
//thread_get_load_avg (void) 
//{
  /* Not yet implemented. */
 // return 0;
//}

/* Returns 100 times the current thread's recent_cpu value. */
//int
//thread_get_recent_cpu (void) 
//{
  /* Not yet implemented. */
 // return 0;
//}

/* Idle thread.  Executes when no other thread is ready to run.

   The idle thread is initially put on the ready list by
   thread_start().  It will be scheduled once initially, at which
   point it initializes idle_thread, "up"s the semaphore passed
   to it to enable thread_start() to continue, and immediately
   blocks.  After that, the idle thread never appears in the
   ready list.  It is returned by next_thread_to_run() as a
   special case when the ready list is empty. */
static void
idle (void *idle_started_ UNUSED) 
{
  struct semaphore *idle_started = idle_started_;
  idle_thread = thread_current ();
  sema_up (idle_started);

  for (;;) 
    {
      /* Let someone else run. */
      intr_disable ();
      thread_block ();

      /* Re-enable interrupts and wait for the next one.

         The `sti' instruction disables interrupts until the
         completion of the next instruction, so these two
         instructions are executed atomically.  This atomicity is
         important; otherwise, an interrupt could be handled
         between re-enabling interrupts and waiting for the next
         one to occur, wasting as much as one clock tick worth of
         time.

         See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
         7.11.1 "HLT Instruction". */
      asm volatile ("sti; hlt" : : : "memory");
    }
}

/* Function used as the basis for a kernel thread. */
static void
kernel_thread (thread_func *function, void *aux) 
{
  ASSERT (function != NULL);

  intr_enable ();       /* The scheduler runs with interrupts off. */
  function (aux);       /* Execute the thread function. */
  thread_exit ();       /* If function() returns, kill the thread. */
}

/* Returns the running thread. */
struct thread *
running_thread (void) 
{
  uint32_t *esp;

  /* Copy the CPU's stack pointer into `esp', and then round that
     down to the start of a page.  Because `struct thread' is
     always at the beginning of a page and the stack pointer is
     somewhere in the middle, this locates the curent thread. */
  asm ("mov %%esp, %0" : "=g" (esp));
  return pg_round_down (esp);
}

/* Returns true if T appears to point to a valid thread. */
static bool
is_thread (struct thread *t)
{
  return t != NULL && t->magic == THREAD_MAGIC;
}

/* Does basic initialization of T as a blocked thread named
   NAME. */
static void
init_thread (struct thread *t, const char *name, int priority)
{
  enum intr_level old_level;

  ASSERT (t != NULL);
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);

  memset (t, 0, sizeof *t);
  t->status = THREAD_BLOCKED;
  strlcpy (t->name, name, sizeof t->name);
  t->stack = (uint8_t *) t + PGSIZE;

     if(current_scheduling_mode == SCHEDULING_PRIORITY){
        t->priority = priority;
      }
      else if(current_scheduling_mode == SCHEDULING_MLFQS){
 if (t == initial_thread)
        {
            t->nice = 0;
            t->recent_cpu = FP_CONST(0);
        }
        else
        {
            t->nice = thread_current()->nice;
            t->recent_cpu = thread_current()->recent_cpu;
        }
        calculate_priority(t, NULL);         
   
  }

  t->magic = THREAD_MAGIC;

  old_level = intr_disable ();
  list_push_back (&all_list, &t->allelem);
  intr_set_level (old_level);

#ifdef USERPROG
// 자식 스레드 생성시, 부모-자식간 동기화 위한 세마포어 초기화 
  sema_init(&t->load_sema, 0);
  sema_init(&t->exit_sema, 0);
  sema_init(&t->wait_sema, 0);  // 부모가 자식을 정리하는 동안 기다리는 세마포어


  // 자식 스레드 리스트 초기화 
  list_init(&t->child_list);

 if (t != initial_thread) {
    /* 현재 스레드를 자식 스레드의 부모로 설정 */
    t->parent = thread_current();
    /* 부모 스레드의 자식 리스트에 자신을 추가 */
    list_push_back(&t->parent->child_list, &t->child_elem);
  } else {
    /* initial_thread의 경우 부모가 없으므로 NULL로 설정 */
    t->parent = NULL;
  }

    t->reserved_entry[0] = 0; // stdin
    t->reserved_entry[1] = 1; // stdout
    /* 파일 디스크립터 테이블 초기화 */
     t->next_fd = (sizeof(t->reserved_entry) / sizeof(t->reserved_entry[0])); // 예약된 항목의 개수로 초기화

  for (int i = 0; i < FDT_MAX; i++) {
        t->fdt[i] = NULL; // 모든 파일 디스크립터를 NULL로 초기화
    }

    
    t->suppress_exit_msg = false;  // 플래그 초기화

  t->exec_file = NULL;  // 실행 파일에 대한 파일 포인터 초기화

#endif

}

/* Allocates a SIZE-byte frame at the top of thread T's stack and
   returns a pointer to the frame's base. */
static void *
alloc_frame (struct thread *t, size_t size) 
{
  /* Stack data is always allocated in word-size units. */
  ASSERT (is_thread (t));
  ASSERT (size % sizeof (uint32_t) == 0);

  t->stack -= size;
  return t->stack;
}

/* Chooses and returns the next thread to be scheduled.  Should
   return a thread from the run queue, unless the run queue is
   empty.  (If the running thread can continue running, then it
   will be in the run queue.)  If the run queue is empty, return
   idle_thread. */
static struct thread *
next_thread_to_run (void) 
{
  if (list_empty (&ready_list))
    return idle_thread;
  else
    return list_entry (list_pop_front (&ready_list), struct thread, elem);
}

/* Completes a thread switch by activating the new thread's page
   tables, and, if the previous thread is dying, destroying it.

   At this function's invocation, we just switched from thread
   PREV, the new thread is already running, and interrupts are
   still disabled.  This function is normally invoked by
   thread_schedule() as its final action before returning, but
   the first time a thread is scheduled it is called by
   switch_entry() (see switch.S).

   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf()s should be
   added at the end of the function.

   After this function and its caller returns, the thread switch
   is complete. */
void
thread_schedule_tail (struct thread *prev)
{
  struct thread *cur = running_thread ();
  
  ASSERT (intr_get_level () == INTR_OFF);

  /* Mark us as running. */
  cur->status = THREAD_RUNNING;

  /* Start new time slice. */
  thread_ticks = 0;

#ifdef USERPROG
  /* Activate the new address space. */
  process_activate ();
#endif

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc().) */
  if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread) 
    {
      ASSERT (prev != cur);
      palloc_free_page (prev);
    }
}

/* Schedules a new process.  At entry, interrupts must be off and
   the running process's state must have been changed from
   running to some other state.  This function finds another
   thread to run and switches to it.

   It's not safe to call printf() until thread_schedule_tail()
   has completed. */
static void
schedule (void) 
{
  struct thread *cur = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;

  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (cur != next)
    prev = switch_threads (cur, next);
  thread_schedule_tail (prev);
}

/* Returns a tid to use for a new thread. */
static tid_t
allocate_tid (void) 
{
  static tid_t next_tid = 1;
  tid_t tid;

  lock_acquire (&tid_lock);
  tid = next_tid++;
  lock_release (&tid_lock);

  return tid;
}


// 프로젝트3을위한 각각의 함수들을 구현한 부분입니다!

// 1. 
// sleep list에서 더 일찍 깨어나야 하는 thread 순으로 정렬하기 위한 함수
bool
cmp_wake_up_time(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED)
{
  struct thread *t_a = list_entry(a, struct thread, elem);
  struct thread *t_b = list_entry(b, struct thread, elem);
  // a의 wake_up_time이 더 작으면 true
  if(t_a->wake_up_time < t_b->wake_up_time)
    return true;
  else
    return false;
}


// 우선순위 비교함수

bool cmp_priority(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED)
{
  struct thread *t_a = list_entry(a, struct thread, elem);
  struct thread *t_b = list_entry(b, struct thread, elem);
if(t_a->priority > t_b->priority){
    // 우선순위는 큰게 더 높은 것임 
    return true;

}else{
    return false;
}
}


/* Put the current thread to sleep until ticks */
void
thread_sleep(int64_t ticks)
{
  struct thread *cur = thread_current();
  enum intr_level old_level;

  ASSERT(!intr_context());
  ASSERT(intr_get_level() == INTR_ON);

  if (ticks <= 0)
    return;

  old_level = intr_disable();
  cur->wake_up_time = ticks;
  list_insert_ordered(&sleep_list, &cur->elem, cmp_wake_up_time, NULL);
  thread_block();
  intr_set_level(old_level);
}



// 3- mlfq를 위한 구현


/* Fixed-point operation function */
fixed_point_t
fixed_point_operation(fixed_point_op_t op, fixed_point_t A, int B, fixed_point_t C)
{
  switch (op)
    {
    case FP_ADD_OP:
      return A + C;

    case FP_SUB_OP:
      return A - C;

    case FP_MUL_INT_OP:
      return A * B;

    case FP_DIV_INT_OP:
      return A / B;

    case FP_MUL_OP:
      return ((fixed_point_t)(((int64_t) A) * C >> FP_SHIFT_AMOUNT));

    case FP_DIV_OP:
      return ((fixed_point_t)((((int64_t) A) << FP_SHIFT_AMOUNT) / C));

    case FP_ADD_INT_OP:
      return A + (B << FP_SHIFT_AMOUNT);

    case FP_SUB_INT_OP:
      return A - (B << FP_SHIFT_AMOUNT);

    case FP_INT_PART_OP:
      return A >> FP_SHIFT_AMOUNT;

    case FP_ROUND_OP:
      return (A >= 0 ? ((A + (1 << (FP_SHIFT_AMOUNT - 1))) >> FP_SHIFT_AMOUNT)
                    : ((A - (1 << (FP_SHIFT_AMOUNT - 1))) >> FP_SHIFT_AMOUNT));

    case FP_CONST_OP:
        return (A << FP_SHIFT_AMOUNT);
    default:
    // 존재하지 않는 연산
        return 0;
    }
}

void
thread_awake(int64_t current_ticks)
{
    /* 계속해서 sleep_list의 첫 번째 요소를 확인 */
    while (!list_empty(&sleep_list)) {
        struct thread *t = list_entry(list_front(&sleep_list), struct thread, elem);
        
        /* 현재 틱보다 작거나 같으면 해제 */
        if (t->wake_up_time <= current_ticks) {
            list_pop_front(&sleep_list);
            thread_unblock(t);
        }
        else {
            /* 첫 번째 요소가 조건을 만족하지 않으면 더 이상 검사할 필요 없음 */
            break;
        }
    }
}



// 최대 우선순위 테스트 함수void
void test_max_priority(void)
{
  
  if (!list_empty(&ready_list))
    {
      struct thread *max_ready_t_ = list_entry(list_front(&ready_list), struct thread, elem);
      if(thread_current()->priority < max_ready_t_->priority){thread_yield();}
    }
}



/// 스케줄링 모드 set
void thread_set_scheduling_mode(bool mlfqs) {
    enum intr_level old_level = intr_disable();
if(!mlfqs){
    current_scheduling_mode = SCHEDULING_PRIORITY; }
else{ current_scheduling_mode =SCHEDULING_MLFQS; }

    intr_set_level(old_level);
}


static void calculate_priority(struct thread *t, void *aux UNUSED)
{
  if (t == idle_thread)
    return;

  /* priority = PRI_MAX - (recent_cpu / 4) - (nice * 2) */
    fixed_point_t temp = FP_DIV_INT(t->recent_cpu, 4);
    temp = FP_SUB(FP_CONST(PRI_MAX), temp);
    temp = FP_SUB_INT(temp, t->nice * 2);
    t->priority = FP_INT_PART(temp);

    if (t->priority > PRI_MAX)
        t->priority = PRI_MAX;
    if (t->priority < PRI_MIN)
        t->priority = PRI_MIN;
}

/* Calculate recent_cpu for a thread */
static void
calculate_recent_cpu(struct thread *t, void *aux UNUSED)
{
  if (t == idle_thread)
    return;
    /* recent_cpu = (2 * load_avg) / (2 * load_avg + 1) * recent_cpu + nice */
fixed_point_t coef1 = FP_MUL_INT(load_avg, 2);
    fixed_point_t coef2 = FP_ADD_INT(coef1, 1);
    fixed_point_t coefficient = FP_DIV(coef1, coef2);
    t->recent_cpu = FP_ADD_INT(FP_MUL(coefficient, t->recent_cpu), t->nice);

}

/* Calculate system load_avg */
static void
calculate_load_avg(void)
{
  int ready_threads = list_size(&ready_list);
  if (thread_current() != idle_thread)
    ready_threads++;
 
  /* load_avg = (59/60) * load_avg + (1/60) * ready_threads */
  fixed_point_t coef1 = FP_DIV_INT(FP_CONST(59), 60);
    fixed_point_t coef2 = FP_DIV_INT(FP_CONST(1), 60);
    fixed_point_t term1 = FP_MUL(coef1, load_avg);
    fixed_point_t term2 = FP_MUL_INT(coef2, ready_threads);
    load_avg = FP_ADD(term1, term2);
}


int
thread_get_load_avg (void) 
{
    enum intr_level old_level = intr_disable();
    int load_avg_int = FP_ROUND(FP_MUL_INT(load_avg, 100));
    intr_set_level(old_level);
    return load_avg_int;
}

int
thread_get_recent_cpu (void) 
{
    enum intr_level old_level = intr_disable();
    int recent_cpu_int = FP_ROUND(FP_MUL_INT(thread_current()->recent_cpu, 100));
    intr_set_level(old_level);
    return recent_cpu_int;
}

/* Offset of `stack' member within `struct thread'.
   Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof (struct thread, stack);




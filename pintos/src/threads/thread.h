#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/synch.h"   // 세마포어 사용을 위해 추가
#include "filesys/file.h"  // 추가: struct file의 정의를 포함

#define FDT_MAX 128 


// 플젝3------------------------------------

#ifndef USERPROG
// project3 
extern bool thread_prior_aging;
#endif

#ifndef USERPROG
void thread_aging(void);
#endif

// 3번 - mlfq
#define FP_SHIFT_AMOUNT 14  /* Number of fractional bits */
/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

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
        0 kB +---------------------------------+

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
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */

typedef int fixed_point_t; // mlfq에서 부동소수점 연산에 활용위해 추가


// 부동 소수점 연산 종류 enum 
/* Fixed-point operations */
typedef enum {
    FP_ADD_OP,
    FP_SUB_OP,
    FP_MUL_INT_OP,
    FP_DIV_INT_OP,
    FP_MUL_OP,
    FP_DIV_OP,
    FP_ADD_INT_OP,
    FP_SUB_INT_OP,
    FP_INT_PART_OP,
    FP_ROUND_OP
} fixed_point_op_t;

// 스케줄링 모드 확인위한 enum 추가 
// 기본이 priority임 
typedef enum {
    SCHEDULING_PRIORITY,    /* 우선순위 기반 스케줄러 */
    SCHEDULING_MLFQS        /* 멀티레벨 피드백 큐 스케줄러 */
} scheduling_mode_t;


struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */


#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
    int exit_status;                /* 종료 상태 */
    struct semaphore load_sema;     /* 로드 동기화를 위한 세마포어 */
    struct semaphore exit_sema;     /* 종료 동기화를 위한 세마포어 */
   struct semaphore wait_sema;     /* 부모가 자식을 처리하는 동안 기다리는 세마포어 */
    struct thread *parent;          /* 부모 스레드 */
    struct list_elem child_elem;    /* 자식 리스트의 요소 */
    struct list child_list;         /* Child thread list */
    bool exited;  // 종료 상태를 저장하는 변수 추가
    bool wrong_exit;  // 잘못된 종료 상태를 저장하는 변수 추가



    int reserved_entry[2];  // 0: stdin, 1: stdout
     /* 파일 디스크립터 테이블 추가 */
    struct file *fdt[FDT_MAX]; /* 파일 디스크립터 테이블 */
    int next_fd;           /* 다음에 할당할 파일 디스크립터 번호 */
    struct file *exec_file; /* 실행 파일에 대한 파일 포인터 */

    bool suppress_exit_msg;  // 종료 메시지 억제를 제어하는 플래그 추가

#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
      
 /* MLFQS scheduler 관련 필드 */
    int nice;                           
    fixed_point_t recent_cpu;           /* Recent CPU usage */

    /* 알람 클럭 관련 필드 */
    int64_t wake_up_time;               /* Tick to wake up at */

 };

// 전체 sleep threads를 한번에 관리하는 전역 슬립 리스트 추가 
extern struct list sleep_list;

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

// 우선순위 관련 함수들 
int thread_get_priority (void);
void thread_set_priority (int);

// mlfq 용 함수들
int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

// mlfq의 연산에서 활용돠는 
fixed_point_t fixed_point_operation(fixed_point_op_t op, fixed_point_t A, int B, fixed_point_t C);


// 새로 추가
struct thread *get_thread_by_tid(tid_t tid);


// 알람 클럭 함수들 
void thread_sleep(int64_t ticks);
void thread_awake(int64_t ticks);
bool cmp_wake_up_time(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);

// 스케줄링 모드 명확하게 구분 위해 알맞게 동작할 수 있도록 하는 함수
void thread_set_scheduling_mode(scheduling_mode_t mode);// set
scheduling_mode_t thread_get_scheduling_mode(void); // get
 

#endif /* threads/thread.h */


#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/interrupt.h"
#include "threads/vaddr.h"
#include "userprog/syscall.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/malloc.h"

#define MAX_STACK_SIZE (8 * 1024 * 1024)  // 8MB

static long long page_fault_cnt;

static void kill (struct intr_frame *);
static void page_fault (struct intr_frame *);

static bool try_grow_stack(void *fault_addr, void *esp);

void
exception_init (void) 
{
  intr_register_int (3, 3, INTR_ON, kill, "#BP Breakpoint Exception");
  intr_register_int (4, 3, INTR_ON, kill, "#OF Overflow Exception");
  intr_register_int (5, 3, INTR_ON, kill, "#BR BOUND Range Exceeded Exception");
  intr_register_int (0, 0, INTR_ON, kill, "#DE Divide Error");
  intr_register_int (1, 0, INTR_ON, kill, "#DB Debug Exception");
  intr_register_int (6, 0, INTR_ON, kill, "#UD Invalid Opcode Exception");
  intr_register_int (7, 0, INTR_ON, kill, "#NM Device Not Available Exception");
  intr_register_int (11, 0, INTR_ON, kill, "#NP Segment Not Present");
  intr_register_int (12, 0, INTR_ON, kill, "#SS Stack Fault Exception");
  intr_register_int (13, 0, INTR_ON, kill, "#GP General Protection Exception");
  intr_register_int (16, 0, INTR_ON, kill, "#MF x87 FPU Floating-Point Error");
  intr_register_int (19, 0, INTR_ON, kill, "#XF SIMD Floating-Point Exception");
  intr_register_int (14, 0, INTR_OFF, page_fault, "#PF Page-Fault Exception");
}

void
exception_print_stats (void) 
{
  printf ("Exception: %lld page faults\n", page_fault_cnt);
}

static void
kill (struct intr_frame *f) 
{
  switch (f->cs)
    {
    case SEL_UCSEG:
      /* 사용자 영역 예외: 프로세스 종료 */
      printf ("%s: dying due to interrupt %#04x (%s).\n",
              thread_name (), f->vec_no, intr_name (f->vec_no));
      intr_dump_frame (f);
      thread_exit (); 

    case SEL_KCSEG:
      /* 커널 영역 예외: 커널 패닉 */
      intr_dump_frame (f);
      PANIC ("Kernel bug - unexpected interrupt in kernel"); 

    default:
      /* 알 수 없는 세그먼트 */
      printf ("Interrupt %#04x (%s) in unknown segment %04x\n",
             f->vec_no, intr_name (f->vec_no), f->cs);
      thread_exit ();
    }
}

static void
page_fault (struct intr_frame *f) 
{
  bool not_present;  
  bool write;        
  bool user;         
  void *fault_addr;  

  struct thread *t = thread_current();

  asm ("movl %%cr2, %0" : "=r" (fault_addr));
  intr_enable ();
  page_fault_cnt++;

  not_present = (f->error_code & PF_P) == 0; 
  write = (f->error_code & PF_W) != 0;
  user = (f->error_code & PF_U) != 0;

  void *esp = user ? f->esp : t->saved_esp;
  if (esp == NULL)
    esp = f->esp;

  if (!is_user_vaddr(fault_addr)) {
    exit(-1); // 바로 종료, exit(-1)에서 메시지 출력
  }

  if (not_present) {
    if (!try_grow_stack(fault_addr, esp)) {
      exit(-1);
    }
    return;
  } else {
    // 권한 문제, write 불가능한 페이지에 write 시도 등
    exit(-1);
  }
}

static bool
try_grow_stack(void *fault_addr, void *esp) {
  if (fault_addr == NULL)
    return false;

  // esp와의 거리 계산
  if ((uint32_t)esp > (uint32_t)fault_addr && (uint32_t)esp - (uint32_t)fault_addr > 4096) {
    // pt-grow-bad 테스트 대비: esp-4096보다 낮은 주소 -> 확장 불가
    return false;
  }

  if ((uint32_t)fault_addr < (uint32_t)esp - 32) {
    return false;
  }

  void *limit = (void *)((uint8_t *)PHYS_BASE - MAX_STACK_SIZE);
  if (fault_addr < limit) {
    return false; 
  }

  void *page_addr = pg_round_down(fault_addr);
  if (pagedir_get_page(thread_current()->pagedir, page_addr) != NULL) {
    return false;
  }

  uint8_t *kpage = palloc_get_page(PAL_USER | PAL_ZERO);
  if (kpage == NULL)
    return false;

  if (!pagedir_set_page(thread_current()->pagedir, page_addr, kpage, true)) {
    palloc_free_page(kpage);
    return false;
  }

  return true;
}

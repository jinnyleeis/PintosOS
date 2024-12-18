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

/* 프로젝트 요구 사항: stack growth 구현 관련 상수 정의 */
#define MAX_STACK_SIZE (8 * 1024 * 1024)  // 8MB

static long long page_fault_cnt;

static void kill (struct intr_frame *);
static void page_fault (struct intr_frame *);

/* 스택 확장 함수 선언 */
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

  /* Page Fault 예외 처리 등록 */
  intr_register_int (14, 0, INTR_OFF, page_fault, "#PF Page-Fault Exception");
}

void
exception_print_stats (void) 
{
  printf ("Exception: %lld page faults\n", page_fault_cnt);
}

/* 사용자 프로그램이 발생시킨 예외 처리 */
static void
kill (struct intr_frame *f) 
{
  switch (f->cs)
    {
    case SEL_UCSEG:
      /* 유저 영역에서 예외 발생: 해당 프로세스 종료 */
      printf ("%s: dying due to interrupt %#04x (%s).\n",
              thread_name (), f->vec_no, intr_name (f->vec_no));
      intr_dump_frame (f);
      thread_exit (); 

    case SEL_KCSEG:
      /* 커널 영역에서 예외 발생: 커널 패닉 */
      intr_dump_frame (f);
      PANIC ("Kernel bug - unexpected interrupt in kernel"); 

    default:
      /* 알 수 없는 세그먼트: 종료 */
      printf ("Interrupt %#04x (%s) in unknown segment %04x\n",
             f->vec_no, intr_name (f->vec_no), f->cs);
      thread_exit ();
    }
}

/* 페이지 폴트 처리 함수 */
static void
page_fault (struct intr_frame *f) 
{
  bool not_present;  
  bool write;        
  bool user;         
  void *fault_addr;  
  struct thread *t = thread_current();

  /* 잘못된 주소 획득 */
  asm ("movl %%cr2, %0" : "=r" (fault_addr));
  intr_enable ();  /* 인터럽트 다시 활성화 */
  page_fault_cnt++;

  /* 에러 코드 파싱 */
  not_present = (f->error_code & PF_P) == 0; 
  write = (f->error_code & PF_W) != 0;
  user = (f->error_code & PF_U) != 0;

  /* 유저/커널 모드에 따라 스택 포인터 결정 */
  void *esp = user ? f->esp : t->saved_esp;
  if (esp == NULL)
    esp = f->esp; // 혹은 적절히 커널에서 esp 추적하도록

  /* 유효한 사용자 주소인지 검사 */
  if (!is_user_vaddr(fault_addr)) {
    t->wrong_exit = true;
    exit(-1);
  }

  /* 페이지가 없는 경우(not_present)일 때 스택 확장 시도 */
  // 현재는 lazy load나 spt를 구현하지 않았으므로 fault_addr에 대한 매핑이 없으면 스택 확장만 고려
  // pt-grow-bad를 위해 esp에서 4096바이트 이상 떨어진 경우도 처리
  if (not_present) {
    // 스택 확장 가능성 체크
    // 만약 stack growth 조건 불충족 시 종료
    if (!try_grow_stack(fault_addr, esp)) {
      // 확장 불가한 경우 -1 종료
      t->wrong_exit = true;
      exit(-1);
    }
    return; // 성공적으로 스택 확장 후 반환
  } else {
    // 존재하지만 write 금지 페이지를 write하려고 하는 등 권한 문제 발생 시 종료
    t->wrong_exit = true;
    exit(-1);
  }
}

/* 스택 확장 시도 함수 
   조건:
   - fault_addr < PHYS_BASE
   - fault_addr >= esp - 32 (푸시 명령용)
   - esp보다 4096바이트 이상 아래이면(pt-grow-bad 대비) 안됨
   - 최대 8MB 스택 제한 준수 */
static bool
try_grow_stack(void *fault_addr, void *esp) {
  if (fault_addr == NULL)
    return false;

  // 스택 pointer와의 거리 계산
  // fault_addr가 esp보다 낮은 주소를 접근하는데 
  // 그 차이가 4096바이트(4KB) 이상이면 pt-grow-bad 테스트 실패 → false 리턴
  if ((uint32_t)esp > (uint32_t)fault_addr && (uint32_t)esp - (uint32_t)fault_addr > 4096) {
    return false;
  }

  // fault_addr가 esp - 32보다 작으면 stack 확장 불필요한 접근으로 간주
  if ((uint32_t)fault_addr < (uint32_t)esp - 32) {
    return false;
  }

  // 최대 8MB 제한: PHYS_BASE - 8MB <= fault_addr < PHYS_BASE 범위 내여야 함
  void *limit = (void *)((uint8_t *)PHYS_BASE - MAX_STACK_SIZE);
  if (fault_addr < limit) {
    return false; 
  }

  // 페이지 경계로 맞춤
  void *page_addr = pg_round_down(fault_addr);

  // 이미 매핑된 페이지가 아닌지 확인
  if (pagedir_get_page(thread_current()->pagedir, page_addr) != NULL) {
    // 이미 페이지가 존재 -> 굳이 새로 확장할 필요 없음(하지만 여기 왔다는건 not_present였으니 예외적)
    return false;
  }

  // 새로운 페이지 할당
  uint8_t *kpage = palloc_get_page(PAL_USER | PAL_ZERO);
  if (kpage == NULL)
    return false;

  // 할당한 페이지를 user 페이지 테이블에 매핑
  if (!pagedir_set_page(thread_current()->pagedir, page_addr, kpage, true)) {
    palloc_free_page(kpage);
    return false;
  }

  return true;
}

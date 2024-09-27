#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "process.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/synch.h" // sema_down, sema_up 포함
#include "userprog/pagedir.h"  // pagedir_get_page 함수 사용을 위해 포함


static void syscall_handler (struct intr_frame *);
void halt (void) NO_RETURN;
int write (int fd, const void *buffer, unsigned length);
void exit (int status) NO_RETURN;
int read (int fd, void *buffer, unsigned length);
int wait (tid_t);
tid_t exec (const char *file);
//void check_valid_vaddr(uint32_t *esp, int n);
void check_user_vaddr(const void *vaddr);  // 유효성 검사 함수

/* 사용자 주소 유효성 확인 함수 */
void check_user_vaddr(const void *vaddr) {
    if (!is_user_vaddr(vaddr) || pagedir_get_page(thread_current()->pagedir, vaddr) == NULL) {
        exit(-1);  // 유효하지 않은 포인터는 exit(-1)로 프로세스 종료
    }
}

void check_valid_vaddr(const void *vaddr);  // 유효성 검사 함수

/* 사용자 주소 유효성 확인 함수 */
void check_valid_vaddr(const void *vaddr) {
    if (!is_user_vaddr(vaddr) || pagedir_get_page(thread_current()->pagedir, vaddr) == NULL) {
        exit(-1);  // 유효하지 않은 포인터는 exit(-1)로 프로세스 종료
    }
}

// 사용자 주소 유효성 확인 함수

/*
esp는 uint32_t 크기(32비트)의 정수형 데이터를 가리킴
esp[i]는 배열의 i번째 요소에 해당하는 값(해당 값은 32비트의 부호 없는 정수)
그런데, 유저 스택에 저장된 값 중 하나일 것임 
여기선 - 이 스택에 저장된 값 중에서 메모리 주소를 사용하는 것임 
아래의 주소 유효성 함수에선

esp[i] 값 자체는 정수지만, 이 값을 포인터로 사용할 것이므로, 
1. uintptr_t로 변환
'uintptr_t로 명시적 형변환을 통해 포인터로 사용 가능한 정수형 값으로 변환'
2. (void *)로 캐스팅
is_user_vaddr() 함수는 인자로 메모리 주소를 받음
그 인자의 타입은 void *임
따라서 uintptr_t로 변환된 값을 포인터로 사용하려면 다시 void *로 캐스팅해야함 

 예시) 
esp[i]가 0x08048000이라면, 이를 is_user_vaddr()에 전달할 때 (void *) 0x08048000으로 전달되어 
메모리 주소처럼 동작하는 값으로 취급됨  
즉, 해당 esp[i]에 저장되어있는 정수값 메모리 주소를 가리키는 포인터 값으로 취급됨
이 값을 is_user_vaddr() 함수에 전달할 때 포인터로 캐스팅함 

*/
/*

void check_valid_vaddr(uint32_t *esp, int n) {
    for (int i = 0; i <= n; ++i) {
        // 명시적 형변환을 통해 부호 없는 32비트 정수로 변환
        if (!is_user_vaddr((void *)(uintptr_t)esp[i])) {
            exit(-1);  // 유효하지 않은 포인터는 exit(-1)로 프로세스 종료
            // 이거 테스트하는 테스트 케이스 있는 듯!!
        }
    }
}
*/

/*
void check_valid_vaddr(const void *vaddr) {
    if (!is_user_vaddr(vaddr) || pagedir_get_page(thread_current()->pagedir, vaddr) == NULL) {
        exit(-1);  // 유효하지 않은 포인터는 exit(-1)로 프로세스 종료
    }
}
*/


/*
void check_user_vaddr (const void *vaddr) {
  if (!is_user_vaddr(vaddr)) {
    exit(-1); // 잘못된 주소일 경우 프로세스 종료
  }
}
*/

void syscall_init (void) {
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler (struct intr_frame *f UNUSED) {

    /* esp가 가리키는 주소가 유효한지 먼저 검사 */
  check_user_vaddr(f->esp);

  /* 시스템 호출 번호를 가져옴 */
  uint32_t syscall_number = *(uint32_t *)(f->esp);

  /* esp에서 전달되는 파라미터에 대한 유효성 검사 및 호출 */
  switch (syscall_number) {
    case SYS_HALT:
      halt();
      break;

    case SYS_EXIT:
      /* status 파라미터에 대한 유효성 검사 */
      check_user_vaddr((f->esp + 4));
      exit(*(uint32_t *)(f->esp + 4));
      break;

    case SYS_EXEC:
      /* cmd_line 파라미터에 대한 유효성 검사 */
      check_user_vaddr((f->esp + 4));
      f->eax = exec((const char *)*(uint32_t *)(f->esp + 4));
      break;

    case SYS_WAIT:
      /* pid 파라미터에 대한 유효성 검사 */
      check_user_vaddr(f->esp + 4);
      f->eax = wait((tid_t)*(uint32_t *)(f->esp + 4));
      break;

    case SYS_READ:
      /* read 함수의 fd, buffer, size 파라미터에 대한 유효성 검사 */
      check_user_vaddr(f->esp + 20);  // fd
      check_user_vaddr(f->esp + 24);  // buffer
      check_user_vaddr(f->esp + 28);  // size
      f->eax = read((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*(uint32_t *)(f->esp + 28));
      break;

    case SYS_WRITE:
      /* write 함수의 fd, buffer, size 파라미터에 대한 유효성 검사 */
      check_user_vaddr(f->esp + 4);  // fd
      check_user_vaddr(f->esp + 8);  // buffer
      check_user_vaddr(f->esp + 12);  // size
      f->eax = write((int)*(uint32_t *)(f->esp + 4), (const void *)*(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
      break;

    default:
      /* 처리되지 않은 시스템 호출 번호 */
      printf("Unknown system call: %d\n", syscall_number);
      thread_exit();
      break;
  }

  // esp가 유효한 주소인지 체크
  //ASSERT(f->esp != NULL);
  //ASSERT(is_user_vaddr(f->esp));  // 사용자 영역 주소인지 확인

/*
  switch (*(uint32_t *)(f->esp)) {
    case SYS_HALT:
      halt();
      break;
    case SYS_WRITE:
      write((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
      break;   
    case SYS_EXIT:
      exit(*(uint32_t *)(f->esp + 4));
      break;
    case SYS_EXEC:
     // check_user_vaddr(f->esp + 4);
     // f->eax = exec((const char *)*(uint32_t *)(f->esp + 4));
     //  check_valid_vaddr(f->esp, 1);
    //   f->eax = exec((const char *)(f->esp + 4));
  //  f->eax = exec((const char *)*((uint32_t *)f->esp + 1));

      break; 
    case SYS_WAIT:
   // check_valid_vaddr(f->esp, 1);
  //   f->eax = wait((tid_t)*(uint32_t *)(f->esp + 4));
 // f->eax = wait((tid_t)*((uint32_t *)f->esp + 1));


      break;
    case SYS_READ:
    //  f->eax = read((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
      break;
    default:
      break;
  }

  
  /*
    uint32_t *esp = (uint32_t *)f->esp;
    check_valid_vaddr(esp); // esp 유효성 검사

    switch (*esp) {
        case SYS_HALT:
            halt();
            break;
        case SYS_WRITE:
            check_valid_vaddr(esp + 1);
            check_valid_vaddr((const void *)*(esp + 2));
            f->eax = write((int)*(esp + 1), (const void *)*(esp + 2), (unsigned)*(esp + 3));
            break;
        case SYS_EXIT:
            check_valid_vaddr(esp + 1);
            exit((int)*(esp + 1));
            break;
        case SYS_EXEC:
            check_valid_vaddr(esp + 1);
            f->eax = exec((const char *)*(esp + 1));
            break;
        case SYS_WAIT:
            check_valid_vaddr(esp + 1);
            f->eax = wait((tid_t)*(esp + 1));
            break;
        default:
            break;
    }*/


}

void halt (void) {
  shutdown_power_off(); // 시스템 종료
}

void exit (int status) {
  struct thread *cur = thread_current();
  cur->exit_status = status;  // 종료 상태 설정
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_exit ();
}

int write (int fd, const void *buffer, unsigned length) {
  if (fd == 1) {
    putbuf(buffer, length);
    return length;
  }
  return -1; 
}


int wait(tid_t tid) {
    return process_wait(tid);
}


tid_t exec(const char *cmd_line) {
  if (cmd_line == NULL || !is_user_vaddr(cmd_line))
        exit(-1);

    tid_t tid = process_execute(cmd_line);

    if (tid == TID_ERROR)
        return -1;

    struct thread *child = get_thread_by_tid(tid);

    /* 자식 프로세스의 로드 완료 대기 */
    sema_down(&child->load_sema);

    if (!child->load_success)
        return -1;

    return tid;
}

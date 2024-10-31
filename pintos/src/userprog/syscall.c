#include "userprog/syscall.h"
#include "lib/stdio.h"
#include "lib/kernel/stdio.h"
#include "lib/string.h"


//#include "devices/console.h"  // printf 함수 사용을 위해 추가
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "process.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/synch.h" // sema_down, sema_up 포함
#include "userprog/pagedir.h"  // pagedir_get_page 함수 선언

#include "filesys/filesys.h"
#include "filesys/file.h"

//디버깅용
#include "devices/timer.h"


static struct lock filesys_lock; /* 파일 시스템 동기화를 위한 전역 락 */


static void syscall_handler(struct intr_frame *);
void halt(void) NO_RETURN;
int write(int fd, const void *buffer, unsigned length);
void exit(int status) NO_RETURN;
int read(int fd, void *buffer, unsigned length);
int wait(tid_t);
tid_t exec(const char *file);
void check_valid_vaddr(const void *vaddr);
void check_valid_buffer(const void *buffer, unsigned size);
void check_valid_string(const char *str);


bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);

void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

// 사용자 주소 유효성 검사 함수
void check_valid_vaddr(const void *vaddr) {
    if (!is_user_vaddr(vaddr)) {
        exit(-1);  // 유효하지 않은 포인터는 exit(-1)로 프로세스 종료
    }

      // 페이지 디렉토리에서 해당 페이지가 유효한지 확인
    if (pagedir_get_page(thread_current()->pagedir, vaddr) == NULL) {
        exit(-1);  // 매핑되지 않은 페이지일 경우 프로세스 종료
    }
}

// 버퍼의 유효성 검사 함수
void check_valid_buffer(const void *buffer, unsigned size) {
    for (unsigned i = 0; i < size; i++) {
        check_valid_vaddr((const char *)buffer + i);
    }
}

/* 문자열의 유효성 검사 함수 */
void check_valid_string(const char *str)
{
  check_valid_vaddr((const void *)str);
  while (*str != '\0')
  {
    check_valid_vaddr((const void *)str);
    str++;
  }
}



void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
    lock_init(&filesys_lock); /* 전역 락 초기화 */
}

static void syscall_handler(struct intr_frame *f UNUSED) {

      // 먼저 esp가 유효한 사용자 주소인지 확인
    if (f->esp == NULL || !is_user_vaddr(f->esp)) {
        exit(-1);  // 유효하지 않은 esp일 경우 프로세스 종료
    }

    // esp+4가 유효한 사용자 주소인지 확인
    if (!is_user_vaddr(f->esp + 4)) {
        exit(-1);  // 잘못된 스택 포인터일 경우 프로세스 종료
    }

    switch (*(uint32_t *)(f->esp)) {
        case SYS_HALT:
            halt();
            break;
        case SYS_WRITE:
            check_valid_vaddr(f->esp + 4); // fd
            check_valid_vaddr(f->esp + 8); // buffer
            check_valid_vaddr(f->esp + 12); // size
            f->eax = write((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
            break;   
        case SYS_EXIT:
            check_valid_vaddr(f->esp + 4); // status
            exit(*(uint32_t *)(f->esp + 4));
            break;
        case SYS_EXEC:
            check_valid_vaddr(f->esp + 4);
            f->eax = exec((const char *)*(uint32_t *)(f->esp + 4));
            break; 
        case SYS_WAIT:
            check_valid_vaddr(f->esp + 4); // tid
            f->eax = wait((tid_t)*(uint32_t *)(f->esp + 4));
            break;
        case SYS_READ:
            check_valid_vaddr(f->esp + 4); // fd
            check_valid_vaddr(f->esp + 8); // buffer
            check_valid_vaddr(f->esp + 12); // size
            f->eax = read((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
            break;
        case SYS_FIBONACCI:
            f->eax = fibonacci(*(int *)(f->esp + 4));
            break;
        case SYS_MAX_OF_FOUR_INT:
            f->eax = max_of_four_int(*(int *)(f->esp + 4), *(int *)(f->esp + 8),
                                    *(int *)(f->esp + 12), *(int *)(f->esp + 16));
            break;
            
        case SYS_CREATE:
            check_valid_vaddr(f->esp + 4);
            check_valid_vaddr(f->esp + 8);
            f->eax = create((const char *)*(uint32_t *)(f->esp + 4), *(unsigned *)(f->esp + 8));
            break;
        case SYS_REMOVE:
            check_valid_vaddr(f->esp + 4);
            f->eax = remove((const char *)*(uint32_t *)(f->esp + 4));
            break;
        case SYS_OPEN:
            check_valid_vaddr(f->esp + 4);
            f->eax = open((const char *)*(uint32_t *)(f->esp + 4));
            break;
        case SYS_CLOSE:
            check_valid_vaddr(f->esp + 4);
            close(*(int *)(f->esp + 4));
             break;
        case SYS_FILESIZE:
            check_valid_vaddr(f->esp + 4);
            f->eax = filesize(*(int *)(f->esp + 4));
            break;
        case SYS_SEEK:
            check_valid_vaddr(f->esp + 4);
            check_valid_vaddr(f->esp + 8);
            seek(*(int *)(f->esp + 4), *(unsigned *)(f->esp + 8));
            break;
        case SYS_TELL:
            check_valid_vaddr(f->esp + 4);
            f->eax = tell(*(int *)(f->esp + 4));
            break; 
            
        default:
            exit(-1); // 잘못된 시스템 콜 번호의 경우 프로세스 종료
            break;
    }
}

void halt(void) {
    shutdown_power_off(); // 시스템 종료
}

void exit(int status){

 struct thread *cur = thread_current();

   

    
    // 이미 종료된 상태라면 중복 출력 방지
    if (!cur->exited) {
        // 상태를 종료로 표시
        cur->exited = true;
        cur->exit_status = status;

if(!cur->wrong_exit && !cur->suppress_exit_msg){
        printf("%s: exit(%d)\n", thread_name(), status);
        }


     /* 부모 스레드가 자식의 종료를 기다리는 경우, 종료를 알림 */
      sema_up(&cur->exit_sema);

       /* 부모가 자식을 정리할 때까지 대기 */
       // 자식이 자기가 대기시한다는거임
 if (cur->parent != NULL) {
        sema_down(&cur->wait_sema);
    }

     // 항상 thread_exit()을 호출하여 종료
     

  
    }

      // 잘못된 메모리 접근으로 인한 종료라면 바로 종료
    if (!cur->wrong_exit&&!cur->exited) {
        thread_exit(); // 스레드 종료
    }


}

int write(int fd, const void *buffer, unsigned length) {
    check_valid_buffer(buffer, length);

    struct thread *cur = thread_current();
    int bytes_written = 0;

     //  printf("write() called with fd=%d, length=%u\n", fd, length);


    if (fd == 1) {
        putbuf(buffer, length);
    //  printf("write(): fd=1, wrote to console, length=%u\n", length);

        return length;
    } else if (fd >= 2 && fd < cur->next_fd && cur->fdt[fd] != NULL) {
        lock_acquire(&filesys_lock);
        bytes_written = file_write(cur->fdt[fd], buffer, length);
        lock_release(&filesys_lock);

       printf("else if write(): file_write() returned %d\n", bytes_written);



    } else {

            //  printf("write(): invalid fd=%d\n", fd);
                   printf("else : write(): file_write() returned %d\n", bytes_written);


        bytes_written = -1;

    }
           printf("outside write(): file_write() returned %d\n", bytes_written);

    return bytes_written;
}


int wait(tid_t tid) {

 struct thread *child = get_child_thread_by_tid(thread_current(), tid);
    if (child == NULL || child->parent != thread_current()) {
        return -1;  // 자식 프로세스가 없거나 이미 대기된 경우
    }


     timer_sleep(10);

    return process_wait(tid);
}



int read(int fd, void *buffer, unsigned size) {
    check_valid_buffer(buffer, size); // 버퍼 유효성 검사 추가

    struct thread *cur = thread_current();
    int bytes_read = 0;

    

    if (fd == 0) {  // 파일 디스크립터가 콘솔 입력인 경우
        unsigned int i;
        for (i = 0; i < size; i++) {
              if (buffer == NULL) {  // 버퍼가 NULL일 경우 종료하도록 
                break;
            }
        
            char c = input_getc();  // 콘솔에서 문자 입력
            if (c == '\0') {  // NULL 입력 시에도 종료
                break;
            }
            ((char *)buffer)[i] = c;  // 입력된 문자를 -> 버퍼 저장 
             bytes_read++;

        }

       return bytes_read;  // 실제로 읽은 문자의 수 반환
    } else if (fd >= 2 && fd < cur->next_fd && cur->fdt[fd] != NULL) {
        /* 파일에서 읽기 */
        lock_acquire(&filesys_lock);
        bytes_read = file_read(cur->fdt[fd], buffer, size);
        lock_release(&filesys_lock);
        
    } else {
        bytes_read = -1; /* 잘못된 fd */
       
    }
     return bytes_read;
    
}




tid_t exec(const char *file) {

check_valid_string(file);  // 유효한 문자열인지 확인

  tid_t pid = process_execute(file);  // 새로운 프로세스 생성 및 실행


    struct thread *child = get_child_thread_by_tid(thread_current(), pid);

    // 자식 스레드가 NULL이면 -1 반환
    if (child == NULL) {
        return -1;
    }

    // 자식이 로드를 완료할 때까지 대기
    sema_down(&child->load_sema);

    // 로드 실패 시 -1 반환
    if (child->exit_status == -1) {
        return -1;
    }

    return pid;
    }

// 추가 구현 함수 2개 !!
/* 동적 프로그래밍으로 구현한 dp */
int fibonacci(int n) {
  if (n <= 1)
    return n;

  // DP 배열 생성
  int fib[n + 1];

  // 초기값 설정
  fib[0] = 0;
  fib[1] = 1;

  // bottom-up 방식으로 Fibonacci 수 계산
  for (int i = 2; i <= n; i++) {
    fib[i] = fib[i - 1] + fib[i - 2];
  }

  // n번째 Fibonacci 수 반환
  return fib[n];
}

int max_of_four_int(int a, int b, int c, int d) {
  int max = a;
  if (b > max) max = b;
  if (c > max) max = c;
  if (d > max) max = d;
  return max;
}



// 프로젝트2 

/* create 시스템 콜 */
bool create(const char *file, unsigned initial_size)
{
  check_valid_string(file);
  lock_acquire(&filesys_lock);

   //   printf("create(): file=%s, initial_size=%u\n", file, initial_size);


  bool success = filesys_create(file, initial_size);
  lock_release(&filesys_lock);
  return success;
}

/* remove 시스템 콜 */
bool remove(const char *file)
{
  check_valid_string(file);
  lock_acquire(&filesys_lock);
  bool success = filesys_remove(file);
  lock_release(&filesys_lock);
  return success;
}

/* open 시스템 콜 */
/*int open(const char *file)
{
  check_valid_string(file);
  lock_acquire(&filesys_lock);
  struct file *opened_file = filesys_open(file);
  if (opened_file == NULL)
  {
    lock_release(&filesys_lock);
    return -1;
  }
 struct thread *cur = thread_current();
  
  // 파일 이름 비교하여 실행 파일인지 확인
  if (strcmp(file, cur->name) == 0)
  {
    // 실행 파일인 경우 쓰기 거부
    file_deny_write(opened_file);
  }

  int fd = cur->next_fd;
  cur->fdt[fd] = opened_file;
  cur->next_fd++;
  lock_release(&filesys_lock);
  return fd;
}
*/

int open(const char *file)
{
  check_valid_string(file);
  lock_acquire(&filesys_lock);
      printf("open(): 파일 '%s'을(를) 열려고 시도 중\n", file);

  struct file *opened_file = filesys_open(file);
  if (opened_file == NULL)
  {
            printf("open(): filesys_open()이 파일 '%s'에 대해 NULL을 반환함\n", file);

    lock_release(&filesys_lock);
    return -1;
  }
  struct thread *cur = thread_current();

  printf("open(): filesys_open()이 파일 '%s'에 대해 성공함\n", file);
    printf("open(): 파일 '%s'을(를) 스레드 이름 '%s'과(와) 비교 중\n", file, cur->name);
 // 파일 이름을 비교하여 실행 파일인지 확인
  if (strcmp(file, cur->name) == 0)
  {
            printf("open(): 파일 '%s'에 대한 쓰기 접근을 거부함\n", file);

    // 실행 파일인 경우 쓰기 거부
    file_deny_write(opened_file);
  }
  

  int fd = cur->next_fd;
      printf("open(): 파일 디스크립터 %d 할당됨\n", fd);

  cur->fdt[fd] = opened_file;
  cur->next_fd++;
  lock_release(&filesys_lock);
  return fd;
}

/* close 시스템 콜 */
/*
void close(int fd)
{
  struct thread *cur = thread_current();
  if (fd < 2 || fd >= cur->next_fd || cur->fdt[fd] == NULL)
  {
    exit(-1);
  }
  lock_acquire(&filesys_lock);

   // 실행 파일인지 확인
  if (cur->fdt[fd] == cur->exec_file)
  {
   // file_allow_write(cur->fdt[fd]);
    cur->exec_file = NULL;
  }


  file_close(cur->fdt[fd]);
  cur->fdt[fd] = NULL;
  lock_release(&filesys_lock);
}
*/
void close(int fd)
{
  struct thread *cur = thread_current();
  if (fd < 2 || fd >= cur->next_fd || cur->fdt[fd] == NULL)
  {
    exit(-1);
  }
  lock_acquire(&filesys_lock);


  // 실행 파일인지 확인
  
  if (cur->fdt[fd] == cur->exec_file)
  {
    file_allow_write(cur->fdt[fd]);
    cur->exec_file = NULL;
  }

  
  file_close(cur->fdt[fd]);
  cur->fdt[fd] = NULL;

  lock_release(&filesys_lock);
  
}

/* filesize 시스템 콜 */
int filesize(int fd)
{
  struct thread *cur = thread_current();
  if (fd < 2 || fd >= cur->next_fd || cur->fdt[fd] == NULL)
  {
    exit(-1);
  }
  lock_acquire(&filesys_lock);
  int size = file_length(cur->fdt[fd]);
  lock_release(&filesys_lock);
  return size;
}


/* seek 시스템 콜 */
void seek(int fd, unsigned position)
{
  struct thread *cur = thread_current();
  if (fd < 2 || fd >= cur->next_fd || cur->fdt[fd] == NULL)
  {
    exit(-1);
  }
  lock_acquire(&filesys_lock);
  file_seek(cur->fdt[fd], position);
  lock_release(&filesys_lock);
}

/* tell 시스템 콜 */
unsigned tell(int fd)
{
  struct thread *cur = thread_current();
  if (fd < 2 || fd >= cur->next_fd || cur->fdt[fd] == NULL)
  {
    exit(-1);
  }
  lock_acquire(&filesys_lock);
  unsigned position = file_tell(cur->fdt[fd]);
  lock_release(&filesys_lock);
  return position;
}




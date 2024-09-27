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
#include "userprog/pagedir.h"  // pagedir_get_page 함수 선언


static void syscall_handler(struct intr_frame *);
void halt(void) NO_RETURN;
int write(int fd, const void *buffer, unsigned length);
void exit(int status) NO_RETURN;
int read(int fd, void *buffer, unsigned length);
int wait(tid_t);
tid_t exec(const char *file);
void check_valid_vaddr(const void *vaddr);
void check_valid_buffer(const void *buffer, unsigned size);

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

void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
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
            write((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
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
        default:
            exit(-1); // 잘못된 시스템 콜 번호의 경우 프로세스 종료
            break;
    }
}

void halt(void) {
    shutdown_power_off(); // 시스템 종료
}

void exit(int status) {

 struct thread *cur = thread_current();
    
    // 이미 종료된 상태라면 중복 출력 방지
    if (!cur->exited) {
        // 상태를 종료로 표시
        cur->exited = true;

        printf("%s: exit(%d)\n", thread_name(), status);
        cur->exit_status = status;
    }

    // 항상 thread_exit()을 호출하여 종료
    thread_exit();


   // printf("%s: exit(%d)\n", thread_name(), status);
   // thread_exit();
}

int write(int fd, const void *buffer, unsigned length) {
    check_valid_buffer(buffer, length); // 버퍼 유효성 검사 추가

    if (fd == 1) {
        putbuf(buffer, length);
        return length;
    }
    return -1;
}

int wait(tid_t tid) {
    return process_wait(tid);
}

int read(int fd, void *buffer, unsigned size) {
    check_valid_buffer(buffer, size); // 버퍼 유효성 검사 추가

    unsigned int i = 0;
    if (fd == 0) {
        for (i = 0; i < size; i++) {
            char c = input_getc();  // 콘솔로부터 문자 입력
            if (c == '\0' || !buffer) {
                break;
            }
            ((char *)buffer)[i] = c;
        }
    }
    return i;
}

tid_t exec(const char *file) {
    return process_execute(file);
}

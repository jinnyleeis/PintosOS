#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
// 추가
#include "threads/synch.h"
#include <list.h>


// 자식 프로세스를 관리하기 위한 별도의 구조체
/* 자식 프로세스 정보를 저장하는 구조체 */
struct child_process {
    tid_t tid;                    // 자식 프로세스의 TID
    int exit_status;              // 자식 프로세스의 종료 상태
    bool wait;                    // 부모가 이미 wait 호출했는지 여부
    bool exit;                    // 자식 프로세스가 종료되었는지 여부
    bool load_success;            // 자식 프로세스 로드 성공 여부
    struct semaphore wait_sema;   // 부모가 자식의 종료를 기다리기 위한 세마포어
    struct semaphore load_sema;   // 부모가 자식의 로드 완료를 기다리기 위한 세마포어
    struct list_elem elem;        // 자식 프로세스 리스트의 리스트 요소
};

// 기존 
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);


/* 함수 선언 수정 - 자식 프로세스 관리용 */
struct child_process* get_child_process(struct thread *t, tid_t tid);
void remove_child_process(struct child_process *cp);





#endif /* userprog/process.h */



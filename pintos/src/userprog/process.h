#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include <string.h> // strncmp를 사용하기 위한 헤더 파일 추가
#include "userprog/syscall.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
int strncmp(const char *s1, const char *s2, size_t n);

// process.h 파일 내 추가
struct thread *get_child_thread_by_tid(struct thread *parent, tid_t child_tid);

void exit(int status); // 사용자 정의 exit 함수의 프로토타입

#endif /* userprog/process.h */

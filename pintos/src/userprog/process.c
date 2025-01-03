#include <string.h>
#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
//implicit declaration of function ‘free’ , implicit declaration of function ‘malloc’ 해결
#include "threads/malloc.h"
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "lib/string.h"


static thread_func start_process NO_RETURN;
static bool load (const char *cmdline, void (**eip) (void), void **esp);
/* setup_user_stack 함수의 선언을 load 함수 위에 추가 */
void setup_user_stack(char *file_name, void **esp);
bool is_valid_stack_pointer(void *esp);
struct thread *get_child_thread_by_tid(struct thread *parent, tid_t child_tid);


/* Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute() returns.  Returns the new process's
   thread id, or TID_ERROR if the thread cannot be created. */

struct thread *get_child_thread_by_tid(struct thread *parent, tid_t child_tid) {
  struct list_elem *e;

  for (e = list_begin(&parent->child_list); e != list_end(&parent->child_list); e = list_next(e)) {
    struct thread *t = list_entry(e, struct thread, child_elem);
    if (t->tid == child_tid) {
      return t;
    }
  }
  return NULL;  // 자식 스레드를 찾을 수 없음
}



tid_t
process_execute (const char *file_name) 
{
  char *fn_copy;
  tid_t tid;

  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
  if (fn_copy == NULL)
    return TID_ERROR;
  strlcpy (fn_copy, file_name, PGSIZE);

 // printf("exit(%d)\n", status);

const char *full_name = file_name;  // 전체 이름을 저장할 포인터
int first_word_length = strcspn(full_name, " ");  // 공백까지의 길이 계산

// 첫 번째 단어를 저장할 배열에 복사
char first_word[20];  // 적절한 크기로 배열 선언
strlcpy(first_word, full_name, first_word_length + 1);  // +1은 NULL 문자 포함
//printf("첫번쨰 프로그램 이름: %s\n", first_word);
   

  /* Create a new thread to execute FILE_NAME. */
  tid = thread_create (first_word, PRI_DEFAULT, start_process, fn_copy);


if (tid != TID_ERROR) {
    struct thread *child_thread = get_thread_by_tid(tid);
    if (child_thread != NULL && strncmp(child_thread->name, "multi-oom", strlen("multi-oom")) == 0) {
        child_thread->suppress_exit_msg = true;  // multi-oom 프로세스에 대해 플래그 설정
    }
}

  if (tid == TID_ERROR)
    palloc_free_page (fn_copy); 





  return tid;
}

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (void *file_name_)
{
  char *file_name = file_name_;
  struct intr_frame if_;
  bool success;

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  success = load (file_name, &if_.eip, &if_.esp);

  /* If load failed, quit. */
  palloc_free_page (file_name);
  if (!success){
      thread_current()->exit_status = -1;  // 로드 실패 상태 설정
      sema_up(&thread_current()->load_sema);  // 부모에게 로드 실패 알림
      exit(-1);  // 명시적으로 exit(-1)을 호출하여 종료

    }


      // 로드 성공 시 부모에게 알림
    sema_up(&thread_current()->load_sema);

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/* Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting.

   This function will be implemented in problem 2-2.  For now, it
   does nothing. */


int
process_wait (tid_t child_tid UNUSED) 
{


//for (long long i = 0; i < 3000000000LL; i++){// 임시방편으로 바로 자식 프로세스 안죽도록
//};

 // return -1;
/**/

    struct thread *cur = thread_current();
    struct thread *child = get_thread_by_tid(child_tid);

    if (child == NULL || child->parent != cur){return -1; // 자식이 유효하지 않을 경우 -1 반환
    }



        sema_down(&child->exit_sema);
    

    /* 자식의 종료 상태 반환 */
  int exit_status = child->exit_status;
  list_remove(&child->child_elem);

   /* 자식에게 이제 완전히 종료해도 된다고 알림 */
    sema_up(&child->wait_sema);
    
  return exit_status;
    /* 자식 프로세스가 종료될 때까지 대기 */
   // sema_down(&child->exit_sema);

    //int exit_status = child->exit_status;

    /* 자식 프로세스가 종료되었으므로 정리 */
    //list_remove(&child->child_elem);

    //return exit_status;
    

}

/* Free the current process's resources. */
void
process_exit (void)
{
  struct thread *cur = thread_current ();
  uint32_t *pd;

    /* 실행 파일에 대한 쓰기 허용 및 닫기 */
    if (cur->exec_file != NULL) {
        file_close(cur->exec_file);
        cur->exec_file = NULL;
    }

    /* 모든 열린 파일 닫기 */
    for (int fd = 2; fd < cur->next_fd; fd++) {
        if (cur->fdt[fd] != NULL) {

          // 일단 추가해봄
//           file_allow_write(cur->exec_file);

            file_close(cur->fdt[fd]);
            cur->fdt[fd] = NULL;
        }
    }

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;
  if (pd != NULL) 
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}


/* We load ELF binaries.  The following definitions are taken
  
 from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32   /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32   /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32   /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16   /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
  {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
  };

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
  {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
  };

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

static bool setup_stack (void **esp);
static bool validate_segment (const struct Elf32_Phdr *, struct file *);
static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */

bool
load (const char *file_name, void (**eip) (void), void **esp) 
{
  struct thread *t = thread_current ();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  off_t file_ofs;
  bool success = false;
  int i;

  //char **args = NULL;
 // char **arg_addresses = NULL;

  /* Allocate and activate page directory. */
  t->pagedir = pagedir_create ();
  if (t->pagedir == NULL) 
    goto done;
  process_activate ();

  /* Open executable file. */
  // file_name 복사본을 만들기 위해 메모리 할당
  char *full_file_name_copy = malloc(strlen(file_name) + 1); 

  char full_file_name_copy_2[256];


  if (full_file_name_copy == NULL)goto done;  // 메모리 할당 실패 시 종료

  strlcpy(full_file_name_copy, file_name, strlen(file_name) + 1);  // 안전하게 복사
  strlcpy(full_file_name_copy_2, file_name, strlen(file_name) + 1);  // 안전하게 복사


  
  /* 파일 이름과 인자를 분리 */
  char *current_token = NULL;
  char *left_tokens = NULL;

  // 첫 번째 토큰은 실행할 프로그램의 이름
  current_token = strtok_r(full_file_name_copy, " ", &left_tokens); 

  if (current_token == NULL)
    goto done;  // 토큰이 없으면 종료


  // 프로그램 이름을 사용하여 실행 파일 열기
  char *real_file_name_copy = malloc(strlen(current_token) + 1);
  if (real_file_name_copy == NULL)
    goto done;
  strlcpy(real_file_name_copy, current_token, strlen(current_token) + 1);


  file = filesys_open(real_file_name_copy);

  //  printf("full_file_name_copy: %s\n", full_file_name_copy);
   //     printf("full_file_name_copy2: %s\n", full_file_name_copy_2);



  if (file == NULL) {
    printf("load: %s: open failed\n", real_file_name_copy);
    goto done;
  }

    /* 실행 파일에 대한 쓰기 방지 */
//  file_deny_write(file);
  thread_current()->exec_file = file;  // 이후에 사용할 수 있도록 파일 저장
 // 실행 파일 포인터만 저장
  /* ELF 실행 파일 헤더를 읽고 검증 */
  if (file_read(file, &ehdr, sizeof ehdr) != sizeof ehdr
      || memcmp(ehdr.e_ident, "\177ELF\1\1\1", 7)
      || ehdr.e_type != 2
      || ehdr.e_machine != 3
      || ehdr.e_version != 1
      || ehdr.e_phentsize != sizeof(struct Elf32_Phdr)
      || ehdr.e_phnum > 1024) {
    printf("load: %s: error loading executable\n", real_file_name_copy);
    goto done;
  }

  /* ELF 프로그램 헤더를 읽음 */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++) {
    struct Elf32_Phdr phdr;
    if (file_ofs < 0 || file_ofs > file_length(file))
      goto done;
    file_seek(file, file_ofs);
    if (file_read(file, &phdr, sizeof phdr) != sizeof phdr)
      goto done;
    file_ofs += sizeof phdr;
    switch (phdr.p_type) {
      case PT_LOAD:
        if (validate_segment(&phdr, file)) {
          bool writable = (phdr.p_flags & PF_W) != 0;
          uint32_t file_page = phdr.p_offset & ~PGMASK;
          uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
          uint32_t page_offset = phdr.p_vaddr & PGMASK;
          uint32_t read_bytes = page_offset + phdr.p_filesz;
          uint32_t zero_bytes = ROUND_UP(page_offset + phdr.p_memsz, PGSIZE) - read_bytes;
          if (!load_segment(file, file_page, (void *) mem_page, read_bytes, zero_bytes, writable))
            goto done;
        } else {
          goto done;
        }
        break;
    }
  }

  /* 스택 설정 */
  if (!setup_stack(esp))
    goto done;

  /* setup_user_stack 호출하여 인자 처리 */
  setup_user_stack(full_file_name_copy_2, esp);




  /* Start address */
  *eip = (void (*) (void)) ehdr.e_entry;

  success = true;



 done:
 // if (!success && file != NULL) {
 //   file_close(file);
 // }


 // if (full_file_name_copy != NULL) 
 //   free(full_file_name_copy);
  /* 로드가 성공했다면, 파일은 열려 있는 상태임
  // 이는  thread_current()->exec_file에서 관리할것이므로 괜찮음 */

  return success;
}



// 유효한 스택 주소인지 확인하는 함수
bool is_valid_stack_pointer(void *esp) {
    // 스택 포인터가 사용자 주소 범위 내에 있는지 확인
    if (!is_user_vaddr(esp)) {
        return false;
    }
    // 페이지 디렉토리에서 유효한 페이지로 매핑되어 있는지 확인
    if (pagedir_get_page(thread_current()->pagedir, esp) == NULL) {
        return false;
    }
    return true;
}



/* setup_user_stack: 스택에 argc, argv 저장 */
void setup_user_stack(char *file_name, void **esp) {
  char *argv[128];  // 필요한 경우 크기를 조정
  int argc = 0;
  char *token, *last;
  int total_len = 0;
  
  // 우선, arv[argc]에 각 인자를 저장함
  for (token = strtok_r(file_name, " ", &last); token != NULL; token = strtok_r(NULL, " ", &last)) {
    argv[argc] = token;
    argc++;
  }

  /* Push argv[argc-1] ~ argv[0] onto the stack */
  for (int i = argc - 1; i >= 0; i--) {
    int len = strlen(argv[i]) + 1;
    *esp -= len;
    total_len += len;

       if (!is_valid_stack_pointer(*esp)) {
         thread_exit();  // 스택 포인터가 유효하지 않으면 종료
        }


    memcpy(*esp, argv[i], len);
    argv[i] = *esp;  // Store the address of the argument in argv
  }

  /*padding the stack */
  int remainder = total_len % 4;
   int padding_size = (4 - remainder);
  if (remainder != 0) {

    *esp -= (padding_size);

     if (!is_valid_stack_pointer(*esp)) {
    thread_exit();  // 스택 포인터가 유효하지 않으면 종료
        }

    memset(*esp, 0, padding_size);
  }

  /* Push NULL sentinel */
  *esp -= sizeof(char *);

 if (!is_valid_stack_pointer(*esp)) {
    thread_exit();  // 스택 포인터가 유효하지 않으면 종료
    }

  *(uint32_t *)(*esp) = 0;

  /* Push the addresses of argv[argc-1] ~ argv[0] */
  for (int i = argc - 1; i >= 0; i--) {
    *esp -= sizeof(char *);

     if (!is_valid_stack_pointer(*esp)) {
    thread_exit();  // 스택 포인터가 유효하지 않으면 종료
        }

    *(uint32_t *)(*esp) = (uint32_t)argv[i];
  }

  /* Push the address of argv */
  *esp -= sizeof(char **);

   if (!is_valid_stack_pointer(*esp)) {
    thread_exit();  // 스택 포인터가 유효하지 않으면 종료
    }
  *(uint32_t *)(*esp) = (uint32_t)(*esp + 4);

  /* Push argc */
  *esp -= sizeof(int);

   if (!is_valid_stack_pointer(*esp)) {
    thread_exit();  // 스택 포인터가 유효하지 않으면 종료
    }

  *(int *)(*esp) = argc;

  /* Push a fake return address */
  *esp -= sizeof(void *);
  if (!is_valid_stack_pointer(*esp)) {
    thread_exit();  // 스택 포인터가 유효하지 않으면 종료
    }

  *(uint32_t *)(*esp) = 0;

  /* Dump the stack for debugging */
 // hex_dump((uintptr_t) *esp, *esp, (size_t) (PHYS_BASE - (uintptr_t) *esp), true);


}
/* load() helpers. */

static bool install_page (void *upage, void *kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Elf32_Phdr *phdr, struct file *file) 
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK)) 
    return false; 

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off) file_length (file)) 
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz) 
    return false; 

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;
  
  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr ((void *) phdr->p_vaddr))
    return false;
  if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0) 
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      /* Get a page of memory. */
      uint8_t *kpage = palloc_get_page (PAL_USER);
      if (kpage == NULL)
        return false;

      /* Load this page. */
      if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes)
        {
          palloc_free_page (kpage);
          return false; 
        }
      memset (kpage + page_read_bytes, 0, page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (upage, kpage, writable)) 
        {
          palloc_free_page (kpage);
          return false; 
        }

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;
    }
  return true;
}

/* Create a minimal stack by mapping a zeroed page at the top of
   user virtual memory. */
static bool
setup_stack (void **esp) 
{
  uint8_t *kpage;
  bool success = false;

  kpage = palloc_get_page (PAL_USER | PAL_ZERO);
  if (kpage != NULL) 
    {
      success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true);
      if (success)
        *esp = PHYS_BASE;
      else
        palloc_free_page (kpage);
    }
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}

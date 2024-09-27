#include <syscall.h>
#include "../syscall-nr.h"

/* Invokes syscall NUMBER, passing no arguments, and returns the
   return value as an `int'. */
#define syscall0(NUMBER)                                        \
        ({                                                      \
          int retval;                                           \
          asm volatile                                          \
            ("pushl %[number]; int $0x30; addl $4, %%esp"       \
               : "=a" (retval)                                  \
               : [number] "i" (NUMBER)                          \
               : "memory");                                     \
          retval;                                               \
        })

/* Invokes syscall NUMBER, passing argument ARG0, and returns the
   return value as an `int'. */
#define syscall1(NUMBER, ARG0)                                           \
        ({                                                               \
          int retval;                                                    \
          asm volatile                                                   \
            ("pushl %[arg0]; pushl %[number]; int $0x30; addl $8, %%esp" \
               : "=a" (retval)                                           \
               : [number] "i" (NUMBER),                                  \
                 [arg0] "g" (ARG0)                                       \
               : "memory");                                              \
          retval;                                                        \
        })

/* Invokes syscall NUMBER, passing arguments ARG0 and ARG1, and
   returns the return value as an `int'. */
#define syscall2(NUMBER, ARG0, ARG1)                            \
        ({                                                      \
          int retval;                                           \
          asm volatile                                          \
            ("pushl %[arg1]; pushl %[arg0]; "                   \
             "pushl %[number]; int $0x30; addl $12, %%esp"      \
               : "=a" (retval)                                  \
               : [number] "i" (NUMBER),                         \
                 [arg0] "r" (ARG0),                             \
                 [arg1] "r" (ARG1)                              \
               : "memory");                                     \
          retval;                                               \
        })

/* Invokes syscall NUMBER, passing arguments ARG0, ARG1, and
   ARG2, and returns the return value as an `int'. */
#define syscall3(NUMBER, ARG0, ARG1, ARG2)                      \
        ({                                                      \
          int retval;                                           \
          asm volatile                                          \
            ("pushl %[arg2]; pushl %[arg1]; pushl %[arg0]; "    \
             "pushl %[number]; int $0x30; addl $16, %%esp"      \
               : "=a" (retval)                                  \
               : [number] "i" (NUMBER),                         \
                 [arg0] "r" (ARG0),                             \
                 [arg1] "r" (ARG1),                             \
                 [arg2] "r" (ARG2)                              \
               : "memory");                                     \
          retval;                                               \
        })



//  네 개의 인자를 받는 시스템 콜이 존재하지 않으므로, max_of_four_int 시스템 콜을 위해 
// syscall4도 추가 
/* Invokes syscall NUMBER, passing arguments ARG0, ARG1, ARG2, and ARG3,
   and returns the return value as an `int'. */
#define syscall4(NUMBER, ARG0, ARG1, ARG2, ARG3)                   \
        ({                                                         \
          int retval;                                              \
          asm volatile                                             \
            ("pushl %[arg3]; pushl %[arg2]; pushl %[arg1]; pushl %[arg0]; " \
             "pushl %[number]; int $0x30; addl $20, %%esp"         \
               : "=a" (retval)                                     \
               : [number] "i" (NUMBER),                            \
                 [arg0] "r" (ARG0),                                \
                 [arg1] "r" (ARG1),                                \
                 [arg2] "r" (ARG2),                                \
                 [arg3] "r" (ARG3)                                 \
               : "memory");                                        \
          retval;                                                  \
        })



// 플젝 1 - 0
void
halt (void) 
{
  syscall0 (SYS_HALT);
  NOT_REACHED ();
}

// 플젝 1 -1 
void
exit (int status)
{
  syscall1 (SYS_EXIT, status);
  NOT_REACHED ();
}

// 플젝 1 -2
pid_t
exec (const char *file)
{
  return (pid_t) syscall1 (SYS_EXEC, file);
}

// 플젝 1 -3 
int
wait (pid_t pid)
{
  return syscall1 (SYS_WAIT, pid);
}

// 4 
bool
create (const char *file, unsigned initial_size)
{
  return syscall2 (SYS_CREATE, file, initial_size);
}

// 5 
bool
remove (const char *file)
{
  return syscall1 (SYS_REMOVE, file);
}

// 6 
int
open (const char *file)
{
  return syscall1 (SYS_OPEN, file);
}

// 7
int
filesize (int fd) 
{
  return syscall1 (SYS_FILESIZE, fd);
}

// 플젝 1 - 8 
int
read (int fd, void *buffer, unsigned size)
{
  return syscall3 (SYS_READ, fd, buffer, size);
}

// 플젝 1 -9 
int
write (int fd, const void *buffer, unsigned size)
{
  return syscall3 (SYS_WRITE, fd, buffer, size);
}

// 10 
void
seek (int fd, unsigned position) 
{
  syscall2 (SYS_SEEK, fd, position);
}

// 11
unsigned
tell (int fd) 
{
  return syscall1 (SYS_TELL, fd);
}

// 12 
void
close (int fd)
{
  syscall1 (SYS_CLOSE, fd);
}

// 13 
int
fibonacci (int n)
{
  return syscall1 (SYS_FIBONACCI, n);
}

// 14 
int
max_of_four_int (int a, int b, int c, int d)
{
  return syscall4 (SYS_MAX_OF_FOUR_INT, a, b, c, d);
}


// 프로젝트 3,4,라고 함!! - 지금 아님 - 유저 프로그램2까지 고려해서 여기서 끊장!! 
mapid_t
mmap (int fd, void *addr)
{
  return syscall2 (SYS_MMAP, fd, addr);
}

//
void
munmap (mapid_t mapid)
{
  syscall1 (SYS_MUNMAP, mapid);
}

bool
chdir (const char *dir)
{
  return syscall1 (SYS_CHDIR, dir);
}

bool
mkdir (const char *dir)
{
  return syscall1 (SYS_MKDIR, dir);
}

bool
readdir (int fd, char name[READDIR_MAX_LEN + 1]) 
{
  return syscall2 (SYS_READDIR, fd, name);
}

bool
isdir (int fd) 
{
  return syscall1 (SYS_ISDIR, fd);
}

int
inumber (int fd) 
{
  return syscall1 (SYS_INUMBER, fd);
}

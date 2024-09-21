#ifndef __LIB_SYSCALL_NR_H
#define __LIB_SYSCALL_NR_H

/* System call numbers. */
enum 
  {
    /* Projects 2 and later. */
    SYS_HALT,                   /* 0번 Halt the operating system. */
    SYS_EXIT,                   /* 1번 Terminate this process. */
    SYS_EXEC,                   /* 2번 Start another process. */
    SYS_WAIT,                   /* 3번 Wait for a child process to die. */
    SYS_CREATE,                 /* 4번 Create a file. */
    SYS_REMOVE,                 /* 5번 Delete a file. */
    SYS_OPEN,                   /* 6번 Open a file. */
    SYS_FILESIZE,               /* 7번 Obtain a file's size. */
    SYS_READ,                   /* 8번 Read from a file. */
    SYS_WRITE,                  /* 9번 Write to a file. */
    SYS_SEEK,                   /* 10번 Change position in a file. */
    SYS_TELL,                   /* 11번 Report current position in a file. */
    SYS_CLOSE,                  /* 12번 Close a file. */
    // 추가적인 시스템 콜 !! 
    SYS_FIBONACCI,              /* 13번 피보나치의 수 계산 */
    SYS_MAX_OF_FOUR_INT,        /* 14번 4개의 수 중 가장 큰 것 리턴 */

    /* Project 3 and optionally project 4. */
    SYS_MMAP,                   /* Map a file into memory. */
    SYS_MUNMAP,                 /* Remove a memory mapping. */

    /* Project 4 only. */
    SYS_CHDIR,                  /* Change the current directory. */
    SYS_MKDIR,                  /* Create a directory. */
    SYS_READDIR,                /* Reads a directory entry. */
    SYS_ISDIR,                  /* Tests if a fd represents a directory. */
    SYS_INUMBER                 /* Returns the inode number for a fd. */
  };

#endif /* lib/syscall-nr.h */

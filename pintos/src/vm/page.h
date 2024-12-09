#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stdbool.h>
#include <stddef.h>
#include "filesys/off_t.h"
#include "threads/thread.h"
#include "userprog/supplemental_page_table.h"

/* 여기는 vm/page.c에서 구현하는 VM 관련 함수들의 헤더만 둡니다.
   enum page_type나 spt 관련 함수는 supplemental_page_table.h에서 관리하므로 여기서는 중복 정의하지 않음. */

/* grow_stack, page_install_file 등의 VM 관련 함수 프로토타입만 남긴다. */
//struct supplemental_page_table_entry *page_lookup(struct hash *spt, void *addr); 

//struct supplemental_page_table_entry *
//page_lookup(struct hash *spt, void *addr);
struct supplemental_page_table_entry *page_lookup(struct supplemental_page_table *spt, void *addr);

/* 스택 확장 함수 */
bool grow_stack(void *fault_addr);

/* Lazy Loading용 함수: page_install_file 등 */
//bool page_install_file(struct supplemental_page_table *spt, struct file *file, off_t ofs,
  //                     void *upage, uint32_t read_bytes, uint32_t zero_bytes, bool writable);

//bool page_install_file(struct supplemental_page_table *spt, struct file *file, off_t ofs,
  //                     void *upage, uint32_t read_bytes, uint32_t zero_bytes, bool writable);


bool page_install_file(struct supplemental_page_table *spt, struct file *file, off_t ofs,
                       void *upage, uint32_t read_bytes, uint32_t zero_bytes, bool writable);
bool page_install_zero(struct supplemental_page_table *spt, void *upage, bool writable);
#endif /* VM_PAGE_H */

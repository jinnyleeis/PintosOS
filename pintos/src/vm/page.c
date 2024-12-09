#include "vm/page.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"           // 추가됨: PHYS_BASE, PGSIZE, pg_round_down을 인식하기 위함
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include <string.h>
#include <debug.h>
#include <stdio.h>
#include "userprog/supplemental_page_table.h"

/* spt_init, spt_destroy, page_lookup 함수는 supplemental_page_table.c에서 이미 정의되었으므로 제거 */

/*
static unsigned page_hash_func(const struct hash_elem *e, void *aux UNUSED) {
  const struct supplemental_page_table_entry *spte = hash_entry(e, struct supplemental_page_table_entry, hash_elem);
  return hash_bytes(&spte->upage,sizeof spte->upage);
}

static bool page_less_func(const struct hash_elem *a,
                           const struct hash_elem *b, void *aux UNUSED) {
  const struct supplemental_page_table_entry *sa = hash_entry(a, struct supplemental_page_table_entry, hash_elem);
  const struct supplemental_page_table_entry *sb = hash_entry(b, struct supplemental_page_table_entry, hash_elem);
  return sa->upage < sb->upage;
}

void spt_init(struct hash *spt) {
  hash_init(spt,page_hash_func,page_less_func,NULL);
}

static void spt_destroy_func(struct hash_elem *e, void *aux UNUSED) {
  struct supplemental_page_table_entry *spte = hash_entry(e,struct supplemental_page_table_entry,hash_elem);
  free(spte);
}

void spt_destroy(struct hash *spt) {
  hash_destroy(spt,spt_destroy_func);
}

struct supplemental_page_table_entry *
page_lookup(struct hash *spt, void *addr) {
  struct supplemental_page_table_entry spte_tmp;
  spte_tmp.upage = pg_round_down(addr);
  struct hash_elem *e = hash_find(spt,&spte_tmp.hash_elem);
  if (e != NULL)
    return hash_entry(e, struct supplemental_page_table_entry, hash_elem);
  return NULL;
}
*/

/* 페이지 설치 함수들만 남김 */

/* 파일을 기반으로 페이지 설치 */
bool
page_install_file(struct supplemental_page_table *spt, struct file *file, off_t ofs,
                  void *upage, uint32_t read_bytes, uint32_t zero_bytes,
                  bool writable) {
  struct supplemental_page_table_entry *spte = malloc(sizeof(struct supplemental_page_table_entry));
  if (!spte) return false;

  spte->upage = upage;
  spte->type = PAGE_FILE;  // supplemental_page_table.h에 enum page_type = PAGE_FILE 등 정의 필요
  spte->writable = writable;
  spte->is_loaded = false;
  spte->file = file;
  spte->offset = ofs;
  spte->read_bytes = read_bytes;
  spte->zero_bytes = zero_bytes;
  spte->swap_slot = (size_t)-1;

  // spt->pages 해시에 삽입
  if (hash_insert(&spt->pages, &spte->hash_elem) != NULL) {
    free(spte);
    return false;
  }

  return true;
}

/* 제로 페이지 설치 */
bool
page_install_zero(struct supplemental_page_table *spt, void *upage, bool writable) {
  struct supplemental_page_table_entry *spte = malloc(sizeof *spte);
  if (!spte) return false;
  spte->upage = upage;
  spte->type = PAGE_ZERO;
  spte->writable = writable;
  spte->is_loaded = false;
  spte->file = NULL;
  spte->swap_slot = (size_t)-1;

  // spt->pages 해시에 삽입
  if (hash_insert(&spt->pages, &spte->hash_elem) != NULL) {
    free(spte);
    return false;
  }

  return true;
}

struct supplemental_page_table_entry *
page_lookup(struct supplemental_page_table *spt, void *addr) {
  void *upage = pg_round_down(addr);
  struct supplemental_page_table_entry tmp;
  tmp.upage = upage;

  lock_acquire(&spt->spt_lock);
  struct hash_elem *e = hash_find(&spt->pages, &tmp.hash_elem);
  struct supplemental_page_table_entry *spte = NULL;
  if (e != NULL)
    spte = hash_entry(e, struct supplemental_page_table_entry, hash_elem);
  lock_release(&spt->spt_lock);
  return spte;
}


/* 스택 확장 함수 */
/*
bool grow_stack(void *fault_addr) {
  if ((size_t)(PHYS_BASE - pg_round_down(fault_addr)) > 8 * 1024 * 1024)
    return false;
  struct thread *t = thread_current();
  return page_install_zero(&t->spt, pg_round_down(fault_addr), true);
}*/

/* 페이지 로드 함수 */
/*
bool page_load(struct supplemental_page_table_entry *spte) {
  void *kpage = frame_alloc(PAL_USER, spte->upage);
  if (!kpage) return false;
  frame_pin(kpage);

  switch(spte->type) {
    case PAGE_FILE:
      file_seek(spte->file, spte->offset);
      if (file_read(spte->file, kpage, spte->read_bytes) != (int)spte->read_bytes) {
        frame_unpin(kpage);
        frame_free(kpage);
        return false;
      }
      memset(kpage + spte->read_bytes, 0, spte->zero_bytes);
      break;
    case PAGE_ZERO:
      memset(kpage, 0, PGSIZE);
      break;
    case PAGE_SWAP:
      swap_in(spte->swap_slot, kpage);
      break;
    case PAGE_STACK:
      memset(kpage, 0, PGSIZE);
      break;
    default:
      frame_unpin(kpage);
      frame_free(kpage);
      return false;
  }

  if (!pagedir_set_page(thread_current()->pagedir, spte->upage, kpage, spte->writable)) {
    frame_unpin(kpage);
    frame_free(kpage);
    return false;
  }

  spte->is_loaded = true;
  frame_unpin(kpage);
  return true;
}
*/

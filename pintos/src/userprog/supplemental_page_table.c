// userprog/supplemental_page_table.c

#include "userprog/supplemental_page_table.h"
#include "vm/page.h" // page_install_zero 함수를 사용하기 위해 추가
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "filesys/file.h"
#include "threads/palloc.h"
#include <string.h>

/* spt_hash_func 구현 */
unsigned spt_hash_func(const struct hash_elem *e, void *aux UNUSED) {
    const struct supplemental_page_table_entry *spte = hash_entry(e, struct supplemental_page_table_entry, hash_elem);
    return hash_bytes(&spte->upage, sizeof(spte->upage));
}

/* spt_less_func 구현 */
bool spt_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
    const struct supplemental_page_table_entry *spte_a = hash_entry(a, struct supplemental_page_table_entry, hash_elem);
    const struct supplemental_page_table_entry *spte_b = hash_entry(b, struct supplemental_page_table_entry, hash_elem);
    return spte_a->upage < spte_b->upage;
}

/* supplemental_page_table.c에서 사용될 destroy 함수 */
static void spt_destroy_func(struct hash_elem *e, void *aux UNUSED) {
    struct supplemental_page_table_entry *spte = hash_entry(e, struct supplemental_page_table_entry, hash_elem);
    free(spte);
}

void spt_init(struct supplemental_page_table *spt) {
    hash_init(&spt->pages, spt_hash_func, spt_less_func, NULL);
    lock_init(&spt->spt_lock);
}

void spt_destroy(struct supplemental_page_table *spt) {
    lock_acquire(&spt->spt_lock);
    hash_destroy(&spt->pages, spt_destroy_func);
    lock_release(&spt->spt_lock);
}

struct supplemental_page_table_entry *page_lookup(struct supplemental_page_table *spt, void *addr) {
    void *upage = pg_round_down(addr);
    struct supplemental_page_table_entry tmp;
    tmp.upage = upage;

    lock_acquire(&spt->spt_lock);
    struct hash_elem *e = hash_find(&spt->pages, &tmp.hash_elem);
    struct supplemental_page_table_entry *spte = NULL;
    if (e != NULL)
        spte = hash_entry(e, struct supplemental_page_table_entry, hash_elem); // 오타 수정
    lock_release(&spt->spt_lock);
    return spte;
}


bool page_load(struct supplemental_page_table_entry *spte) {
    void *upage = pg_round_down(spte->upage);

    // 기존: palloc_get_page() -> 수정: frame_alloc() 사용
    void *kpage = frame_alloc(PAL_USER, upage);
    if (!kpage)
        return false;
    frame_pin(kpage); // 로딩 중 프레임 pin

    if (spte->file) {
        file_seek(spte->file, spte->offset);
        if (file_read(spte->file, kpage, spte->read_bytes) != (int) spte->read_bytes) {
            frame_unpin(kpage);
            frame_free(kpage);
            return false;
        }
        memset(kpage + spte->read_bytes, 0, spte->zero_bytes);
    } else {
        memset(kpage, 0, PGSIZE);
    }

    if (!pagedir_set_page(thread_current()->pagedir, upage, kpage, spte->writable)) {
        frame_unpin(kpage);
        frame_free(kpage);
        return false;
    }

    spte->is_loaded = true;
    frame_unpin(kpage);
    return true;
}


/*
bool grow_stack(void *fault_addr) {
    struct supplemental_page_table *spt = &thread_current()->spt;
    void *upage = pg_round_down(fault_addr);

    if ((size_t)(PHYS_BASE - upage) > 8 * 1024 * 1024)
        return false;

    lock_acquire(&spt->spt_lock);
    bool success = page_install_zero(spt, upage, true);
    lock_release(&spt->spt_lock);
    return success;
}
*/

bool
grow_stack(void *fault_addr, void *esp) {
  if ((uint8_t*)PHYS_BASE - (uint8_t*)fault_addr > 8*1024*1024)
    return false;
  if (fault_addr >= PHYS_BASE)
    return false;
  // stack page install
  struct supplemental_page_table *spt = &thread_current()->spt;
  lock_acquire(&spt->spt_lock);
  bool ok = page_install_zero(spt, pg_round_down(fault_addr), true);
  lock_release(&spt->spt_lock);
  return ok;
}

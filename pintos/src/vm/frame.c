#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "vm/swap.h"
#include <list.h>
#include <stdio.h>
#include "vm/page.h"




#include "userprog/supplemental_page_table.h" // page_lookup, enum page_type 사용 위해 추가

static struct list frame_table;
static struct lock frame_lock;
static struct list_elem *clock_ptr;

void frame_init(void) {
  list_init(&frame_table);
  lock_init(&frame_lock);
  clock_ptr = NULL;
}

static struct frame_table_entry* frame_find_victim(void) {
  if (list_empty(&frame_table))
    return NULL;
  if (clock_ptr == NULL || clock_ptr==list_end(&frame_table))
    clock_ptr = list_begin(&frame_table);

  while(true) {
    struct frame_table_entry *fte = list_entry(clock_ptr, struct frame_table_entry, elem);
    if(!fte->pinned) {
      if(!pagedir_is_accessed(fte->t->pagedir, fte->upage)){
        return fte;
      } else {
        pagedir_set_accessed(fte->t->pagedir, fte->upage,false);
      }
    }
    clock_ptr = list_next(clock_ptr);
    if(clock_ptr == list_end(&frame_table))
      clock_ptr = list_begin(&frame_table);
  }
}

static void frame_evict(void) {
  struct frame_table_entry *victim = frame_find_victim();
  if (victim == NULL)
    PANIC("No victim found!");

  if (pagedir_is_dirty(victim->t->pagedir,victim->upage)) {
    size_t slot = swap_out(victim->kpage);
    struct supplemental_page_table_entry *spte = page_lookup(&victim->t->spt,victim->upage);
    spte->is_loaded = false;
    spte->type = PAGE_SWAP;
    spte->swap_slot = slot;
  } else {
    struct supplemental_page_table_entry *spte = page_lookup(&victim->t->spt,victim->upage);
    spte->is_loaded = false;
    if (spte->type == PAGE_FILE) {
      // do nothing
    } else {
      spte->type = PAGE_ZERO;
    }
  }

  pagedir_clear_page(victim->t->pagedir,victim->upage);
  palloc_free_page(victim->kpage);
  list_remove(&victim->elem);
  free(victim);
}

void *frame_alloc(enum palloc_flags flags, void *upage) {
  lock_acquire(&frame_lock);
  void *kpage = palloc_get_page(PAL_USER|flags);
  if (kpage==NULL) {
    frame_evict();
    kpage = palloc_get_page(PAL_USER|flags);
    if (kpage==NULL) {
      lock_release(&frame_lock);
      return NULL;
    }
  }
  struct frame_table_entry *fte = malloc(sizeof(struct frame_table_entry));
  fte->kpage = kpage;
  fte->t = thread_current();
  fte->upage = upage;
  fte->pinned = false;
  list_push_back(&frame_table,&fte->elem);
  lock_release(&frame_lock);
  return kpage;
}

void frame_free(void *kpage) {
  lock_acquire(&frame_lock);
  struct list_elem *e;
  for (e=list_begin(&frame_table); e!=list_end(&frame_table); e=list_next(e)) {
    struct frame_table_entry *fte = list_entry(e,struct frame_table_entry, elem);
    if (fte->kpage == kpage) {
      list_remove(&fte->elem);
      free(fte);
      break;
    }
  }
  lock_release(&frame_lock);
}

void frame_pin(void *kpage) {
  lock_acquire(&frame_lock);
  struct list_elem *e;
  for(e=list_begin(&frame_table); e!=list_end(&frame_table); e=list_next(e)){
    struct frame_table_entry *fte=list_entry(e,struct frame_table_entry,elem);
    if (fte->kpage==kpage) {
      fte->pinned=true;
      break;
    }
  }
  lock_release(&frame_lock);
}

void frame_unpin(void *kpage) {
  lock_acquire(&frame_lock);
  struct list_elem *e;
  for(e=list_begin(&frame_table); e!=list_end(&frame_table); e=list_next(e)){
    struct frame_table_entry *fte=list_entry(e,struct frame_table_entry,elem);
    if (fte->kpage==kpage) {
      fte->pinned=false;
      break;
    }
  }
  lock_release(&frame_lock);
}

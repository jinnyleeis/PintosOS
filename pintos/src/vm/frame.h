#ifndef VM_FRAME_H
#define VM_FRAME_H
#include "threads/palloc.h"
#include "threads/synch.h"
#include "lib/kernel/list.h"

struct frame_table_entry {
  void *kpage;
  struct thread *t;
  void *upage;
  bool pinned;
  struct list_elem elem;
};

void frame_init(void);
void *frame_alloc(enum palloc_flags flags, void *upage);
void frame_free(void *kpage);
void frame_pin(void *kpage);
void frame_unpin(void *kpage);

#endif


// userprog/supplemental_page_table.h

#ifndef USERPROG_SUPPLEMENTAL_PAGE_TABLE_H
#define USERPROG_SUPPLEMENTAL_PAGE_TABLE_H

#include <stdbool.h>
#include <hash.h>           // struct hash 사용 위해 필요
#include "threads/synch.h"  // struct lock 사용 위해 필요
#include "filesys/file.h"   // struct file 사용 위해 필요

enum page_type {
    PAGE_FILE,
    PAGE_ZERO,
    PAGE_SWAP,
    PAGE_STACK,
    PAGE_MMAPPED
};

struct supplemental_page_table_entry {
    void *upage;
    enum page_type type;
    bool writable;
    bool is_loaded;

    struct file *file;
    off_t offset;
    size_t read_bytes;
    size_t zero_bytes;
    size_t swap_slot;

    struct hash_elem hash_elem;
};

struct supplemental_page_table {
    struct hash pages;
    struct lock spt_lock;
};

/* Hash function and less function for spt */
unsigned spt_hash_func(const struct hash_elem *e, void *aux);
bool spt_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux);

void spt_init(struct supplemental_page_table *spt);
void spt_destroy(struct supplemental_page_table *spt);
//struct supplemental_page_table_entry *page_lookup(struct supplemental_page_table *spt, void *addr);
bool page_load(struct supplemental_page_table_entry *spte);
bool grow_stack(void *fault_addr);

#endif /* USERPROG_SUPPLEMENTAL_PAGE_TABLE_H */

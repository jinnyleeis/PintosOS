#ifndef VM_SWAP_H
#define VM_SWAP_H
#include <stddef.h>

void swap_init(void);
size_t swap_out(void *kpage);
void swap_in(size_t idx, void *kpage);

#endif

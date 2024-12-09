#include "vm/swap.h"
#include "devices/block.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "bitmap.h"
#include <stdio.h>

static struct block *swap_block;
static struct bitmap *swap_map;
static struct lock swap_lock;

#define SECTORS_PER_PAGE (PGSIZE/BLOCK_SECTOR_SIZE)

void swap_init(void) {
  swap_block = block_get_role(BLOCK_SWAP);
  if(!swap_block) return;
  size_t swap_size = block_size(swap_block)/SECTORS_PER_PAGE;
  swap_map = bitmap_create(swap_size);
  bitmap_set_all(swap_map,true);
  lock_init(&swap_lock);
}

size_t swap_out(void *kpage) {
  lock_acquire(&swap_lock);
  size_t idx = bitmap_scan_and_flip(swap_map,0,1,true);
  if (idx==BITMAP_ERROR) {
    PANIC("No swap space!");
  }
  for(size_t i=0;i<SECTORS_PER_PAGE;i++){
    block_write(swap_block,idx*SECTORS_PER_PAGE+i,(uint8_t*)kpage+i*BLOCK_SECTOR_SIZE);
  }
  lock_release(&swap_lock);
  return idx;
}

void swap_in(size_t idx, void *kpage) {
  lock_acquire(&swap_lock);
  for(size_t i=0;i<SECTORS_PER_PAGE;i++){
    block_read(swap_block,idx*SECTORS_PER_PAGE+i,(uint8_t*)kpage+i*BLOCK_SECTOR_SIZE);
  }
  bitmap_flip(swap_map,idx);
  lock_release(&swap_lock);
}

/* CS503 Spring 2017 - Lab 4 */

#ifndef __LAB3_H_
#define __LAB3_H_

#include <paging.h>

typedef struct {
  int16 alloc_page_proc[NFRAMES];
  uint32 alloc_page[NFRAMES];
  int16 curframe;
  int16 reclaimframe;
} frame_md_t;

extern frame_md_t frame_md;

// Hooks, in hooks.c
void hook_ptable_create(uint32 pagenum);
void hook_ptable_delete(uint32 pagenum);

// changed by adil
void hook_pfault(int16 procid, void *addr, uint32 pagenum, uint32 framenum);

// changed by adil
void hook_pswap_out(int16 procid, uint32 pagenum, uint32 framenum);

/* Add more definitions here if necessary */

#endif // __LAB4_H_

#ifndef _QDEBUG_H
#define _QDEBUG_H

#define QEMUDEBUG 1

extern bool8 xinu_debug_flag;

#ifdef QEMUDEBUG
#define dprintf(...) do {                               \
        if (xinu_debug_flag) {                          \
            kprintf("[%s:%d] ", __FILE__, __LINE__);    \
            kprintf( __VA_ARGS__ );                     \
        }                                               \
    } while (0);
#else
#define dprintf(...)
#endif

#define ASSERT(CONDITION)                       \
  do {                                          \
    if (CONDITION) { }                          \
    else {                                      \
      panic("ASERT FAIL\n");                    \
    }                                           \
  } while(0);

#ifdef QEMU
#define BREAKPOINT() __asm("int $3");
#else
#define BREAKPOINT()
#endif

#endif

#ifndef DEVICES_XINU_SERIAL_H
#define DEVICES_XINU_SERIAL_H

#include <xinu.h>

typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint32 uint64_t;
typedef int32 int64_t;

typedef bool8 bool;

#define true 1
#define false 0

/* EFLAGS Register. */
#define FLAG_IF   0x00000200    /* Interrupt Flag. */

/* Interrupts on or off? */
enum intr_level
{
  INTR_OFF,             /* Interrupts disabled. */
  INTR_ON               /* Interrupts enabled. */
};

enum intr_level intr_get_level (void);

#define UNUSED __attribute__ ((unused))

#endif

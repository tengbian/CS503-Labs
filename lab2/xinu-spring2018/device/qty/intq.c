#include "intq.h"

static int next (int pos);

/* Initializes interrupt queue Q. */
void
intq_init (struct intq *q)
{
  q->sem = semcreate(0);
  q->head = q->tail = 0;
  q->count_getc = 0;
  q->count_putc = 0;
}

/* Returns true if Q is empty, false otherwise. */
bool
intq_empty (const struct intq *q)
{
  ASSERT (intr_get_level () == INTR_OFF);
  return q->head == q->tail;
}

/* Returns true if Q is full, false otherwise. */
bool
intq_full (const struct intq *q)
{
  ASSERT (intr_get_level () == INTR_OFF);
  return next (q->head) == q->tail;
}

/* Removes a byte from Q and returns it.
   If Q is empty, sleeps until a byte is added.
   When called from an interrupt handler, Q must not be empty. */
uint8_t
intq_getc (struct intq *q)
{
  uint8_t byte;

  ASSERT (intr_get_level () == INTR_OFF);

  if (intq_empty(q)) {
    // dprintf("intq_getc (%s): wait due to full (%d)\n",
    //         q->name, semcount(q->sem));
    wait(q->sem);
  }

  byte = q->buf[q->tail];
  q->tail = next (q->tail);

  if (semcount(q->sem) < 0) {
    // dprintf("intq_getc (%s): signal(%d) \n",
    //         q->name, semcount(q->sem));
    signaln(q->sem, 1);
  }
  q->count_getc++;
  return byte;
}

/* Adds BYTE to the end of Q.
   If Q is full, sleeps until a byte is removed.
   When called from an interrupt handler, Q must not be full. */
void
intq_putc (struct intq *q, uint8_t byte)
{
  ASSERT (intr_get_level () == INTR_OFF);

  if (intq_full(q)) {
    // dprintf("intq_putc (%s): wait due to full (%d)\n",
    //         q->name, semcount(q->sem));
    wait(q->sem);
  }

  q->buf[q->head] = byte;
  q->head = next (q->head);

  if (semcount(q->sem) < 0) {
    // dprintf("intq_putc (%s): signal (%d) \n",
    //         q->name, semcount(q->sem));
    signaln(q->sem, 1);
  }
  q->count_putc++;
}

/* Returns the position after POS within an intq. */
static int
next (int pos)
{
  return (pos + 1) % INTQ_BUFSIZE;
}


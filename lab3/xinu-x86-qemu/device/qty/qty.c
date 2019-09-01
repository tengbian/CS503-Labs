#include "qty.h"

/* Returns the current interrupt status. */
enum intr_level
intr_get_level (void)
{
  uint32_t flags;

  /* Push the flags register on the processor stack, then pop the
     value off the stack into `flags'.  See [IA32-v2b] "PUSHF"
     and "POP" and [IA32-v3a] 5.8.1 "Masking Maskable Hardware
     Interrupts". */
  asm volatile ("pushfl; popl %0" : "=g" (flags));
  return flags & FLAG_IF ? INTR_ON : INTR_OFF;
}


devcall	qty_control(
  struct dentry	*devptr,	/* Entry in device switch table	*/
  int32	 func,			/* Function to perform		*/
  int32	 arg1,			/* Argument 1 for request	*/
  int32	 arg2			/* Argument 2 for request	*/
  ) {
  panic("qty_control (not implemented)\n");
  return (devcall)SYSERR;
}


devcall	qty_getc(
  struct dentry	*devptr		/* Entry in device switch table	*/
  ) {
  return input_getc();
}

void qty_handler(
  int32	arg	/* Interrupt handler argument	*/
  ) {
  serial_interrupt(arg);
  return;
}

devcall	qty_init(
	  struct dentry	*devptr		/* Entry in device switch table	*/
	)
{
  input_init();
  serial_init_queue();
  return OK;
}


devcall	qty_putc(
	struct	dentry	*devptr,	/* Entry in device switch table	*/
	char	ch			/* Character to write		*/
	)
{
  serial_putc(ch);
  if (ch == '\n')
    serial_putc('\r');
  return OK;
}

devcall	qty_read(
	  struct dentry	*devptr,	/* Entry in device switch table	*/
	  char          *buffer,	/* Buffer of characters		*/
	  int32	size                /* Count of character to read	*/
	)
{
  uint32 status = 0;
  uint8_t c;

  unsigned counter = size;
  uint8_t *buf = buffer;

  c = input_getc();

  *buf++ = c;
  counter--;

  while ((counter > 1) && (c != TY_NEWLINE) && (c != TY_RETURN) ) {
    c = input_getc();
    *buf++ = c;
    counter--;
  }
  status = size - counter;
  return status;
}


devcall	qty_write(
	  struct dentry	*devptr,	/* Entry in device switch table	*/
	  char	*buff,			/* Buffer of characters		*/
	  int32	count 			/* Count of character to write	*/
	)
{
  for (int i=0; i<count; i++)
    qty_putc(devptr, buff[i]);

  return OK;
}

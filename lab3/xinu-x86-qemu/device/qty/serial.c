#include "serial.h"
#include "input.h"
#include "intq.h"

/* Register definitions for the 16550A UART used in PCs.
   The 16550A has a lot more going on than shown here, but this
   is all we need.

   Refer to [PC16650D] for hardware information. */

/* I/O port base address for the first serial port. */
#define IO_BASE 0x3f8

/* DLAB=0 registers. */
#define RBR_REG (IO_BASE + 0)   /* Receiver Buffer Reg. (read-only). */
#define THR_REG (IO_BASE + 0)   /* Transmitter Holding Reg. (write-only). */
#define IER_REG (IO_BASE + 1)   /* Interrupt Enable Reg.. */

/* DLAB=1 registers. */
#define LS_REG (IO_BASE + 0)    /* Divisor Latch (LSB). */
#define MS_REG (IO_BASE + 1)    /* Divisor Latch (MSB). */

/* DLAB-insensitive registers. */
#define IIR_REG (IO_BASE + 2)   /* Interrupt Identification Reg. (read-only) */
#define FCR_REG (IO_BASE + 2)   /* FIFO Control Reg. (write-only). */
#define LCR_REG (IO_BASE + 3)   /* Line Control Register. */
#define MCR_REG (IO_BASE + 4)   /* MODEM Control Register. */
#define LSR_REG (IO_BASE + 5)   /* Line Status Register (read-only). */

/* Interrupt Enable Register bits. */
#define IER_RECV 0x01           /* Interrupt when data received. */
#define IER_XMIT 0x02           /* Interrupt when transmit finishes. */

/* Line Control Register bits. */
#define LCR_N81 0x03            /* No parity, 8 data bits, 1 stop bit. */
#define LCR_DLAB 0x80           /* Divisor Latch Access Bit (DLAB). */

/* MODEM Control Register. */
#define MCR_OUT2 0x08           /* Output line 2. */

/* Line Status Register. */
#define LSR_DR 0x01             /* Data Ready: received data byte is in RBR. */
#define LSR_THRE 0x20           /* THR Empty. */

/* Transmission mode. */
static enum { UNINIT, POLL, QUEUE } mode;

/* Data to be transmitted. */
static struct intq txq;

static void set_serial (int bps);
static void putc_poll (uint8_t);
static void write_ier (void);

/* Initializes the serial port device for polling mode.
   Polling mode busy-waits for the serial port to become free
   before writing to it.  It's slow, but until interrupts have
   been initialized it's all we can do. */
static void
init_poll (void)
{
  ASSERT (mode == UNINIT);
  outb (IER_REG, 0);                    /* Turn off all interrupts. */
  outb (FCR_REG, 0);                    /* Disable FIFO. */
  set_serial (9600);                    /* 9.6 kbps, N-8-1. */
  outb (MCR_REG, MCR_OUT2);             /* Required to enable interrupts. */
  intq_init (&txq);
  strcpy(txq.name, "txq");
  mode = POLL;
}

/* Initializes the serial port device for queued interrupt-driven
   I/O.  With interrupt-driven I/O we don't waste CPU time
   waiting for the serial device to become ready. */
void
serial_init_queue (void)
{
  if (mode == UNINIT)
    init_poll ();
  ASSERT (mode == POLL);

  // IRQ 4 – serial port controller for serial port 1 (shared with serial port
  // 3, if present)
  // ioapic_irq2vec(4, IRQBASE+4);
  set_evec(IRQBASE+4, (uint32)qty_dispatch);

  mode = QUEUE;
  intmask mask = disable();
  write_ier ();
  restore(mask);
}

/* Sends BYTE to the serial port. */
void serial_putc (uint8_t byte) {
  putc_poll (byte);
}

/* The fullness of the input buffer may have changed.  Reassess
   whether we should block receive interrupts.
   Called by the input buffer routines when characters are added
   to or removed from the buffer. */
void
serial_notify (void)
{
  ASSERT (intr_get_level () == INTR_OFF);
  if (mode == QUEUE)
    write_ier ();
}

/* Configures the serial port for BPS bits per second. */
static void
set_serial (int bps)
{
  int base_rate = 1843200 / 16;         /* Base rate of 16550A, in Hz. */
  uint16_t divisor = base_rate / bps;   /* Clock rate divisor. */

  ASSERT (bps >= 300 && bps <= 115200);

  /* Enable DLAB. */
  outb (LCR_REG, LCR_N81 | LCR_DLAB);

  /* Set data rate. */
  outb (LS_REG, divisor & 0xff);
  outb (MS_REG, divisor >> 8);

  /* Reset DLAB. */
  outb (LCR_REG, LCR_N81);
}

/* Update interrupt enable register. */
static void
write_ier (void)
{
  uint8_t ier = 0;

  ASSERT (intr_get_level () == INTR_OFF);

  /* Enable transmit interrupt if we have any characters to
     transmit. */
  if (!intq_empty (&txq))
    ier |= IER_XMIT;

  /* Enable receive interrupt if we have room to store any
     characters we receive. */
  if (!input_full ())
    ier |= IER_RECV;

  outb (IER_REG, ier);
}

/* Polls the serial port until it's ready,
   and then transmits BYTE. */
static void
putc_poll (uint8_t byte)
{
  ASSERT (intr_get_level () == INTR_OFF);

  while ((inb (LSR_REG) & LSR_THRE) == 0)
    continue;
  outb (THR_REG, byte);
}

/* Serial interrupt handler. */
void serial_interrupt (int arg)
{
  /* Inquire about interrupt in UART.  Without this, we can
     occasionally miss an interrupt running under QEMU. */
  inb (IIR_REG);

  /* As long as we have room to receive a byte, and the hardware
     has a byte for us, receive a byte.  */
  while (!input_full () && (inb (LSR_REG) & LSR_DR) != 0)
    input_putc (inb (RBR_REG));

  /* As long as we have a byte to transmit, and the hardware is
     ready to accept a byte for transmission, transmit a byte. */
  while (!intq_empty (&txq) && (inb (LSR_REG) & LSR_THRE) != 0)
    outb (THR_REG, intq_getc (&txq));

  /* Update interrupt enable register based on queue status. */
  write_ier ();
}

/* kprintf.c -  kputc, kgetc, kprintf */

#include <xinu.h>
#include <stdarg.h>

/*------------------------------------------------------------------------
 * kputc  -  use polled I/O to write a character to the console
 *------------------------------------------------------------------------
 */

#define QUARK_CONS_PORT		1		/* Index of console UART */
#define QUARK_CONS_BAR_INDEX	0		/* Index of console's MMIO 
						   base address register */

/**
 *  console_init()
 *
 * Initialize the serial console.  The serial console is on the 
 * second memory-mapped 16550 UART device. 
 */
int console_init(void)
{
	int	status;
	int	pciDev;

	pciDev = find_pci_device(INTEL_QUARK_UART_PCI_DID,
				 INTEL_QUARK_UART_PCI_VID,
				 QUARK_CONS_PORT);
	if (pciDev < 0) {
		/* Error finding console device */
		return	pciDev;
	}
	/* Store the console device CSR base address into the console device's
	   device table entry. */
	status = pci_get_dev_mmio_base_addr(pciDev, QUARK_CONS_BAR_INDEX,
					   &devtab[CONSOLE].dvcsr);
	return status;
}

/*------------------------------------------------------------------------
 * kputc  -  use polled I/O to write a character to the console
 *------------------------------------------------------------------------
 */
#ifndef QEMU
syscall kputc(byte c)	/* Character to write	*/
{
	struct	dentry	*devptr;
	volatile struct uart_csreg *csrptr;
	intmask	mask;

	/* Disable interrupts */
	mask = disable();

	devptr = (struct dentry *) &devtab[WCONSOLE];
	csrptr = (struct uart_csreg *)devptr->dvcsr;

	/* Fail if no console device was found */
	if (csrptr == NULL) {
		restore(mask);
		return SYSERR;
	}

	/* Repeatedly poll the device until it becomes nonbusy */
	while ((csrptr->lsr & UART_LSR_THRE) == 0) {
		;
	}

	/* Write the character */
	csrptr->buffer = c;

	/* Honor CRLF - when writing NEWLINE also send CARRIAGE RETURN	*/
	if (c == '\n') {
		/* Poll until transmitter queue is empty */
		while ((csrptr->lsr & UART_LSR_THRE) == 0) {
			;
		}
		csrptr->buffer = '\r';
	}

	restore(mask);
	return OK;
}
#else
syscall kputc(byte c)	/* Character to write	*/
{
	struct	dentry	*devptr;
	intmask	mask;

	/* Disable interrupts */
	mask = disable();
	devptr = (struct dentry *) &devtab[QEMUCONSOLE];
    devptr->dvputc(devptr, c);
	restore(mask);
	return OK;
}
#endif

/*------------------------------------------------------------------------
 * kgetc - use polled I/O to read a character from the console serial line
 *------------------------------------------------------------------------
 */
#ifndef QEMU
syscall kgetc(void)
{
	int irmask;
	volatile struct uart_csreg *csrptr;
	byte c;
	struct	dentry	*devptr;
	intmask	mask;

	/* Disable interrupts */
	mask = disable();

	devptr = (struct dentry *) &devtab[WCONSOLE];
	csrptr = (struct uart_csreg *)devptr->dvcsr;

	/* Fail if no console device was found */
	if (csrptr == NULL) {
		restore(mask);
		return SYSERR;
	}

	irmask = csrptr->ier;		/* Save UART interrupt state.   */
	csrptr->ier = 0;		/* Disable UART interrupts.     */

	/* wait for UART transmit queue to empty */

	while (0 == (csrptr->lsr & UART_LSR_DR)) {
		; /* Do Nothing */
	}

	/* Read character from Receive Holding Register */

	c = csrptr->rbr;
	csrptr->ier = irmask;		/* Restore UART interrupts.     */

	restore(mask);
	return c;
}
#else
syscall kgetc(void)
{
	byte c;
	struct	dentry	*devptr;
	intmask	mask;

	/* Disable interrupts */
	mask = disable();
	devptr = (struct dentry *) &devtab[QEMUCONSOLE];
    c = devptr->dvgetc(devptr);
	restore(mask);
	return c;
}
#endif

extern	void	_doprnt(char *, va_list ap, int (*)(int));

/*------------------------------------------------------------------------
 * kprintf  -  use polled I/O to print formatted output on the console
 *------------------------------------------------------------------------
 */
syscall kprintf(char *fmt, ...)
{
	intmask mask;
	mask = disable();

	va_list ap;

	va_start(ap, fmt);
	_doprnt(fmt, ap, (int (*)(int))kputc);
	va_end(ap);

	restore(mask);
	return OK;
}

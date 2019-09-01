/* qty_dispatch.S - qty_dispatch */

#include <icu.s>
		.text
		.globl	qty_dispatch
		.globl	qty_handler

/*------------------------------------------------------------------------
 *  qty_dispatch  -  Dispatch an interrupt to the qty_ handler function
 *------------------------------------------------------------------------
 */
qty_dispatch:
		pushal			/* Save general-purpose regs.	*/
		pushfl			/* Save the flags register	*/
		cli			/* Prevent further interrupts	*/
		movb	$EOI,%al	/* Clear the interrupt		*/
		outb	%al,$OCW1_2
		call	qty_handler	/* Call the handler		*/
		popfl			/* Restore the flags register	*/
		popal			/* Restore general-purpose regs.*/
		iret			/* Return from interrupt	*/

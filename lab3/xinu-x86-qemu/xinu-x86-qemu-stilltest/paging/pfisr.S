/* pfisr.S */
	.text
	.globl pfisr
/* -------- page fault isr -------- */

pfisr:
	pushal
	pushfl
	call pagefault_handler
	popfl
	popal
	add $0x4, %esp
	iret




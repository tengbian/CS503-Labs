/*  main.c  - main */
#include <xinu.h>
#include <stdio.h>

#include "tests.h"

#define PAGESIZE  (4096)
#define MAGIC_VALUE (1205)
#define MAX_STORES (8)
#define MAX_PAGES_PER_STORE (200)
#define WAITTIME (1000)

#define MSG_SUCC (10000)
#define MSG_FAIL (20000)
#define MSG_ERR (30000)

pid32 main_proc; // for hooks to send message to main process

#define TC (3)


// to save CR0 value
unsigned long cr0val;
/**
 * @return true if paging is indeed turned on.
 * This check should be put before any real test.
 */
static bool8 paging_enabled() {
	// fetch CR0
	asm("pushl %eax");
	asm("movl %cr0, %eax");
	asm("movl %eax, cr0val");
	asm("popl %eax");

	// check if the 31th bit is set
	return cr0val & 0x80000000 ? TRUE : FALSE;
}

// ===== Test Cases =====
static bool8 tc3_1();
static bool8 tc3_2(bool8 virtual);

void gentle3(void)
{
	srpolicy(FIFO);

	/* Start the network */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	//netstart();

	/* Initialize the page server */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	//psinit();

	// for others to send messages to main process
	main_proc = getpid();

	kprintf("=== Test Case %d - NFRAMES %d (should be 3072), POLICY %s ===\n", TC, NFRAMES, "FIFO");

	if (!paging_enabled()) {
		kprintf(" PAGING IS NOT ENABLED!! Thus failed TC %d\n", TC);
//		return OK;
	}

	kprintf("Paging enabled succesfully\n");

	kprintf("Running test 3-a\n");
	tc3_1();

	kprintf("Running test 3-b\n");
	tc3_2(FALSE);

	kprintf("Running test 3-c\n");
	tc3_2(TRUE);

	kprintf("=== Test Case %d Completed\n", TC);
//	return OK;
}


// ===== Actual Test Cases Below =====

static void tc3_1_proc(pid32 mp) {
	uint32 nbytes = PAGESIZE * 5;
	char *mem = vgetmem(nbytes);
	if (mem == (char*) SYSERR) {
		send(mp, MSG_ERR);
	}
	else {
		char *nextpage = mem + PAGESIZE;

		uint32 oldfaults = get_faults();
		*((uint32*)nextpage) = MAGIC_VALUE;
		uint32 newfaults = get_faults();

		vfreemem(mem, nbytes);

		if (newfaults == oldfaults + 1) {
			send(mp, MSG_SUCC);
		}
		else {
			send(mp, MSG_FAIL);
		}
	}

	while (1) { sleepms(1); }
}

/**
 * Access a (mapped) virtual address, should trigger some page fault.
 */
static bool8 tc3_1() {
	bool8 result = FALSE;
	recvclr();

	uint32 hsize = 20;
	pid32 p = vcreate(tc3_1_proc, INITSTK, hsize, INITPRIO, "page fault proc", 1, getpid());
	resume(p);

	umsg32 msg = recvtime(1000);
	if (msg == MSG_SUCC) {
		result = TRUE;
		kprintf("TC3-1 Passed\n");
	}
	else {
		kprintf("TC3-1 Error: didn't receive MSG_SUCC message, received %d\n", msg);
	}

	kill(p);
	return result;
}


void invalid_memory_access(void) {
	int *ptr = (int*) (5000 * PAGESIZE);
	*ptr = MAGIC_VALUE;
	while(1) { sleepms(1); };
}

/**
 * Access a invalid (not-mapped) virtual address, should kill the process.
 * Could check against a normal process or a virtual process.
 */
static bool8 tc3_2(bool8 virtual) {
	bool8 result = FALSE;
	recvclr();

	pid32 p;
	if (virtual) {
		uint32 hsize = 20;
		p = vcreate(invalid_memory_access, INITSTK, hsize, INITPRIO, "page fault proc", 0, NULL);
	}
	else {
		p = create(invalid_memory_access, INITSTK, INITPRIO, "page fault proc", 0, NULL);
	}
	resume(p);

	recvtime(100);
	// p should now have been resumed and executed, and access the invalid memory location.

	int32 ret = kill(p);
	if (ret == SYSERR) {
		result = TRUE;
		kprintf("TC3-2 (%s) Passed\n", virtual ? "virtual" : "non-virtual");
	}
	else {
		kprintf("TC3-2 (%s) Error: p should have already been killed, but it seems not, return value %d\n", virtual ? "virtual" : "non-virtual", ret);
	}
	return result;
}


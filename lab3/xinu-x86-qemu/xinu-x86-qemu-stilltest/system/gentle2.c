/*  main.c  - main */
#include <xinu.h>
#include <stdio.h>

#include "tests.h"

#define PAGESIZE  (4096)
#define MAGIC_VALUE (1205)
#define MAX_STORES (8)
#define MAX_PAGES_PER_STORE (200)
#define WAITTIME (1000)

#define TC (2)


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
static bool8 tc2_1();
static bool8 tc2_2();
static bool8 tc2_3();
static bool8 tc2_4();

// ===== Helper Functions =====
static void dumb_proc();


void gentle2(void)
{
	srpolicy(FIFO);

	/* Start the network */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	//netstart();

	/* Initialize the page server */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	//psinit();

	kprintf("=== Test Case %d - NFRAMES %d (should be 3072), POLICY %s ===\n", TC, NFRAMES, "FIFO");

	if (!paging_enabled()) {
		kprintf(" PAGING IS NOT ENABLED!! Thus failed TC %d\n", TC);
//		return OK;
	}


	kprintf("Paging enabled successfully\n");


	kprintf("Running test 2-a\n");
	tc2_1();

	kprintf("Running test 2-b\n");
	tc2_2();

	kprintf("Running test 2-c\n");
	tc2_3();
	tc2_4();

	kprintf("=== Test Case %d Completed ===\n", TC);

//        return OK;
}


// ===== Actual Test Cases Below =====
/**
 * vcreate() test.
 * Should succeed 8 vcreate() calls, when the passed-in pages is small enough.
 */
static bool8 tc2_1() {
	bool8 result = TRUE;

	pid32 ps[MAX_STORES];
	int i;
	for (i = 0; i < MAX_STORES; i++) {
		ps[i] = SYSERR;
	}

	uint32 hsize = 20; // pick a small number
	for (i = 0; i < MAX_STORES; i++) {
		ps[i] = vcreate(dumb_proc, INITSTK, hsize, INITPRIO, "TC2-1", 0, NULL);

		if (ps[i] == SYSERR) {
			// didn't expect to fail here
			result = FALSE;
			kprintf("TC2-1 Error: Failed to successfully create all %d processes\n", MAX_STORES);
			break;
		}
	}

	if (result) {
		kprintf("TC2-1 Passed\n");
	}

	// clean up
	for (i = 0; i < MAX_STORES; i++) {
		kill(ps[i]);
		ps[i] = SYSERR;
	}
	return result;
}

/**
 * vcreate() test.
 * Should successfully vcreate() a process with all backing store storage (split).
 */
static bool8 tc2_2() {
	bool8 result = TRUE;

	pid32 p = vcreate(dumb_proc, INITSTK, MAX_PAGES_PER_STORE * MAX_STORES, INITPRIO, "TC2-2", 0, NULL);

	result = p != SYSERR;
	if (result) {
		kprintf("TC2-2 Passed\n");
	}
	else {
		kprintf("TC2-2 Error: Fail to allocate multiple backing store for one process' use\n");
	}

	kill(p);
	return result;
}

/**
 * For creating dumb processes.
 */
static void dumb_proc() {
	while (1) {}
}

local void tc2_3_proc(pid32 mp) {
	sleepms(100);
	int i, j;
	double data[10][10];
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			data[i][j] = myglobalclock;
		}
	}

	send(mp, MAGIC_VALUE + data[0][0] - data[1][1]);

	while (1) { sleepms(1); }
}

/**
 * A process created by vcreate() should actually work!
 * Tested by letting it sending back a msg to main.
 */
static bool8 tc2_3() {
	bool8 result = FALSE;
	recvclr();

	uint32 hsize = 20;
	pid32 p = vcreate(tc2_3_proc, INITSTK, hsize, INITPRIO, "vcreate() test", 1, getpid());
	resume(p);

	umsg32 msg = recvtime(1000);
	if (msg == MAGIC_VALUE) {
		result = TRUE;
		kprintf("TC2-3 Passed\n");
	}
	else {
		kprintf("TC2-3 Error: didn't receive MAGIC_VALUE message, received %d\n", msg);
	}

	kill(p);
	return result;
}


// put in an array, different slots, so as not to conflict
bool8 tc2_4_results[2];

local void tc2_4_proc(uint32 idx, uint32 val) {
	uint32 nbytes = 1024;
	char *vmem = vgetmem(nbytes);
	if (vmem == (char*) SYSERR) {
		kprintf("TC2-4 Error: Even failed to get virtual memory.\n");
		tc2_4_results[idx] = FALSE;
	}
	else {
		uint32 *p = (uint32*) vmem;
		*p = val;

		sleepms(50);

		uint32 read = *p;
		if (read != val) {
			// error!
			tc2_4_results[idx] = FALSE;
		}

		int32 ret = vfreemem(vmem, nbytes);
		if (ret == SYSERR) {
			tc2_4_results[idx] = FALSE;
		}
	}

	tc2_4_results[idx] = TRUE;
	while (1) { sleepms(1); }
}


/**
 * Ensure that different virtual processes have differnt virtual memory. Won't interfere.
 */
static bool8 tc2_4() {
	bool8 result = FALSE;

	tc2_4_results[0] = FALSE;
	tc2_4_results[1] = FALSE;

	recvclr();

	uint32 hsize = 20;
	pid32 p1 = vcreate(tc2_4_proc, INITSTK, hsize, INITPRIO, "vcreate() test", 2, 0, MAGIC_VALUE + 100);
	pid32 p2 = vcreate(tc2_4_proc, INITSTK, hsize, INITPRIO, "vcreate() test", 2, 1, MAGIC_VALUE + 200);
	resched_cntl(DEFER_START);
	resume(p1);
	resume(p2);
	resched_cntl(DEFER_STOP);

	sleepms(100);

	result = tc2_4_results[0] && tc2_4_results[1];
	if (result) {
		kprintf("TC2-4 Passed\n");
	}
	else {
		kprintf("TC2-4 Error: Not both process read the same written value.\n");
	}

	kill(p1);
	kill(p2);
	return result;
}

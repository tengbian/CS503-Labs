#include <xinu.h>
#include <stdio.h>

#include "tests.h"

#define PAGESIZE  (4096)
#define MAGIC_VALUE (1205)

#define TC (1)

// to save CR0 value
unsigned long cr0val;
/**
 *  * @return true if paging is indeed turned on.
 *    This check should be put before any real test.
 *    
 *    */
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
static bool8 tc1_1();
static bool8 tc1_2();

// ===== Helper Functions =====
static uint32 access_addr(uint32 vpage, uint32 offset, bool8 towrite);

void gentle1() 
{
	srpolicy(FIFO);

	kprintf("=== Test Case %d - NFRAMES %d (should be 3072), POLICY %s ===\n", TC, NFRAMES, "FIFO");

	if (!paging_enabled()) {
		kprintf(" PAGING IS NOT ENABLED!! Thus failed TC %d\n", TC);
		//return OK;
	}

	kprintf("Paging enabled successfully\n");

	kprintf("Running test 1-a\n");
	tc1_1();

	kprintf("Running test 1-b\n");
	tc1_2();

	kprintf("=== Test Case %d Completed ===\n", TC);
	
	//return OK;
}

static bool8 tc1_1() {
	bool8 result = FALSE;

	uint32 oldfaults = get_faults();
	uint32 pg;

	kprintf("starting page faults: %ld\n", oldfaults);
	
	for (pg = 0; pg < 4094; pg++) {
		access_addr(pg, pg, FALSE);
	}

	uint32 newfaults = get_faults();

	kprintf("ending page faults: %ld\n", newfaults);
	result = oldfaults == newfaults;

	if (result) {
		kprintf("TC1-1 Passed\n");
	}
	else {
		kprintf("TC1-1 Error: accessing 0-4095 page should not trigger page fault\n");
	}

	return result;
}

static bool8 tc1_2() {
	bool8 result = FALSE;

	uint32 oldfaults = get_faults();

	uint32 pg = 589824; // first page in device memory
	uint32 count = 0;

	for (count = 0; count < 1024; count++) {
		access_addr(pg + count, count, FALSE);
	}

	uint32 newfaults = get_faults();

	result = oldfaults == newfaults;
	if (result) {
		kprintf("TC1-2 Passed\n");
	}
	else {
		kprintf("TC1-2 Error: accessing device memory should not trigger page fault\n");
	}

	return result;
}

static uint32 access_addr(uint32 vpage, uint32 offset, bool8 towrite) {
	offset %= PAGESIZE;

	uint32 *p = (uint32*)(vpage * PAGESIZE + offset);

	if (towrite) {
		*p = MAGIC_VALUE;
		return 0;
	}
	else {
		// read
		return *p;
	}
}


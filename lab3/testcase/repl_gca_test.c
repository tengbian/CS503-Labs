#include <xinu.h>

#include "tests.h"

#define PAGESIZE  4096
#define PAGE_ALLOCATION 200

/* NOTE: This does not check if paging is enabled or not, you
   should check that before starting the tests.
*/

/* Set to 1 to test page replacement
 * Set to 0 to check page fault handling is correct
 */
#define PAGE_REPLACEMENT 1

// Return a deterministic value per addr for testing.
static uint32 get_test_value(uint32 *addr) {
  static uint32 v1 = 0x12345678;
  static uint32 v2 = 0xdeadbeef;
  return (uint32)addr + v1 + ((uint32)addr * v2);
}

static void do_policy_test(void) {
  uint32 npages = PAGE_ALLOCATION - 1;
  uint32 nbytes = npages * PAGESIZE;

  kprintf("Running Page Replacement Policy Test, with NFRAMES = %d\n", NFRAMES);

  char *mem = vgetmem(nbytes);
  if (mem == (char*) SYSERR) {
    panic("Page Replacement Policy Test failed\n");
    return;
  }

  // Write data
  for (uint32 i = 0; i<npages; i++) {
    uint32 *p = (uint32*)(mem + (i * PAGESIZE));

    if (i%2 == 0) {
    	kprintf("Write Iteration [%3d] at 0x%08x\n", i, p);
    	for (uint32 j=0; j<PAGESIZE; j=j+4) {
      		uint32 v = get_test_value(p);
      		*p++ = v;
    	}
    } else {	
    	kprintf("Read Iteration [%3d] at 0x%08x\n", i, p);
    	for (uint32 j=0; j<PAGESIZE; j=j+4) {
      		uint32 v = get_test_value(p);
		kprintf("%c", *p++);
    	}
	kprintf("\n");
    }

    sleepms(20); // to make it slower
  }

  // Check the data was truly written
  for (uint32 i = 0; i<npages; i++) {
    uint32 *p = (uint32*)(mem + (i * PAGESIZE));
    kprintf("Check Iteration [%3d] at 0x%08x\n", i, p);

    if (i%2 == 0) {
    	for (uint32 j=0; j<PAGESIZE; j=j+4) {
      		uint32 v = get_test_value(p);
      		ASSERT(*p++ == v);
    	}
    }

    sleepms(20); // to make it slower
  }

  if (vfreemem(mem, nbytes) == SYSERR) {
    panic("Policy Test: vfreemem() failed.\n");
  } else {
#if PAGE_REPLACEMENT == 1
    kprintf("\nPage Replacement Policy (GCA) Test Finished.\n");
#else
    kprintf("\nPage Fault Handling Test Finished\n");
#endif
    kprintf("Here NFRAMES = %d\n", NFRAMES);
  }
}

/**
 * Just iterate through a lot of pages, and check if the output satisfies the policy.
 * Based on the hooks: hook_pfault and hook_pswap_out, you can ascertain if the test
 * passes or not. The hooks are supposed to throw a panic if the policy is not being
 * followed. (NFRAMES should be <= 50 for page replacement testing)
 */
void gca_policy_test(int num_proc) {

  recvclr();

  if (srpolicy(GCA) == SYSERR) {
	kprintf("GCA not implemented\n");
	return;
  }

  kprintf("GCA implemented it seems\n");

#if PAGE_REPLACEMENT == 1
  kprintf("Starting Policy (GCA) Testing Test\n");
  if (NFRAMES > 50) {
    kprintf("Test Failed. Please set NFRAMES to <= 50 and try again.\n");
    return;
  }
#else
  kprintf("Starting Page Fault Handling Test\n");
  if (NFRAMES < 200) {
    kprintf("Test Failed. Please set NFRAMES to >= 200 and try again\n");
    return;
  }
#endif

  pid32 p[num_proc];
  for (int i = 0; i < num_proc; i++) {
  	p[i] = vcreate(do_policy_test, INITSTK, PAGE_ALLOCATION,
                    INITPRIO, "page rep", 0, NULL);
  	resume(p[i]);
  }

  int flag = 0;

  while (1) {

    flag = 0;

    for (int i = 0; i < num_proc; i++) {
	if(proctab[p[i]].prstate != PR_FREE) {
  		flag = 1;
	}
    }

    if (flag == 0) {
	break;
    }
    else {
      sleepms(100);
    }

  }

  kprintf("\n\nTest Passed.\n\n");

  return;
}

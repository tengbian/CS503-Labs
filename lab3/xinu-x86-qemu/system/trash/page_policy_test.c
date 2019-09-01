#include <xinu.h>

#define PAGESIZE  4096
#define PAGE_ALLOCATION 100

/* NOTE: This does not check if paging is enabled or not, you
   should check that before starting the tests.
*/

/* Set to 1 to test page replacement
 * Set to 0 to check page fault handling is correct
 */
#define PAGE_REPLACEMENT 0

// Return a deterministic value per addr for testing.
uint32 get_test_value(uint32 *addr) {
  static uint32 v1 = 0x12345678;
  static uint32 v2 = 0xdeadbeef;
  return (uint32)addr + v1 + ((uint32)addr * v2);
}

static void do_policy_test(void) {
  uint32 npages = PAGE_ALLOCATION - 1;
  uint32 nbytes = npages * PAGESIZE;

  kprintf("Running Page Replacement Policy Test, with NFRAMES = %d\n", NFRAMES);

  kprintf("currpid: %d, Start vgetmem, cr3 = %u\n",currpid,read_cr3());
  char *mem = vgetmem(nbytes);
  kprintf("currpid: %d, End vgetmem, cr3 = %u\n",currpid,read_cr3());

  if (mem == (char*) SYSERR) {
    panic("Page Replacement Policy Test failed\n");
    return;
  }

  // Write data
  kprintf("----------- Write begin\n");
  for (uint32 i = 0; i<npages; i++) {
	kprintf("i = %u, write data: vaddr: %u\n",i,(uint32) (mem+ i*PAGESIZE));
	
	//test, Teng
	//uint32 aaa = (uint32) (mem + i*PAGESIZE);
	//uint32 ppp = aaa >> 22;
	//uint32 qqq = (aaa << 10) >> 22;
	//kprintf("aaa: %u ,ppp: %u,qqq %u\n",aaa,ppp,qqq);
	//kprintf("pd pres: %u\n",(((pd_t *) fid_2_vaddr(proctab[currpid].pr_pdid))[ppp].pd_pres));
	//kprintf("pt pres: %u\n",((pt_t *)(((pd_t *) fid_2_vaddr(proctab[currpid].pr_pdid))[ppp].pd_base*PAGECHUNKSIZE))[qqq].pt_pres);

    uint32 *p = (uint32*)(mem + (i * PAGESIZE));
	kprintf("address of p: %u, value of p: %u\n",(uint32) p, *p);

    // kprintf("Write Iteration [%3d] at 0x%08x\n", i, p);
    for (uint32 j=0; j<PAGESIZE; j=j+4) {
      uint32 v = get_test_value(p);
      *p++ = v;
    }

    sleepms(20); // to make it slower
  }
  kprintf("----------- Write succeed\n");

  // Check the data was truly written
  for (uint32 i = 0; i<npages; i++) {
    uint32 *p = (uint32*)(mem + (i * PAGESIZE));
	kprintf("address of p: %u, value of p: %u\n",(uint32) p, *p);
    kprintf("Check Iteration [%3d] at 0x%08x\n", i, p);
    for (uint32 j=0; j<PAGESIZE; j=j+4) {
      uint32 v = get_test_value(p);
      ASSERT(*p++ == v);
    }

    sleepms(20); // to make it slower
  }

  if (vfreemem(mem, nbytes) == SYSERR) {
    panic("Policy Test: vfreemem() failed.\n");
  } else {
#if PAGE_REPLACEMENT == 1
    kprintf("\nPage Replacement Policy Test Finished.\n");
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
void page_policy_test(void) {
  recvclr();

#if PAGE_REPLACEMENT == 1
  kprintf("Starting Policy (FIFO) Testing Test\n");
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

  pid32 p = vcreate(do_policy_test, INITSTK, PAGE_ALLOCATION,
                    INITPRIO, "page rep 0", 0, NULL);

  pid32 p1 = vcreate(do_policy_test, INITSTK, PAGE_ALLOCATION, INITPRIO, "page rep 1", 0, NULL);

  pid32 p2 = vcreate(do_policy_test, INITSTK, PAGE_ALLOCATION,INITPRIO, "page rep 2", 0, NULL);

  //pid32 p3 = vcreate(do_policy_test, INITSTK, PAGE_ALLOCATION,INITPRIO, "page rep 3", 0, NULL);
  resume(p);
  resume(p1);
  resume(p2);
  //resume(p3);
  kprintf("resume page rep\n");
  kprintf("resume page rep\n");
  kprintf("resume page rep\n");
  kprintf("resume page rep\n");

  while (1) {
    if(proctab[p].prstate == PR_FREE) {
      break;
    }
    else {
      sleepms(100);
    }
  }

  kprintf("\n\nTest Passed.\n\n");

  return;
}

#include <xinu.h>

extern void page_policy_test(void);

process	main(void)
{
  srpolicy(FIFO);

  /* Start the network */
  /* DO NOT REMOVE OR COMMENT BELOW */
#ifndef QEMU
  netstart();
#endif

  /*  TODO. Please ensure that your paging is activated 
      by checking the values from the control register.
  */

  /* Initialize the page server */
  /* DO NOT REMOVE OR COMMENT THIS CALL */
  psinit();
  //kprintf("%u\n",read_cr3());
  show32(read_cr3());
  //page_policy_test();
	gentle2();

  return OK;
}

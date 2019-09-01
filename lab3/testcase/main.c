/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

#include "tests.h"

process	main(void)
{
	/* Start the network */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	netstart();

	/* Initialize the page server */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	psinit();

	kprintf("=== Lab4 ===\n");

	kprintf("Running Simple (Non-page replacement tests)\n");
// Run these with 3072 frames
	for (int i = 0; i < 8; i++) {
		
		kprintf("Running test #%d\n\n", i);

		if (i == 0) {
			gentle1();
		} else if (i == 1) {
			gentle2();
		} else if (i == 2) {
			gentle3();
		} else if (i == 3) {
			custom(1);
		} else if (i == 4) {
			hard(1);
		} else if (i == 5) {
			custom(2);
		} else if (i == 6) {
			hard(2);
		} else if (i == 7) {
			custom(3);
		} else if (i == 8) {
		//	custom(4);
		}

		kprintf("\n\nCompleted test #%d\n\n", i);

		sleep(10);
	}
/*
// Run these with 50 frames
	for (int i = 0; i < 4; i++) {
		kprintf("Running Test #%d\n", i);

		if (i == 0) {
			fifo_policy_test(1);
		} else if (i == 1) {
			fifo_policy_test(2);
		} else if (i == 2) {
			srpolicy(GCA);
			gca_policy_test(1);
		} else if (i == 3) {
			srpolicy(GCA);
			gca_policy_test(2);
		}

		kprintf("\n\nCompleted Test #%d\n\n", i);

		sleep(10);

	}
*/
	return OK;
}

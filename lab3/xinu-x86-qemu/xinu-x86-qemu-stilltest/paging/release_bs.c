/* release_bs.c - release_bs */

#include <xinu.h>

/*----------------------------------------------------------------------------
 *  release_bs  -  This call requests that the page server release the backing
 *  store with ID bs_id.
 *
 *  Note: You don't need to invoke this directly.
 *----------------------------------------------------------------------------
 */
syscall release_bs (bsd_t bs_id)
{
	if(PAGE_SERVER_STATUS == PAGE_SERVER_INACTIVE){
		kprintf("Page server is not active\r\n");
		return SYSERR;
	}

	wait(bs_sem);
	if (bs_id > MAX_ID || bs_id < MIN_ID || bstab[bs_id].isopen == FALSE) {
		kprintf("releasing the bs failed for bs_id %d\r\n",
						bs_id);
		signal(bs_sem);
		return SYSERR;
	}

	bstab[bs_id].isopen = FALSE;
	bstab[bs_id].npages = 0;
	signal(bs_sem);

	return OK;
}

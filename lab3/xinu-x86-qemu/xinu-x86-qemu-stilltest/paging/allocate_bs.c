/* allocate_bs.c - allocate_bs */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  allocate_bs  -  Allocates a free backing store; returns a bs number if
 *  successfull or SYSERR if they are all occupied.
 *------------------------------------------------------------------------
 */
bsd_t	allocate_bs (
	uint32 npages
	)
{
	int32	i;
	intmask	mask;

	/* Ensure only one process accesses bstab */
	mask = disable();

	if(PAGE_SERVER_STATUS == PAGE_SERVER_INACTIVE){
		psinit();
	}

	// FIXME may be more consistent to traverse from MIN_ID to MAX_ID.
	/* Find an unallocated store */
	for(i = 0; i < MAX_BS_ENTRIES; i++) {
		if(bstab[i].isallocated == FALSE) { /* Found an unallocated store */
			if(get_bs(i, npages) == SYSERR) { /* Try to open it */
				continue;
			}
			/* Successfully opened, allocate it */
			bstab[i].isallocated = TRUE;
			bstab[i].usecount = 0;

			restore(mask);
			return i;
		}
	}

	restore(mask);
	return SYSERR;
}

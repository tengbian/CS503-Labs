/* open_bs.c - open_bs */

#include <xinu.h>

/*----------------------------------------------------------------------------
 *  open_bs  -  Return the store number if the backing is allocated and
 *  increase the use count; return SYSERR otherwise.
 *----------------------------------------------------------------------------
 */
bsd_t	open_bs (
	bsd_t	store
	)
{
	/* Sanity check on store ID */
	if (store > MAX_ID || store < MIN_ID) {
		return SYSERR;
	}

	/* Ensure only one process accesses the bstab */
	wait(bs_sem);

	/* Ensure the store is allocated */
	if(bstab[store].isallocated == FALSE) {
		signal(bs_sem);
		return SYSERR;
	}

	/* Increase the usecount of the store */
	bstab[store].usecount++;

	signal(bs_sem);
	return store;
}

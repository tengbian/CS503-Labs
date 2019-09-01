/* close_bs.c - close_bs */

#include <xinu.h>

/*----------------------------------------------------------------------------
 *  close_bs  -  Return the store number if successful and decrease use count;
 *  return SYSERR otherwise.
 *----------------------------------------------------------------------------
 */
bsd_t	close_bs (
	bsd_t store
	)
{
	/* Sanity check on store ID */
	if (store > MAX_ID || store < MIN_ID) {
		return SYSERR;
	}

	/* Ensure only one process accesses bstab */
	wait(bs_sem);

	/* Ensure the store is allocated */
	if(bstab[store].isallocated == FALSE) {
		signal(bs_sem);
		return SYSERR;
	}

	/* Decrease the usecount of the store */
	bstab[store].usecount--;

	signal(bs_sem);
	return store;
}

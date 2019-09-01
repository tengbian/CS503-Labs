/* deallocate_bs.c - deallocate_bs */

#include <xinu.h>

/*----------------------------------------------------------------------------
 *  deallocate_bs  -  Deallocate the backing store if it is not being used and
 *  return the bs number; return SYSERR otherwise.
 *----------------------------------------------------------------------------
 */
bsd_t	deallocate_bs (
	bsd_t store
	)
{
	intmask	mask;

	/* Sanity check on store ID */
	if (store > MAX_ID || store < MIN_ID) {
		return SYSERR;
	}

	/* Ensure only one process accesses bstab */
	mask = disable();

	/* Ensure this store is allocated */
	if(bstab[store].isallocated == FALSE) {
		restore(mask);
		return SYSERR;
	}

	/* Ensure no one is using this store */
	if(bstab[store].usecount > 0) {
		restore(mask);
		return SYSERR;
	}

	/* Release the store */
	release_bs(store);

	bstab[store].isallocated = FALSE;
	bstab[store].usecount = 0;

	restore(mask);
	return store;
}

/* srpolicy.c - srpolicy */

#include <xinu.h>

int32 currpolicy;

/*------------------------------------------------------------------------
 *  srplicy  -  Set the page replacement policy.
 *------------------------------------------------------------------------
 */
syscall srpolicy(int policy)
{
	switch (policy) {
	case FIFO:
		/* LAB3 TODO */
		currpolicy = FIFO;
		return OK;

	case GCA:
		/* LAB3TODO - Bonus Problem */
	    currpolicy = GCA;
    //return SYSERR;
		return OK;

	default:
		return SYSERR;
	}
}

/* vfreemem.c - vfreemem */

#include <xinu.h>

syscall	vfreemem(
	  char		*blkaddr,	/* Pointer to memory block	*/
	  uint32	nbytes		/* Size of block in bytes	*/
	)
{
  // Lab3 TODO.
  	
	intmask mask;
	struct memblk *next, *prev, *block;
	struct memblk *vheapptr;
	uint32 top;

	mask = disable();

	if ((nbytes == 0) || ((uint32) blkaddr < MIN_VHEAP)
			|| ((uint32) blkaddr > (uint32) MAX_VHEAP)) {
		restore(mask);
		return SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes); /* use memblk multiples */
	block = (struct memblk *)blkaddr;

	vheapptr =  &proctab[currpid].vheaphead;
	prev = vheapptr;
	next = prev->mnext;
	while ((next != NULL) && (next < block)) {
		prev = next;
		next = next->mnext;
	}

	if (prev == vheapptr){ /* compute top of previous block */
		top = (uint32) NULL;
	} else {
		top = (uint32) prev + prev->mlength;
	}

	/* Ensure new block does not overlap previous or next blocks */
	if (((prev != vheapptr) && (uint32) block < top)
			|| ((next != NULL) && (uint32) block + nbytes > (uint32)next)) {
		restore(mask);
		return SYSERR;
	}

	/* Either coalesce with previous block or add to free list */
	
	if (top == (uint32) block) { /* Coalesce with previous block */
		prev->mlength += nbytes;
		block = prev;
	} else {
		block->mnext = next;
		block->mlength = nbytes;
		prev->mnext = block;
	}

	/* Coalesce with next block if adjacent */

	if (((uint32) block + block->mlength) == (uint32) next) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
	}
	restore(mask);
	return OK;
}

/* vgetmem.c - vgetmem */

#include <xinu.h>

// vgetmem can only be used except in vcreate
char  	*vgetmem(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
  // Lab3 TODO.
  	
	intmask mask;
	struct memblk *prev, *curr, *leftover;
	struct memblk *vheaphead;
	struct memblk *memptr;// for temp use

	mask = disable();
	if (nbytes == 0){
		restore(mask);
		return (char *) SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes); /*use memblk multiples */
	

	vheaphead = &proctab[currpid].vheaphead; // this vheaphead is a pointer
	// initialize vheap
	if(proctab[currpid].vheap_made == 0){
		//kprintf("currpid: %d, ----- vgetmem init -----\n",currpid);
		// make the heap
		memptr = (struct memblk *)MIN_VHEAP;
		//kprintf("currpid: %d, new memptr vaddr %u\n",currpid,(uint32) memptr);
		vheaphead->mnext = memptr;
		vheaphead->mlength = proctab[currpid].vheapsize;//MAX_VHEAP-MIN_VHEAP;
		//kprintf("currpid: %d, 1\n",currpid);

		//kprintf("currpid: %d, vaddr of memptr->next %u\n",currpid,(uint32) &memptr->mnext);
		memptr->mnext = (struct memblk *)NULL;
		//kprintf("currpid: %d, 2\n",currpid);
		memptr->mlength = proctab[currpid].vheapsize;
		//vheaphead->mlength -= proctab[currpid].vheapsize;
		//kprintf("3\n");

		proctab[currpid].vheap_made = 1;
		//kprintf("4\n");
	}

	/*
	if(nbytes > vheaphead->mlength){
		kill(currpid);
		restore(mask);
		return (char *) SYSERR;
	}
	*/

	prev = vheaphead;
	curr = vheaphead->mnext;
	//kprintf("curr value: %u\n",(uint32) curr);
	
	while(curr != NULL){
		//kprintf("-- curr page number: %u\n",(uint32) curr/PAGECHUNKSIZE);
		if (curr->mlength == nbytes){ /* Block is exact match */
			prev->mnext = curr->mnext;
			vheaphead->mlength -= nbytes;
			//kprintf("block exact match\n");
			restore(mask);
			return (char *)(curr);

		} else if (curr->mlength > nbytes) { /* Split big block */
			leftover = (struct memblk *) ( (uint32) curr + nbytes);
			prev->mnext = leftover;
			//kprintf("Here 000 \n");
			leftover->mnext = curr->mnext;
			//kprintf("Here 001 \n");
			leftover->mlength = curr->mlength - nbytes;
			vheaphead->mlength -= nbytes;
			//kprintf("block split, assigned vaddr head: %u\n",(uint32) curr);
			restore(mask);
			return (char *)(curr);
		} else {
			prev = curr;
			curr = curr->mnext;
		}
	}
	//kprintf("Error in vgetmem\n");

	kill(currpid);
	restore(mask);
	return (char *)SYSERR;
}


/*
// Error in this function; we can just simplify this initial process
// delete it
char *vgetmem_init(int32 pid, uint32 nbytes){

	intmask mask;
	struct memblk *prev, *curr, *leftover;
	struct memblk *vheaphead;
	struct memblk *memptr;

	kprintf("Here 1 ----\n");

	mask =disable();
	if(nbytes == 0){
		restore(mask);
		return (char *) SYSERR;
	}
	nbytes = (uint32) roundmb(nbytes); // use memblk multiples

	kprintf("Here 1 ----\n");

	// Initialize the free list 
	kprintf("Here 1 ----\n");

	vheaphead = &proctab[pid].vheaphead;
	kprintf("vheaphead: %u\n",vheaphead);
	memptr = vheaphead;
	memptr->mnext = (struct memblk *)NULL;
	memptr->mlength = 0;

	//uint32 vheapmin = 0x01000000;
	//uint32 vheapmax = 0x8FFFFFFF;

	kprintf("Here 1 ----\n");

	// Error here
	// I can not construct the heap list in current proc
	memptr->mnext = (struct memblk *)MIN_VHEAP;
	memptr->mlength = MAX_VHEAP - MIN_VHEAP;

	memptr = memptr->mnext;
	memptr->mnext = (struct memblk *) NULL;
	memptr->mlength = MAX_VHEAP - MIN_VHEAP;

	kprintf("Here 1 ----\n");

	// assign heap
	prev = vheaphead;
	curr = vheaphead->mnext;

	while(curr != NULL){
		kprintf("Here 2 ----\n");
		
		if (curr->mlength == nbytes){ // Block is exact match 
			prev->mnext = curr->mnext;
			vheaphead->mlength -= nbytes;
			restore(mask);
            return (char *)(curr);

		} else if (curr->mlength > nbytes) { // Split big block 
            leftover = (struct memblk *) ( (uint32) curr + nbytes);
            prev->mnext = leftover;
            leftover->mnext = curr->mnext;
            leftover->mlength = curr->mlength - nbytes;
            vheaphead->mlength -= nbytes;
            restore(mask);
            return (char *)(curr);
        } else {
            prev = curr;
            curr = curr->mnext;
		}
	}
	kprintf("Here 3 ----\n");

	restore(mask);
	return (char *) SYSERR;
}
*/








/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
    // LAB2: TODO: Modify this function to clean-up the pipe.

	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}

	freestk(prptr->prstkbase, prptr->prstklen);


	// declaration for Lab 2
	int pi;
	int di;
	struct pipe_t *pipptr;

	switch (prptr->prstate) {
	case PR_CURR:

		// if it's the owner of pipe, delete the pipe
		pi = 0;
		while(pi < MAXPIPES){
			pipptr = &pipe_tables[pi];
			di = pipid32_to_did32(pi);
			// when reader/writer terminates
			if( pipptr->writer == currpid || pipptr->reader == currpid ){
				//kprintf("Call disconnect\n");
				pipdisconnect(di);
				//kprintf("%s exits!\n",prptr->prname);
			}
			pi++;
		}

		//kprintf("%s exits!\n",prptr->prname);
		prptr->prstate = PR_FREE;	/* Suicide */

		/*
		for(int iii=0; iii < NPROC; iii++){
			if(proctab[iii].prstate != PR_FREE){
				//kprintf("Remaining process: %s, state: %d, priority: %d\n",proctab[iii].prname,proctab[iii].prstate,proctab[iii].prprio);
			}
		}
		kprintf("\n");
		*/

		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}

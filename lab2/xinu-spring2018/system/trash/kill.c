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

	// if it's the owner of pipe, delete the pipe
	int pi;
	int di;
	struct pipe_t *pipptr;
	pi = 0;
	// if currpid = owner, reader, writer, it can call pipdelete
	// if currpid = reader, writer, it can call pipdisconnecte
	// if pid = owner? delete the pipe
	// if pid = reader? 
	// if pid = writer?
	while(pi < MAXPIPES){
		pipptr = &pipe_tables[pi];
		di = pipid32_to_did32(pi);

		if(pipptr->owner == pid){
			pipdelete(di);
		}
		else if(pipptr->writer == pid){
			if(pipptr->state == PIPE_CONNECTED){
				pipptr->state = PIPE_RD_CONN;
			}else if (pipptr->state == PIPE_WT_CONN){
				//pipptr->state = PIPE_DISCONN;
				//pipdelete(di);
				pipdisconnect(di);
			}else{
				kprintf("Wrong state when killed\n");
				return SYSERR;
			}
		}else if(pipptr->reader == pid){
			if(pipptr->state == PIPE_CONNECTED){
				pipptr->state = PIPE_WT_CONN;
			}else if (pipptr->state == PIPE_RD_CONN){
				//pipptr->state = PIPE_DISCONN;
				//pipdelete(di);
				pipdisconnect(di);
			}else{
				kprintf("Wrong state when killed\n");
				return SYSERR;
			}
		}
		pi++;
	}


	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
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

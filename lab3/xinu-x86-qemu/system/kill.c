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
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	//kprintf("KILL : %d\n", pid);
	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}
	
	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}

  // Lab3 TODO. Free frames as a process gets killed.
	// deallocate bs
	pd_t *pdptr;
	pt_t *ptptr;
	int32 j;

 	for(i=0;i<MAX_BS_ENTRIES;i++){
		if( (bsmaptab[i].state == BS_USED) && (bsmaptab[i].pid == pid) ){
			deallocate_bs(i);
			bsmaptab[i].state = BS_FREE;
			// ignore others, since they would be assigned next time used
		}
	}
	// deallocate pd,pt (also pages?), pages of no need, replace latter
	// or may be we can save some time replacing?
	// free fifolist
	for(i=0;i<NFRAMES;i++){
		if( frametab[i].pid == pid){
			// delete pd/pt
			if(frametab[i].ftype == PD_TYPE){
				pdptr =	(pd_t *) fid_2_vaddr(frametab[i].fid); // just i..., huashetianzu...
				for(j=0;j<PAGEDIRSIZE;j++){
					pdptr[j].pd_pres = 0;
					pdptr[j].pd_write = 0;
					pdptr[j].pd_user = 0;
					pdptr[j].pd_pwt = 0;
					pdptr[j].pd_pcd = 0;
					pdptr[j].pd_acc = 0;
					pdptr[j].pd_mbz = 0;
					pdptr[j].pd_fmb = 0;
					pdptr[j].pd_global = 0;
					pdptr[j].pd_avail = 0;
					pdptr[j].pd_base = 0;
				}
				//kprintf("pd deleted\n");
			}
			else if (frametab[i].ftype == PT_TYPE){
				ptptr = (pt_t *) fid_2_vaddr(frametab[i].fid);
				for(j=0;j<PAGETABSIZE;j++){
					ptptr[j].pt_pres = 0;
					ptptr[j].pt_write = 0;
					ptptr[j].pt_user = 0;
					ptptr[j].pt_pwt = 0;
					ptptr[j].pt_pcd = 0;
					ptptr[j].pt_acc = 0;
					ptptr[j].pt_dirty = 0;
					ptptr[j].pt_mbz = 0;
					ptptr[j].pt_global = 0;
					ptptr[j].pt_avail = 0;
					ptptr[j].pt_base = 0;
				}
				//kprintf("pt deleted\n");
			}
			// ...

			// delete frame
			frametab[i].pid = -1;
			frametab[i].vid = -1;
			frametab[i].ftype = NO_TYPE;
			frametab[i].fstate = FRAME_FREE;
			frametab[i].refcount = 0;
			frametab[i].advisor = 0;
		}
	}

	

	//kprintf("killed: prname: %s, prpid: %d\n",prptr->prname,pid);
	freestk(prptr->prstkbase, prptr->prstklen);

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

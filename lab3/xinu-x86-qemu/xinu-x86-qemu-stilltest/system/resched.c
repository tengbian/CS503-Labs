/* resched.c - resched, resched_cntl */

#include <xinu.h>

int32 tmp;

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	//kprintf("resched\n");
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */

	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/

  // Lab3. TODO: change the page directories as a process is ctx out
	
	// change cr3 only when change pd address
	// here check???? pr_pdid is initalized only for vpr???
	//if( ptnew->pr_pdid != proctab[NULLPROC].pr_pdid){
	if( ptnew->pr_pdid != ptold->pr_pdid){
		//kprintf("ctx out by different type process: %s, pr_pdid = %d, oldpdid: %d\n",ptnew->prname,ptnew->pr_pdid,ptold->pr_pdid);
		//kprintf("should cr3 = %u\n",fid_2_vaddr(ptnew->pr_pdid));
		//kprintf("vaddr of pr_pdid: %u\n",fid_2_vaddr(ptnew->pr_pdid));
		//kprintf("previous cr3 = %u\n",read_cr3());
		set_cr3(fid_2_vaddr(ptnew->pr_pdid));
		//kprintf("cr3 = %u\n",read_cr3());
	}
	else{
		//kprintf("ctx out by the same type process: %s, pr_pdid = %d\n",ptnew->prname,ptnew->pr_pdid);
	}

	/*
	if( ptnew->pr_pdid != proctab[NULL].pr_pdid){
		if( ((pd_t *)(4247552))[4].pd_base != 1036){
			kprintf("============ pd: %u,   %u   =============      ===========\n",(uint32)&(((pd_t *)(4247552))[4]),(uint32) ((pd_t *)(4247552))[4].pd_base);
		}
		else{
			kprintf("*************************************************\n");
		}
	}
	*/

	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}

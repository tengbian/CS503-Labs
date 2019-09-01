/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */

/* get information of all two groups */
void	get_group_info(
	struct	gprocent *gps1, 
	struct	gprocent *gps2
	)
{
	int s1i = 0;	/* flag for 1st proc in group 1 */
	int s2i = 0;	/* flag for 1st proc in group 2 */
	pid32  ready_i; /* id in ready list */

	gps1->pnum = 0;
	gps2->pnum = 0;

	/* transverse readylist and get priority, firstid, firstPRIOi for each group */
	ready_i = firstid(readylist);
	while(nextid(ready_i) != EMPTY){
		switch(proctab[ready_i].group) {
			case PS1:
				if (ready_i != NULLPROC){
					gps1->pnum ++;
					if (s1i == 0){
						gps1->g_firstid = ready_i;
						s1i ++;
					}
				}
				break;
			case PS2:
				if (ready_i != NULLPROC){
					gps2->pnum ++;
					if (s2i == 0){
						gps2->g_firstid = ready_i;
						s2i ++;
					}
				}
				break;
			default:
				kprintf("System Error: not group 1 or 2!\n\n");
				return;
		}
		ready_i = nextid(ready_i);
	}

	/* get group priority .... */
	grouptab[proctab[currpid].group].gprio_curr = grouptab[proctab[currpid].group].gprio_init; /* current group */
	gps1->gprio_curr = gps1->gprio_curr + gps1->pnum;
	gps2->gprio_curr = gps2->gprio_curr + gps2->pnum;

	return;
}


/* update old process */
void	oldprocess_update(
	struct 	procent *ptold
	)
{
	/* change Pi of current process */
	if (currpid != NULLPROC){
		ptold->Pi = ptold->Pi + (200-preempt)*100/ptold->prprio;
		ptold->PRIOi = INT_MAX - ptold->Pi;
		
		// if old process is blocked
		if (!( (ptold->prstate == PR_CURR) || (ptold->prstate == PR_READY) || (preempt <= 0))){
			ptold->blocked = 1;
		}
	}
	return;
}


/* update new process */
void	newprocess_update(
	struct 	procent *ptnew
	)
{
	// if new process was blocked before, update it's Pi & PRIOi
	if (currpid != NULLPROC){
		if (ptnew->blocked != 0){
			ptnew->blocked = 0;
			if (clktime_ms + clktime*1000  >= ptnew->Pi){
				ptnew->Pi = clktime_ms+clktime*1000;
				ptnew->PRIOi = INT_MAX - ptnew->Pi;
			}
		}
	}
	ptnew->prstate = PR_CURR;
	return;
}

/* select group and process id */
void	group_process_selection(
		struct	gprocent *gps1,
		struct	gprocent *gps2,
		int		*ptr_select_group,
		pid32	*ptr_select_firstid,
		pri16	*ptr_select_firstPRIOi
		)
{
	// judge whether 1 group or 2 groups
	if ((gps1->pnum == 0) || (gps2->pnum == 0)){
		*ptr_select_group = gps1->pnum ? PS1 : PS2;
		*ptr_select_firstid = gps1->pnum ? gps1->g_firstid : gps2->g_firstid;
		*ptr_select_firstPRIOi = proctab[*ptr_select_firstid].PRIOi;
	}
	else{
		*ptr_select_group = (gps1->gprio_curr >= gps2->gprio_curr) ? PS1 : PS2;
		*ptr_select_firstid = (gps1->gprio_curr >= gps2->gprio_curr) ? gps1->g_firstid : gps2->g_firstid;
		*ptr_select_firstPRIOi = proctab[*ptr_select_firstid].PRIOi;
	}
	return;
}
// can be upgraded so that no need to judge whether 0 group or 1/2 groups

void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	// Teng: variable for group 
	struct	gprocent *gps1 = &grouptab[0];
	struct 	gprocent *gps2 = &grouptab[1];
	pid32  	select_firstid;
	pri16  	select_firstPRIOi;
	int  	select_group;

	// initialize selected parameters
	select_firstid = EMPTY;
	select_firstPRIOi = -1;
	select_group = -1;

	/* If rescheduling is deferred, record attempt and return */
	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	// renew info for groups
	get_group_info(gps1,gps2);

	// get ptold
	ptold = &proctab[currpid];


	// judge whether all groups are empty
	if ( (gps1->pnum == 0) && (gps2->pnum == 0) ){ 
		
		//if(currpid == NULLPROC){
		//	return;
		//}		

		// update old process
		oldprocess_update(ptold);
		if (ptold->prstate == PR_CURR) {
			if(ptold->PRIOi > firstkey(readylist)) {
				preempt = QUANTUM;
				return;
			}
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->PRIOi);
		}
	 
		// find new process
		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];

		// update new process
		newprocess_update(ptnew);

		// Rest time slice for process
		preempt = QUANTUM;		
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

		/* Old process returns here when resumed */
		return;

	}
	else{
		// there are 1 or 2 groups

		//selet group and process to pick up
		group_process_selection(gps1, gps2,	&select_group, &select_firstid, &select_firstPRIOi);

		// update old process
		oldprocess_update(ptold);
		if (ptold->prstate == PR_CURR){
			if ( (ptold->group == select_group) && (ptold->PRIOi > select_firstPRIOi) ) {
				preempt = QUANTUM;
				return;
			}
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->PRIOi);
		}

		// find new process
		currpid = select_dequeue(select_firstid);
		ptnew = &proctab[currpid];


		// update new process
		newprocess_update(ptnew);

		// Reset time slice for process
		preempt = QUANTUM;		
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

		/* Old process returns here when resumed */
		return;

	}// end if




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

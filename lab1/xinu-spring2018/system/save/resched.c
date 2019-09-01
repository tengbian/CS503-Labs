/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	// Teng: variable for group 
	struct gprocent *gps1;
	struct gprocent *gps2;
	pid32  ready_id;
	pid32  select_firstid;
	pid32  select_g1;
	pid32  select_g2;
	int	   s1i=0;
	int	   s2i=0;
	pri16  select_firstPRIOi;
	int    group_curr;

	gps1 = &grouptab[0];
	gps2 = &grouptab[1];

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	
	// Teng: select a group
	gps1->pnum = 0;
	gps2->pnum = 0;

	/* transverse readylist and get priority for each group */
	ready_id = firstid(readylist);
	while(nextid(ready_id) != EMPTY){
		switch(proctab[ready_id].group) {
			case PS1:
				if (strcmp(proctab[ready_id].prname,"prnull") != 0){
					gps1->pnum ++;
					if (s1i == 0){
						select_g1 = ready_id;
						s1i ++;
					}
				}
				break;
			case PS2:
				gps2->pnum ++;
				if (s2i == 0){
					select_g2 = ready_id;
					s2i ++;
				}
				break;
			default:
				kprintf("System Error: not group 1 or 2!\n\n");
				return;
		}
		ready_id = nextid(ready_id);
	}

	ptold = &proctab[currpid];

	/* get group priority .... */
	grouptab[ptold->group].gprio_curr = grouptab[ptold->group].gprio_init;
	gps1->gprio_curr = gps1->gprio_curr + gps1->pnum;
	gps2->gprio_curr = gps2->gprio_curr + gps2->pnum;


	/* Point to process table entry for the current (old) process */

	/* if no ready process, remain current
	 * if ready process in 1 group, do it as before 
	 * if ready process in 2 group, select 1 group and do context switch */
	if ( (gps1->pnum == 0) && (gps2->pnum == 0) ){ // in this case, one process + null would run 


		// or only null process would run
		group_curr = ptold->group;

		/* change Pi of current process */
		if (strcmp(ptold->prname,"prnull") != 0){
			ptold->Pi = ptold->Pi + (200-preempt)*100/ptold->prprio;
			ptold->PRIOi = INT_MAX - ptold->Pi;
		}

		if (ptold->prstate == PR_CURR) {
			if(ptold->PRIOi > firstkey(readylist)) {
				return;
			}
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->PRIOi);
			//if ( ((ptold->prstate == PR_CURR) || (ptold->prstate == PR_READY)) && (preempt < QUANTUM)){
			if ( (ptold->prstate == PR_CURR) || (ptold->prstate == PR_READY) || (preempt <= 0)){
			}
			else{
				ptold->blocked = 1;
			}
		} 

		/* Force context switch to highest priority ready process */
		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];

		// Update PRIOi/Pi (exclude PRIOi/Pi update for  null process)
		// if unblocked: keep PRIOi/Pi ; if new or blocked, get max(Pi,TIME) and update PRIOi
		if (strcmp(ptnew->prname,"prnull") != 0){
			//if ( ((ptnew->prstate == PR_CURR) || (ptnew->prstate == PR_READY)) && (preempt == QUANTUM)){
			if (ptnew->blocked == 0){
				// unblocked, keep PRIOi/Pi
			}
			else{
				ptnew->blocked = 0;
				// blocked, choose from TIME and Pi
				// kprintf("Not null process, bloked, Pi %d, TIME: %d\n",ptnew->Pi,TIME);
				if (clktime_ms + clktime*1000  >= ptnew->Pi){
					ptnew->Pi = clktime_ms+clktime*1000;
					ptnew->PRIOi = INT_MAX - ptnew->Pi;
				}
				// kprintf("Not null process, bloked, Pi %d, TIME: %d\n",ptnew->Pi,TIME);
			}
		}

		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;		/* Reset time slice for process	*/
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

		/* Old process returns here when resumed */
		return;

	}
	else{
		// there are 1 or 2 groups
		if ((gps1->pnum == 0) || (gps2->pnum == 0)){
			group_curr = gps1->pnum ? PS1 : PS2;
			select_firstid = gps1->pnum ? select_g1 : select_g2;
		}
		else{
			group_curr = (gps1->gprio_curr >= gps2->gprio_curr) ? PS1 : PS2;
			select_firstid = (gps1->gprio_curr >= gps2->gprio_curr) ? select_g1 : select_g2;
		}
		select_firstPRIOi = proctab[select_firstid].PRIOi;


		/* change Pi of current process, no matter it's CURR or not (if not null)*/
		if (strcmp(ptold->prname,"prnull") != 0){
			ptold->Pi = ptold->Pi + (200-preempt)*100/ptold->prprio;
			ptold->PRIOi = INT_MAX - ptold->Pi;

			if ( (ptold->prstate == PR_CURR) || (ptold->prstate == PR_READY) || (preempt <= 0 )){
			}
			else{
				ptold->blocked = 1;
			}
		}


		/* Point to process table entry for the current (old) process */
		if (ptold->prstate == PR_CURR){
			/* Old process keep current */
			if ( (ptold->group == group_curr) && (ptold->PRIOi > select_firstPRIOi) ) {
				return;
			}
			
			/* Old process will no longer remain current */
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->PRIOi);
		}


		/* Force context switch to highest priority ready process */
		currpid = select_dequeue(select_firstid);
		ptnew = &proctab[currpid];
		//kprintf("ptnew->blocked: %d\n",ptnew->blocked);
		//kprintf("Anything Anything Anything Anything Anyting Anything\n");

		// Update PRIOi/Pi (exclude PRIOi/Pi update for  null process)
		// if unblocked: keep PRIOi/Pi ; if new or blocked, get max(Pi,TIME) and update PRIOi
		if (strcmp(ptnew->prname,"prnull") != 0){
			if (ptnew->blocked != 0){
				ptnew->blocked = 0;
				// blocked, choose from TIME and Pi
				//kprintf("id: %d, Anything\n",currpid);
				//kprintf("Anything\n");
				uint32 totaltime = (1000-ctr1000)+clktime*1000;
				if ( totaltime >= ptnew->Pi){
					ptnew->Pi = totaltime;
					ptnew->PRIOi = INT_MAX - ptnew->Pi;
				}
				//kprintf("Not null process, bloked, updated Pi %d, TIME: %d\n",ptnew->Pi,TIME);
			}

		}

		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;		/* Reset time slice for process	*/
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

		/* Old process returns here when resumed */
		return;

	}// end choose from 0, 1, 2 group



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

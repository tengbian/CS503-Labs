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
	pri16  select_firstPRIOi;
	int    group_curr;
	uint32 TIME;

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
	//kprintf("%d\n",ready_id);
	kprintf("Current process: %s, id = %d\n",proctab[currpid].prname,currpid);
	kprintf("=========  Processes in Readylist  ==========\n");
	while(nextid(ready_id) != EMPTY){
		kprintf("prname: %s, ready_id: %d, Pi: %d, group: %d\n",proctab[ready_id].prname, ready_id, proctab[ready_id].Pi, proctab[ready_id].group);
		switch(proctab[ready_id].group) {
			case PS1:
				if (strcmp(proctab[ready_id].prname,"prnull") != 0){
					gps1->pnum ++;
				}
				break;
			case PS2:
				gps2->pnum ++;
				break;
			default:
				kprintf("\nSystem Error: not group 1 or 2!\n");
				kprintf("System Error: not group 1 or 2!\n");
				kprintf("System Error: not group 1 or 2!\n\n");
				return;
		}
		ready_id = nextid(ready_id);
	}
	kprintf("============================================\n");

	ptold = &proctab[currpid];

	/* get group priority .... */
	grouptab[ptold->group].gprio_curr = grouptab[ptold->group].gprio_init;
	gps1->gprio_curr = gps1->gprio_curr + gps1->pnum;
	gps2->gprio_curr = gps2->gprio_curr + gps2->pnum;
	kprintf("g1: num= %d, gprio= %d\ng2: num= %d, gprio= %d\n",gps1->pnum,gps1->gprio_curr,gps2->pnum,gps2->gprio_curr);


	/* Point to process table entry for the current (old) process */
	kprintf("Current group: %d, id: %d, Pi: %d\n",ptold->group,currpid,ptold->Pi);

	/* if no ready process, remain current
	 * if ready process in 1 group, do it as before 
	 * if ready process in 2 group, select 1 group and do context switch */
	if ( (gps1->pnum == 0) && (gps2->pnum == 0) ){ // in this case, one process + null would run 
												   // or only null process would run
		//kprintf("Nothing in any group, only 'null' process\n");
		group_curr = ptold->group;

		kprintf("All 0\n");
		kprintf("For old process:\n");
		/* change Pi of current process */
		if (strcmp(ptold->prname,"prnull") != 0){
			kprintf("Runtime: %d, current preempt: %d\n",200-preempt%QUANTUM, preempt);
			ptold->Pi = ptold->Pi + (200-preempt%QUANTUM)*100/ptold->prprio;
			ptold->PRIOi = INT_MAX - ptold->Pi;
		}

		if (ptold->prstate == PR_CURR) {
			if(ptold->PRIOi > firstkey(readylist)) {
				kprintf("Keep current process, updated Pi: %d\n\n",ptold->Pi);
				return;
			}
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->PRIOi);
			kprintf("Replace current process, updated Pi: %d\n",ptold->Pi);
			//if ( ((ptold->prstate == PR_CURR) || (ptold->prstate == PR_READY)) && (preempt < QUANTUM)){
			if ( (ptold->prstate == PR_CURR) || (ptold->prstate == PR_READY) || (preempt == QUANTUM)){
				kprintf("Old process not get blocked.\n");
			}
			else{
				ptold->blocked = 1;
				kprintf("Old process get blocked.\n");
			}
		} 
		kprintf("Replace current process (PR_CURR), updated Pi: %d\n",ptold->Pi);

		/* Force context switch to highest priority ready process */
		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];

		// Update PRIOi/Pi (exclude PRIOi/Pi update for  null process)
		// if unblocked: keep PRIOi/Pi ; if new or blocked, get max(Pi,TIME) and update PRIOi
		kprintf("For new process:\n");
		if (strcmp(ptnew->prname,"prnull") != 0){
			//if ( ((ptnew->prstate == PR_CURR) || (ptnew->prstate == PR_READY)) && (preempt == QUANTUM)){
			if (ptnew->blocked == 0){
				// unblocked, keep PRIOi/Pi
				kprintf("Not null process, unblocked, keep old Pi: %d\n",ptnew->Pi);
			}
			else{
				ptnew->blocked = 0;
				// blocked, choose from TIME and Pi
				TIME = clktime_ms+clktime*1000; 
				kprintf("Not null process, blocked, Pi: %d, TIME: %d\n",ptnew->Pi,TIME);
				if (TIME >= ptnew->Pi){
					ptnew->Pi = TIME;
					ptnew->PRIOi = INT_MAX - ptnew->Pi;
				}
				kprintf("Not null process, blocked, updated Pi: %d, TIME: %d\n",ptnew->Pi,TIME);
			}
		}
		kprintf("New process Pi/PRIOi updated.\n\n");

		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;		/* Reset time slice for process	*/
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

		/* Old process returns here when resumed */
		kprintf("\n");
		return;

	}
	else{
		// there are 1 or 2 groups
		kprintf("Not all 0\n");
		if ((gps1->pnum == 0) || (gps2->pnum == 0)){
			group_curr = gps1->pnum ? PS1 : PS2;
		}
		else{
			group_curr = (gps1->gprio_curr >= gps2->gprio_curr) ? PS1 : PS2;
		}
		kprintf("Select group: group_curr: %d\n",group_curr);

		// initialize select group thing
		select_firstid = EMPTY;
		select_firstPRIOi = -1;
		// find out the first key for group_curr
		ready_id = firstid(readylist);
		while(nextid(ready_id) != EMPTY){// at least 1 loop, promised
				if(proctab[ready_id].group == group_curr){
					select_firstid = ready_id;
					select_firstPRIOi = proctab[ready_id].PRIOi;
					break;
				}
				ready_id = nextid(ready_id);
		}
		kprintf("Select id: %d, Pi: %d, PRIOi: %d\n",select_firstid,proctab[ready_id].Pi,select_firstPRIOi);

		kprintf("For old process:-----------\n");
		/* change Pi of current process, no matter it's CURR or not (if not null)*/
		if (strcmp(ptold->prname,"prnull") != 0){
			kprintf("Runtime: %d, current preempt: %d\n",200-preempt%QUANTUM, preempt);
			ptold->Pi = ptold->Pi + (200-preempt%QUANTUM)*100/ptold->prprio;
			kprintf("Runtime: %d, current preempt: %d\n",200-preempt%QUANTUM, preempt);//Teng debug
			ptold->PRIOi = INT_MAX - ptold->Pi;

			if ( (ptold->prstate == PR_CURR) || (ptold->prstate == PR_READY) || (preempt == QUANTUM)){
				kprintf("Old process not get blocked.\n");
			}
			else{
				ptold->blocked = 1;
				kprintf("Old process get blocked.\n");
			}
		}


		/* Point to process table entry for the current (old) process */
		if (ptold->prstate == PR_CURR){
			/* Old process keep current */
			if ( (ptold->group == group_curr) && (ptold->PRIOi > select_firstPRIOi) ) {
				kprintf("Keep current process, update Pi: %d\n\n",ptold->Pi);
				return;
			}
			
			/* Old process will no longer remain current */
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->PRIOi);
			kprintf("Replace current process (PR_CURR), updated Pi: %d\n",ptold->Pi);
		}


		/* Force context switch to highest priority ready process */
		currpid = select_dequeue(select_firstid);
		ptnew = &proctab[currpid];

		// Update PRIOi/Pi (exclude PRIOi/Pi update for  null process)
		// if unblocked: keep PRIOi/Pi ; if new or blocked, get max(Pi,TIME) and update PRIOi
		kprintf("For new process:-----------\n");
		if (strcmp(ptnew->prname,"prnull") != 0){
			//if ( ((ptnew->prstate == PR_CURR) || (ptnew->prstate == PR_READY)) && (preempt == QUANTUM)){
			if (ptnew->blocked == 0){
				// unblocked, keep PRIOi/Pi
				kprintf("Not null process, unblocked, keep old Pi: %d\n",ptnew->Pi);
			}
			else{
				ptnew->blocked = 0;
				// blocked, choose from TIME and Pi
				TIME = clktime_ms+clktime*1000; 
				kprintf("Not null process, blocked, Pi: %d, TIME: %d\n",ptnew->Pi, TIME);
				if (TIME >= ptnew->Pi){
					ptnew->Pi = TIME;
					ptnew->PRIOi = INT_MAX - ptnew->Pi;
				}
				kprintf("Not null process, blocked, updated Pi: %d, TIME: %d\n",ptnew->Pi, TIME);
			}
		}
		kprintf("New process Pi/PRIOi updated.\n\n");

		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;		/* Reset time slice for process	*/
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

		/* Old process returns here when resumed */
		kprintf("\n");
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

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
	pri16  select_firstprio;
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
	while(nextid(ready_id) != EMPTY){
		kprintf("prname: %s, ready_id: %d, priority: %d, group: %d\n",proctab[ready_id].prname, ready_id, proctab[ready_id].prprio, proctab[ready_id].group);
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
				kprintf("\nSystem Error!\n");
				kprintf("System Error!\n");
				kprintf("System Error!\n\n");
				return;
		}
		ready_id = nextid(ready_id);
	}
	kprintf("group1: %d\ngroup2: %d\n",gps1->pnum,gps2->pnum);

	/* Point to process table entry for the current (old) process */
	ptold = &proctab[currpid];
	kprintf("Current id: %d\nCurrent group: %d\n",currpid,ptold->group);

	/* get group priority .... */
	grouptab[ptold->group].gprio_curr = grouptab[ptold->group].gprio_init;
	gps1->gprio_curr = gps1->gprio_curr + gps1->pnum;
	gps2->gprio_curr = gps2->gprio_curr + gps2->pnum;
	kprintf("gps1->gprio_curr: %d\n",gps1->gprio_curr);
	kprintf("gps2->gprio_curr: %d\n",gps2->gprio_curr);

	/* if no ready process, remain current
	 * if ready process in 1 group, do it as before 
	 * if ready process in 2 group, select 1 group and do context switch */
	if ( (gps1->pnum == 0) || (gps2->pnum == 0) ){
		
		// get the current group; for debug
		if ( (gps1->pnum == 0) && (gps2->pnum == 0) ){
			kprintf("nothing in any group\n");
			group_curr = proctab[currpid].group;
		}
		else{
			group_curr = gps1->pnum ? PS1 : PS2;
			kprintf("only 1 group: %d\n",group_curr);
		}

		// as origianl
		if (ptold->prstate == PR_CURR) {
			if(ptold->prprio > firstkey(readylist)) {
				kprintf("keep it\n\n");
				return;
			}
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->PRIOi);
		} 
		currpid = dequeue(readylist);








	}
	else{
		group_curr = (gps1->gprio_curr >= gps2->gprio_curr) ? PS1 : PS2;

		// initialize select group thing
		select_firstid = EMPTY;
		select_firstprio = 0;
		// find out the first key for group_curr
		ready_id = firstid(readylist);
		while(nextid(ready_id) != EMPTY){// at least 1 loop, promised
				kprintf("ready_id: %d\n",ready_id);
				if(proctab[ready_id].group == group_curr){
					select_firstid = ready_id;
					select_firstprio = proctab[ready_id].prprio;
					break;
				}
				ready_id = nextid(ready_id);
			}
		kprintf("2 groups, select: %d\n",group_curr);
		kprintf("select_firstid: %d\n",select_firstid);
		kprintf("select_firstprio: %d\n",select_firstprio);

		/* Point to process table entry for the current (old) process */
		if (ptold->prstate == PR_CURR){
			if ( (ptold->group == group_curr) && (ptold->prprio > select_firstprio) ) {
				kprintf("keep it\n\n");
				return;
			}
			
			/* Old process will no longer remain current */
			/* if not null */
			/* change Pi of current process first*/
			if (strcmp(ptold->prname,"prnull") != 0){
				ptold->Pi = ptold->Pi + (200-preempt%QUANTUM)*100/ptold->prprio;
				ptold->PRIOi = INT_MAX - ptold->Pi;
			}
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->PRIOi);
		}
		currpid = select_dequeue(select_firstid);
	}



	kprintf("\n");
	/* Force context switch to highest priority ready process */
	ptnew = &proctab[currpid];

	// if not null process
	// if unblocked: Pi_temp = Pi + 100*t/Ri
	// if new or blocked, get max(T,Pi)

	if (strcmp(ptnew->prname,"prnull") != 0){
		if ( ((ptnew->prstate == PR_CURR) || (ptnew->prstate == PR_READY)) && (preempt == QUANTUM)){
		}
		else{
			TIME = clktime_ms+clktime*1000; 
			if (TIME >= ptnew->PRIOi){
				ptnew->PRIOi = TIME;
			}
			kprintf("ptnew->prstate: %d, preempt: %d\n",ptnew->prstate, preempt);
			kprintf("TIME: %d\n",TIME);
		}
	}

	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
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

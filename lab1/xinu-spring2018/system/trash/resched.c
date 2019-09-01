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

	gps1 = &grouptab[0];
	gps2 = &grouptab[1];
//	intmask mask; // what is mask for?


	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}
	
	// Teng: select a group
	gps1->pnum = 0;
	gps2->pnum = 0;

	// Teng Debug: check state
	/*
	kprintf("id 0: %s, state: %d\n",proctab[0].prname,proctab[0].prstate);
	kprintf("id 1: %s, state: %d\n",proctab[1].prname,proctab[1].prstate);
	kprintf("id 2: %s, state: %d\n",proctab[2].prname,proctab[2].prstate);
	kprintf("id 3: %s, state: %d\n",proctab[3].prname,proctab[3].prstate);
	*/


	/* transverse readylist and get priority for each group */
	ready_id = firstid(readylist);
	while(nextid(ready_id) != EMPTY){
		kprintf("prname: %s, ready_id: %d, priority: %d, group: %d\n",proctab[ready_id].prname, ready_id, proctab[ready_id].prprio, proctab[ready_id].group);
		switch(proctab[ready_id].group) {
			case PS1:
				gps1->pnum ++;
				break;
			case PS2:
				gps2->pnum ++;
				break;
			default:
				printf("\nSystem Error!\n");
				printf("System Error!\n");
				printf("System Error!\n\n");
				return;
		}
		ready_id = nextid(ready_id);
	}
	kprintf("group1: %d\ngroup2: %d\n",gps1->pnum,gps2->pnum);




	/* calculate */
	gps1->gprio_curr = gps1->gprio_init + gps1->pnum;
	gps2->gprio_curr = gps2->gprio_init + gps2->pnum;


	if(gps1->pnum == 0){
		group_curr = PS2;}
	else if(gps2->pnum == 0){
		group_curr =PS1;}
	else{
		if(gps1->gprio_curr >= gps2->gprio_curr){
			group_curr = PS1;}
		else{
			group_curr = PS2;}
	}



	kprintf("gps1->gprio_curr: %d\n",gps1->gprio_curr);
	kprintf("gps2->gprio_curr: %d\n",gps2->gprio_curr);
	kprintf("group_curr: %d\n",group_curr);

	// find out the first key for group_curr
	ready_id = firstid(readylist);

	// default select_firstid & select_firstprio
	// if empty queue, no select group firstid
	// if not empty, do while to get select group firstid
	select_firstid = EMPTY;
	select_firstprio = 0;

	while(nextid(ready_id) != EMPTY){
		kprintf("ready_id: %d\n",ready_id);
		if(proctab[ready_id].group == group_curr){
			select_firstid = ready_id;
			select_firstprio = proctab[ready_id].prprio;
			break;
		}
		ready_id = nextid(ready_id);
	}
	
	kprintf("select_firstid: %d\n",select_firstid);
	kprintf("select_firstprio: %d\n",select_firstprio);

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];


	if (ptold->prstate == PR_CURR){ /* Process remains eligibla */
		if ( (ptold->group == group_curr) && (ptold->prprio > select_firstprio)){
			printf("keep it\n");
			return;
		}

		/* Old process will no longer remain current */
		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	kprintf("\n");

	/* Force context switch to highest priority ready process */

	if (select_firstid != EMPTY){
		currpid = select_dequeue(select_firstid);
	}
	else{
		currpid = dequeue(readylist);
	}
	ptnew = &proctab[currpid];
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

/* initialize.c - nulluser, sysinit, sizmem */

/* Handle system initialization and become the null process */

#include <xinu.h>
#include <string.h>
#include <lab3.h>

extern	void	start(void);	/* Start of Xinu code			*/
extern	void	*_end;		/* End of Xinu code			*/

/* Function prototypes */

extern	void main(void);	/* Main is the first process created	*/
extern	void xdone(void);	/* System "shutdown" procedure		*/
static	void sysinit(); 	/* Internal system initialization	*/
extern	void meminit(void);	/* Initializes the free memory list	*/

/* Lab3. initializes data structures and necessary set ups for paging */
static	void initialize_paging();

/* Declarations of major kernel variables */

struct	procent	proctab[NPROC];	/* Process table			*/
struct	sentry	semtab[NSEM];	/* Semaphore table			*/
struct	memblk	memlist;	/* List of free memory blocks		*/

/* Lab3. frames metadata handling */
frame_md_t frame_md;// not by Teng
// Teng
frame_t frametab[NFRAMES];

// fifo list
int32 fifohead;
int32 fifotail;
fifo_entry fifolist[NFRAMES];

// gca dirtylist
uint32 gcapin = 0;
int32 gca_dirtylist[NFRAMES]; // 1 or 0

bsmap	bsmaptab[MAX_BS_ENTRIES];// 8
//inverted_frame inverted_frametab[NFRAMES];

int32 num_faults;
sid32 pf_sem;


/* Active system status */

int	prcount;		/* Total number of live processes	*/
pid32	currpid;		/* ID of currently executing process	*/


bool8   PAGE_SERVER_STATUS;    /* Indicate the status of the page server */
sid32   bs_init_sem;

/*------------------------------------------------------------------------
 * nulluser - initialize the system and become the null process
 *
 * Note: execution begins here after the C run-time environment has been
 * established.  Interrupts are initially DISABLED, and must eventually
 * be enabled explicitly.  The code turns itself into the null process
 * after initialization.  Because it must always remain ready to execute,
 * the null process cannot execute code that might cause it to be
 * suspended, wait for a semaphore, put to sleep, or exit.  In
 * particular, the code must not perform I/O except for polled versions
 * such as kprintf.
 *------------------------------------------------------------------------
 */

void	nulluser()
{
	struct	memblk	*memptr;	/* Ptr to memory block		*/
	uint32	free_mem;		/* Total amount of free memory	*/

	/* Initialize the system */

	sysinit();

  // Lab3
	initialize_paging();

	kprintf("\n\r%s\n\n\r", VERSION);

	/* Output Xinu memory layout */
	free_mem = 0;
	for (memptr = memlist.mnext; memptr != NULL;
						memptr = memptr->mnext) {
		free_mem += memptr->mlength;
	}
	kprintf("%10d bytes of free memory.  Free list:\n", free_mem);
	for (memptr=memlist.mnext; memptr!=NULL;memptr = memptr->mnext) {
	    kprintf("           [0x%08X to 0x%08X]\r\n",
		(uint32)memptr, ((uint32)memptr) + memptr->mlength - 1);
	}

	kprintf("%10d bytes of Xinu code.\n",
		(uint32)&etext - (uint32)&text);
	kprintf("           [0x%08X to 0x%08X]\n",
		(uint32)&text, (uint32)&etext - 1);
	kprintf("%10d bytes of data.\n",
		(uint32)&ebss - (uint32)&data);
	kprintf("           [0x%08X to 0x%08X]\n\n",
		(uint32)&data, (uint32)&ebss - 1);

	/* Create the RDS process */

	rdstab[0].rd_comproc = create(rdsprocess, RD_STACK, RD_PRIO,
					"rdsproc", 1, &rdstab[0]);
	if(rdstab[0].rd_comproc == SYSERR) {
		panic("Cannot create remote disk process");
	}
	resume(rdstab[0].rd_comproc);

	/* Enable interrupts */

	enable();

	/* Create a process to execute function main() */

	kprintf("Null process begins\n");

	resume (
	   create((void *)main, INITSTK, INITPRIO, "Main process", 0,
           NULL));

	/* Become the Null process (i.e., guarantee that the CPU has	*/
	/*  something to run when no other process is ready to execute)	*/

	while (TRUE) {
		;		/* Do nothing */
	}

}

/*------------------------------------------------------------------------
 *
 * sysinit  -  Initialize all Xinu data structures and devices
 *
 *------------------------------------------------------------------------
 */
static	void	sysinit()
{
	int32	i;
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	struct	sentry	*semptr;	/* Ptr to semaphore table entry	*/

	/* Platform Specific Initialization */

	platinit();

	/* Initialize the interrupt vectors */

	initevec();

	/* Initialize free memory list */

	meminit();

	/* Initialize system variables */

	/* Count the Null process as the first process in the system */

	prcount = 1;

	/* Scheduling is not currently blocked */

	Defer.ndefers = 0;

	/* Initialize process table entries free */

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		prptr->prstate = PR_FREE;
		prptr->prname[0] = NULLCH;
		prptr->prstkbase = NULL;
		prptr->prprio = 0;
		prptr->pr_pdid = -1;
	}

	/* Initialize the Null process entry */

	prptr = &proctab[NULLPROC];
	prptr->prstate = PR_CURR;
	prptr->prprio = 0;
	strncpy(prptr->prname, "prnull", 7);
	prptr->prstkbase = getstk(NULLSTK);
	prptr->prstklen = NULLSTK;
	prptr->prstkptr = 0;
	currpid = NULLPROC;

	/* Initialize semaphores */

	for (i = 0; i < NSEM; i++) {
		semptr = &semtab[i];
		semptr->sstate = S_FREE;
		semptr->scount = 0;
		semptr->squeue = newqueue();
	}

	/* Initialize buffer pools */

	bufinit();

	/* Create a ready list for processes */

	readylist = newqueue();

	/* Initialize the real time clock */

	clkinit();

#ifndef QEMU
	for (i = 0; i < NDEVS; i++) {
		init(i);
	}
#else
    init(QEMUCONSOLE);
    init(QEMURDISK);
#endif

	PAGE_SERVER_STATUS = PAGE_SERVER_INACTIVE;
	bs_init_sem = semcreate(1);

	return;
}



static void initialize_paging()
{
	/* LAB3 TODO */
	//kprintf("----------- Here ----------\n");

	// 1. Initialize all necessary data structures
	//frame_t *frameptr;
	uint32 curr_pdaddr;
	initialize_bs();// only in NULL process
	initialize_frame();// only in NULL process
	initialize_fifolist();
	initialize_gca_dirtylist();

	// 2. Allocate and initialize a page directory for the NULL process
	// 3-4. Create 4 global PT, 1 device PT for NULL process
	curr_pdaddr = (uint32) pd_allocate();
	proctab[currpid].pr_pdid = vaddr_2_fid(curr_pdaddr);
	//kprintf("The base is : %u\n", (uint32) curr_pdaddr/PAGECHUNKSIZE);// upper 20 bits
	//kprintf("prpdid: %d\n",vaddr_2_fid(curr_pdaddr));

	// 5. Set CR3 register to the page directory of NULL process
	set_cr3(curr_pdaddr);// lower 12 bits of pd is 0s

	// 6. Install the page fualt interrupt service routine
	set_evec( ((uint32) 14), (uint32) pfisr);
	//kprintf("CR3 is %u now, set success.\n",read_cr3());

	// 7. Enable paging
	num_faults = 0;
	pf_sem = semcreate(1);
	//kprintf("CR0 is %u\n",read_cr0());
	enablepaging();
	//kprintf("CR0 is %u\n",read_cr0());
	kprintf("page initialization pass\n");

	return;
}

int32	stop(char *s)
{
	kprintf("%s\n", s);
	kprintf("looping... press reset\n");
	while(1)
		/* Empty */;
}

int32	delay(int n)
{
	DELAY(n);
	return OK;
}

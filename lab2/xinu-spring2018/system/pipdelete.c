#include <xinu.h>

status pipdelete(did32 devpipe) {
    // LAB2: TODO
	
	intmask mask;
	struct pipe_t *pipptr;

	mask = disable();

	if(isbaddev(devpipe)){
		kprintf("Bad device\n");
		restore(mask);
		return SYSERR;
	}
	pipid32 pip = did32_to_pipid32(devpipe);
	pipptr = &pipe_tables[pip];

	if(currpid != pipptr->owner && currpid != pipptr->writer && currpid != pipptr->reader){
		kprintf("Only pipe owner can delete pipe\n");
		restore(mask);
		return SYSERR;
	}
	
	pipptr->state = PIPE_FREE;
	pipptr->piphead = pipptr->pipbuffer;
	pipptr->piptail = pipptr->pipbuffer;
	semdelete(pipptr->pipwsem);
	semdelete(pipptr->piprsem);
	// reset head/tail?

	restore(mask);
	return OK;
}




#include <xinu.h>

devcall pipputc(struct dentry *devptr, char ch) {
    // LAB2: TODO
	
	intmask mask;
	pipid32 pip;
	struct pipe_t *pipptr;

	mask = disable();

	if(isbaddev(devptr->dvnum)){
		kprintf("Bad device\n");
		restore(mask);
		return SYSERR;
	}

	pip = did32_to_pipid32(devptr->dvnum);
	pipptr = &pipe_tables[pip];

	wait(pipptr->pipwsem);

	// in case reader exits when writing is waiting (sem if full/not full)
	if(pipptr->state == PIPE_WT_CONN && semtab[pipptr->pipwsem].sstate == S_FREE){
		//kprintf("Escape from waiting to write\n");
		restore(mask);
		return SYSERR;
	}



	*pipptr->piphead++ = ch;
	// Wrap around to beginning of buffer, if needed
	if(pipptr->piphead >= &pipptr->pipbuffer[PIPE_SIZE]){
		pipptr->piphead = pipptr->pipbuffer;
	}

	//kprintf("put: %d, %d (from %d)\n",(int) ch, (int) pipptr->piphead, (int)pipptr->pipbuffer);

	signal(pipptr->piprsem);
	restore(mask);
    return OK;
}


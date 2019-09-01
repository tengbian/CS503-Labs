#include <xinu.h>

status pipdisconnect(did32 devpipe) {
    // LAB2: TODO
	
	intmask mask;
	struct pipe_t *pipptr;
	mask = disable();

	if(isbaddev(devpipe)){
		kprintf("Bad device\n");
		return SYSERR;
	}
	pipid32 pip = did32_to_pipid32(devpipe);
	pipptr = &pipe_tables[pip];

	if(currpid != pipptr->writer && currpid != pipptr->reader){
		kprintf("Called by %d, while writer=%d, reader=%d\n",currpid,pipptr->writer,pipptr->reader);
		kprintf("Pipedisconnect can only be called by its reader/writer\n");
		return SYSERR;
	}

	if(pipptr->state == PIPE_CONNECTED){
		if(currpid == pipptr->writer){
			//kprintf("Writer is disconncected\n");
			pipptr->state = PIPE_RD_CONN;

			// if reader is still waiting for new  
			//if(pipptr->piphead == pipptr->piptail){
			if(semtab[pipptr->piprsem].scount < 0){
				//kprintf("r sem delete\n");
				semdelete(pipptr->piprsem);
			}
		}
		else if(currpid == pipptr->reader){
			//kprintf("Reader is disconncected\n");
			pipptr->state = PIPE_WT_CONN;
			//kprintf("w sem delete\n");
			semdelete(pipptr->pipwsem);
		}
	}
	else if( 	(pipptr->state == PIPE_RD_CONN && currpid == pipptr->reader)
			||	(pipptr->state == PIPE_WT_CONN && currpid == pipptr->writer) 
			){
		//kprintf("Totally disconnect: %d\n",pip);
		//

		pipdelete(devpipe);

		/*
		pipptr->state = PIPE_FREE;
		pipptr->piphead = pipptr->pipbuffer;
		pipptr->piptail = pipptr->pipbuffer;
		semdelete(pipptr->piprsem);
		semdelete(pipptr->pipwsem);
		*/
		//pipptr->pipwsem = semcreate(PIPE_SIZE);
		//pipptr->piprsem = semcreate(0);
		// or can we use pipdelete here?
	}
	else{
		kprintf("Wrong use of disconnect\n");
		restore(mask);
		return SYSERR;
	}

	restore(mask);
	return OK;
}

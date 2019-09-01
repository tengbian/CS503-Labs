#include <xinu.h>

pipid32 did32_to_pipid32(did32 devpipe){
	ASSERT(devpipe >= PIPELINE0 && devpipe <= PIPELINE9);
	return devpipe - PIPELINE0;
}

status pipconnect(did32 devpipe , pid32 writer, pid32 reader) {
    // LAB2: TODO
	struct pipe_t *pipptr;
	intmask mask;
	mask = disable();

	pipid32 pip = did32_to_pipid32(devpipe);
	pipptr = &pipe_tables[pip];

	if(isbaddev(devpipe)){
		kprintf("Bad device\n");
		restore(mask);
		return SYSERR;
	}
	if(isbadpid(writer) || isbadpid(reader)){
		kprintf("Bad writer or reader, %d, %d\n",writer,reader);
		restore(mask);
		return SYSERR;
	}
	if(pipptr->state != PIPE_USED){
		kprintf("Pipe state not PIPE_USED\n");
		restore(mask);
		return SYSERR;
	}
	if(writer == reader){
		kprintf("Writer and Reader are the same\n");
		restore(mask);
		return SYSERR;
	}

	pipptr->state = PIPE_CONNECTED;
	pipptr->writer = writer;
	pipptr->reader = reader;

	restore(mask);
	return OK;
}

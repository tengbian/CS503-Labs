#include <xinu.h>

did32 pipid32_to_did32(pipid32 pip){
	ASSERT(pip >= 0 && pip <=9);
	return PIPELINE0 + pip;
}

did32 pipcreate() {
	intmask mask;
	int pi;
	struct pipe_t *pipptr;

	mask = disable();

	pi = 0;
	while(pi < MAXPIPES){
		pipptr = &pipe_tables[pi];
		if(pipptr->state == PIPE_FREE){
			pipptr->state = PIPE_USED;
			pipptr->owner = currpid;
			pipptr->piphead = pipptr->pipbuffer;
			pipptr->piptail = pipptr->pipbuffer;
			pipptr->pipwsem = semcreate(PIPE_SIZE);
			pipptr->piprsem = semcreate(0);
			restore(mask);
			return pipid32_to_did32(pipptr->pipid);
		}
		pi++;
	}
	
	kprintf("No more free piplines\n");
	restore(mask);
    return SYSERR;
}

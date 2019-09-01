#include <xinu.h>

devcall pipgetc(struct dentry *devptr) {
    // LAB2: TODO
	
	intmask mask;
	struct pipe_t *pipptr;
	pipid32 pip;

	//static int iii = 0;
	//iii++;
	//kprintf("iii: %d\n",iii);

	mask = disable();

	if(isbaddev(devptr->dvnum)){
		kprintf("Bad device\n");
		restore(mask);
		return SYSERR;
	}

	pip = did32_to_pipid32(devptr->dvnum);
	pipptr = &pipe_tables[pip];

	if(pipptr->state != PIPE_CONNECTED && pipptr->state != PIPE_RD_CONN){
		//kprintf("Only reader-connected pipe can get char\n");
		restore(mask);
		return SYSERR;
	}


	//kprintf("There: state = %d, %d, %d, semnum: %d, semcount: %d\n", pipptr->state, (int) pipptr->piphead , (int) pipptr->piptail, pipptr->piprsem, semtab[(pipptr->piprsem)].scount);

	// read until empty
	//if(pipptr->state == PIPE_RD_CONN && pipptr->piphead == pipptr->piptail){
	if(pipptr->state == PIPE_RD_CONN && semtab[(pipptr->piprsem)].scount == 0 ){
		//kprintf("Buffer is empty\n");
		restore(mask);
		return SYSERR;
	}


	wait(pipptr->piprsem);

	// in case writer exits when reader is waiting
	//if(pipptr->state == PIPE_RD_CONN && pipptr->piphead == pipptr->piptail){
	if(pipptr->state == PIPE_RD_CONN && semtab[(pipptr->piprsem)].sstate == S_FREE){ //semtab[(pipptr->piprsem)].scount < 0 ){
		//kprintf("Escape from wait, buffer is empty\n");
		restore(mask);
		return SYSERR;
	}



	char ch = *pipptr->piptail++;

	// Wrap around to begining of buffer, if needed
	if(pipptr->piptail >= &pipptr->pipbuffer[PIPE_SIZE]){
		pipptr->piptail = pipptr->pipbuffer;
	}

	//kprintf("get: %d, %d (from %d)\n",(int) ch, (int) pipptr->piphead, (int)    pipptr->pipbuffer);

	signal(pipptr->pipwsem);

	restore(mask);
    return ch;
}


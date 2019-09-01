#include <xinu.h>

uint32 pipread(struct dentry *devptr, char* buf, uint32 len) {
    // LAB2: TODO
	
	intmask mask;
	//int32 avail;
	int32 nread;

	mask = disable();

	if (len <= 0){
		restore(mask);
		return SYSERR;
	}

	/*
	// this may not be useful
	if (len == 0){
		pipid32 pip = did32_to_pipid32(devptr->dvnum);
		struct pipe_t *pipptr = &pipe_tables[pip];
		avail = semcount(pipptr->piprsem);
		if (avail == 0){
			return 0;
		} else {
			len = avail;
		}
	}
	*/

	for(nread=0; nread < len; nread++){
		*buf++ = (char) pipgetc(devptr);
	}
	
	restore(mask);
	return nread;
}

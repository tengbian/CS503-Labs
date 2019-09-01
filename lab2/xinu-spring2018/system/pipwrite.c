#include <xinu.h>

uint32 pipwrite(struct dentry *devptr, char* buf, uint32 len) {
    // LAB2: TODO
	intmask mask;
	mask = disable();

	for(; len >0; len--){
		pipputc(devptr, *buf++);
	}
	
	restore(mask);
    return OK;
}

/*  main.c  - main */

#include <xinu.h>

void sample_proc_cpu(){
	int i,j;
	int LOOP1 = 10;
	int LOOP2 = 10000000;

//	struct procent *pr = &proctab[currpid];

	int v = 0;
	for (i=0; i<LOOP1; i++){
		for (j=0; j<LOOP2; j++){
			// Note: this loop consumes significant CPU cycles.
			v += i * j;
		}

		kprintf("PID: %d, Loop: %d \n", currpid, i);
	}

	kprintf("==== CPU BOUNDED PID %d ends\n", currpid);
}

process main() {
	int i;
	resched_cntl(DEFER_START);

	chprio(currpid,100);
	//chprio(2,100);
	//chgprio(PS2,14);// still 010101010; PS1: 001001001001001...

	for (i=0; i<4; i++){
		resume(create(sample_proc_cpu, 1024, PS2, 25, "cpu-intense", 0, NULL));
		resume(create(sample_proc_cpu, 1024, PS1, 25, "cpu-intense", 0, NULL));
	}

	resched_cntl(DEFER_STOP);

}



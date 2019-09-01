
#include<xinu.h>

uint32 tmp;

void set_cr3(uint32 n){

	intmask mask;
	mask = disable();
	//kprintf("herecr1\n");
	tmp = n;
	asm("pushl %eax");
	//kprintf("herecr2\n");
	asm("movl tmp, %eax");
	//kprintf("herecr3\n");
	asm("movl %eax, %cr3");
	//kprintf("herecr4\n");
	asm("popl %eax");
	//kprintf("herecr5\n");
	//kprintf("herecr6\n");
	restore(mask);
	return;
}

uint32 read_cr2(void){

	intmask mask;
	uint32 local_tmp;
	mask = disable();
	asm("pushl %eax");
	asm("movl %cr2, %eax");
	asm("movl %eax, tmp");
	asm("popl %eax");
	local_tmp = tmp;
	restore(mask);
	return local_tmp;
}

uint32 read_cr3(void){

	intmask mask;
	uint32 local_tmp;
	mask = disable();
	asm("pushl %eax");
	asm("movl %cr3, %eax");
	asm("movl %eax, tmp");
	asm("popl %eax");
	local_tmp = tmp;
	restore(mask);
	return local_tmp;
}

uint32 read_cr0(void){

	intmask mask;
	uint32 local_tmp;
	mask = disable();
	asm("pushl %eax");
	asm("movl %cr0, %eax");
	asm("movl %eax, tmp");
	asm("popl %eax");
	local_tmp = tmp;
	restore(mask);
	return local_tmp;
}

void enablepaging(void){
	intmask mask;
	mask = disable();

	asm("pushl %ebp");
	asm("movl %esp, %ebp");
	asm("movl %cr0, %eax");
	asm("orl $0x80000000, %eax");
	asm("movl %eax, %cr0");
	asm("movl %ebp, %esp");
	asm("popl %ebp");

	/*
	asm("pushl %eax");
	asm("movl %cr0, %eax");
	asm("orl $0x80000001, %eax");
	asm("movl %eax, %cr0");
	asm("popl %eax");
	kprintf("7\n");
	*/

	restore(mask);
}

void show32(uint32 n){
	int i;
	for(i=31; i>=0; i--){
		kprintf("%02d ",i);
	}
	kprintf("\n");
	for(i=31; i>=0;i--)
		kprintf("%d  ",(n&(1<<i))>>i);
	kprintf("\n");
}



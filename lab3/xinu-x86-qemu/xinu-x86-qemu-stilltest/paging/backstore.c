/* backstore.c */

#include<xinu.h>

// which bs, which page
//void bslookup(int pid, uint32 vaddr){

void initialize_bs(){
	int i;
	intmask mask;
	mask = disable();
	for(i=0;i<MAX_BS_ENTRIES;i++){
		bsmaptab[i].pid = -1;	
		bsmaptab[i].vpage = -1;
		bsmaptab[i].npages = 0;
		bsmaptab[i].store = i;// this is of no use
		bsmaptab[i].state = BS_FREE;
	}
	kprintf("initialization %d backstores succeed\n",MAX_BS_ENTRIES);
	restore(mask);
	return;
}


int32 bslookup_store(int pid, uint32 vaddr){
	int32 i;
	intmask mask;
	mask = disable();

	int32 vpagenum = vaddr_2_vpage(vaddr);
	for(i=0;i<MAX_BS_ENTRIES;i++){
		if(bsmaptab[i].state == BS_USED){
			kprintf("pid: %d, bs_pid: %d, bs_vpage: %d, bs_npages: %d, vpagenum: %d\n",pid,bsmaptab[i].pid, bsmaptab[i].vpage, bsmaptab[i].npages, vpagenum);
			if( bsmaptab[i].pid == pid && (vpagenum >= bsmaptab[i].vpage) && (vpagenum < bsmaptab[i].vpage+bsmaptab[i].npages)  ){
				restore(mask);
				return i;
			}
		}
	}
	kprintf("wrong in bslookup_store\n");
	restore(mask);
	return SYSERR;
}


int32 bslookup_pageoffset(int pid, uint32 vaddr){
	int32 i;
	intmask mask;
	mask = disable();
	int32 vpagenum = vaddr_2_vpage(vaddr);
	for(i=0;i<MAX_BS_ENTRIES;i++){
		if(bsmaptab[i].state == BS_USED){
			if( bsmaptab[i].pid == pid && (vpagenum >= bsmaptab[i].vpage) && (vpagenum < bsmaptab[i].vpage+bsmaptab[i].npages)  ){
				restore(mask);
				return (vpagenum-bsmaptab[i].vpage);
			}
		}
	}
	kprintf("wrong in bslookup_offset\n");
	restore(mask);
	return SYSERR;
}


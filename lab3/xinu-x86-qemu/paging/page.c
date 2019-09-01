/* page.c -  */

#include<xinu.h>

/* Note: I don't know why, but never set dirty anywhere */

// return the address of pd
pd_t *pd_allocate(){
	intmask mask;
	int i,j;
	pd_t *pdptr;
	pd_t *curr_pdaddr;
	pt_t *ptptr;
	frame_t *frameptr;

	mask = disable();
	//inverted_frame *iframeptr;

	// allocate page directory
	frameptr = frame_allocate();
	frameptr->ftype = PD_TYPE;
	frameptr->pid = currpid;
	//iframeptr = &inverted_frametab[frameptr->fid];
	//iframeptr->type = PD_TYPE;
	//iframeptr->pid = currpid;

	//proctab[currpid].pr_pdid = frameptr->fid;
	pdptr = (pd_t *)( fid_2_vaddr(frameptr->fid) );
	curr_pdaddr = pdptr; 
	//kprintf("Newly alloc PD: fid %d, pos: %uth frame\n",frameptr->fid,(uint32) pdptr/PAGECHUNKSIZE);

	// initialize pd entry
	for(i=0;i<PAGEDIRSIZE;i++){
		pdptr[i].pd_pres = 0;
		pdptr[i].pd_write = 0;
		pdptr[i].pd_user = 0;
		pdptr[i].pd_pwt = 0;
		pdptr[i].pd_pcd = 0;
		pdptr[i].pd_acc = 0;
		pdptr[i].pd_mbz = 0;
		pdptr[i].pd_fmb = 0;
		pdptr[i].pd_global = 0;
		pdptr[i].pd_avail = 0;
		pdptr[i].pd_base = 0;
	}

	// initialize global page directory and page tables
	for(i=0; i<4; i++){
		// pd entry set
		pdptr[i].pd_pres = 1;
		pdptr[i].pd_write = 1;
		pdptr[i].pd_user = 0;
		pdptr[i].pd_pwt = 0;
		pdptr[i].pd_pcd = 0;
		pdptr[i].pd_acc = 0;
		pdptr[i].pd_mbz = 0;
		pdptr[i].pd_fmb = 0;
		pdptr[i].pd_global = 0;
		pdptr[i].pd_avail = 0;

		frameptr = frame_allocate();// frame is enough?
		frameptr->ftype = PT_TYPE;
		frameptr->pid = currpid;
		//frameptr->refcount = PAGETABSIZE;
		ptptr = (pt_t *)( fid_2_vaddr(frameptr->fid) );
		pdptr[i].pd_base = ( (uint32) ptptr)/PAGECHUNKSIZE; // pd entry set
		//kprintf("check pd_base in page.c: %u\n",pdptr[i].pd_base);
		//kprintf("The base of %dth PD entry: %u\n",i,pdptr[i].pd_base);//
		//kprintf("Global PT: fid %d, vid: %u,pos: %u th frame\n",frameptr->fid,fid_2_vaddr(frameptr->fid)/PAGECHUNKSIZE,(uint32) ptptr/PAGECHUNKSIZE);
		
		// pages of this 4 PT
		for(j=0; j<PAGETABSIZE; j++){
			// pt entry set
			ptptr[j].pt_pres = 1;
			ptptr[j].pt_write = 1;
			ptptr[j].pt_user = 0;
			ptptr[j].pt_pwt = 0;
			ptptr[j].pt_pcd = 0;
			//ptptr[j].pt_acc = 0;
			//ptptr[j].pt_dirty = 0;
			ptptr[j].pt_mbz = 0;
			ptptr[j].pt_global = 0;
			ptptr[j].pt_avail = 0;
			ptptr[j].pt_base = i*PAGETABSIZE+j;// 0-4095, (* 4096)
		}
	}

	// page directory entry and page table for device;
	// i=4 now
	i = 576;// identity paging
	pdptr[i].pd_pres = 1;
	pdptr[i].pd_write = 1;
	frameptr = frame_allocate();
	frameptr->ftype = PT_TYPE;
	frameptr->pid = currpid;
	//frameptr->refcount = PAGETABSIZE;
	ptptr = (pt_t *)( fid_2_vaddr(frameptr->fid) );
	pdptr[i].pd_base = ( (uint32) ptptr)/PAGECHUNKSIZE;
	//kprintf("The base of %dth PD entry: %u\n",i,pdptr[i].pd_base);//
	//kprintf("Device PT: fid %d, vid: %u,pos: %u th frame\n",frameptr->fid,fid_2_vaddr(frameptr->fid)/PAGECHUNKSIZE,(uint32) ptptr/PAGECHUNKSIZE);

	// page of device PT
	for(j=0; j<PAGETABSIZE; j++){
		ptptr[j].pt_pres = 1;
		ptptr[j].pt_write = 1;
		ptptr[j].pt_user = 0;
		ptptr[j].pt_pwt = 0;
		ptptr[j].pt_pcd = 0;
		//ptptr[j].pt_acc = 0;
		//ptptr[j].pt_dirty = 0;
		ptptr[j].pt_mbz = 0;
		ptptr[j].pt_global = 0;
		ptptr[j].pt_avail = 0;
		ptptr[j].pt_base = 576*PAGETABSIZE+j;
	}

	restore(mask);
	return curr_pdaddr;
}


/* new page table can be created by page fault handler, no need for this function
// remember to add page table to page direct after every function call
// for 1 page table
// pages not assigned yet
pt_t *pt_allocate(){
	int j;
	frame_t *frameptr;
	pt_t *ptptr;
	frameptr = frame_allocate();
	frameptr->ftype = PT_TYPE;
	ptptr = (pt_t *) ( fid_2_vaddr(frameptr->fid) );

	for(j=0;j<PAGETABSIZE; j++){
		ptptr[j].pt_pres = 0;
		ptptr[j].pt_write = 1;
		// ptptr[j]->pt_base = //???//
	}
	return (pt_t *)SYSERR;
}
*/

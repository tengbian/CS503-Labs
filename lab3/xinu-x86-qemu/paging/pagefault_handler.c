/* pagefault_handler.c */
#include<xinu.h>

uint32 temp;

void pagefault_handler(){

	// disable ??
	intmask mask;
	mask = disable();

	uint32 a;
	a = read_cr2();  // a has been changed pos // a should be save before semaphore
	//kprintf("currpid: %d, a page fault: %u\n",currpid,read_cr2());
	num_faults += 1;
	
	// 4. check whether a is valid
	// if ( a < 0 && a > MAX_PAGENUM*NBPG){
	if ( a < MIN_VHEAP || a > (MIN_VHEAP+proctab[currpid].vheapsize) ){
		//kprintf("a is not a valid address: %u\n",a);
		//kprintf("5\n");
		kill(currpid);
		signal(pf_sem);
		//kprintf("success killed\n");
		restore(mask);
		return;
	}


	wait(pf_sem);
	
	int32 i;
	int32 store,offset;
	uint32 vp,p,q;
	pd_t *pd;// point to current pd
	pt_t *ptptr;//
	frame_t *pt_frameptr, *page_frameptr;

	// 1. get the faulted address a from CR2, 2,3
	vp = vaddr_2_vpage(a);
	// pd = (pd_t *) fid_2_vaddr( proctab[currpid].pr_pdid );
	pd = (pd_t *) read_cr3();

	

	p = a >> 22; // upper 10 bit, for PD entry
	q = (a << 10) >> 22; // middle 10 bit, for PT entry


	//kprintf("currpid: %d, address(a): %d, vpagenum: %u, pdvaddr: %u, pdentry_n(p): %d, ptentry_n(q): %d\n",currpid,a,vp,(uint32) pd,p,q);

	
	if (pd[p].pd_pres != 1){
		pt_frameptr = frame_allocate();
		pt_frameptr->ftype = PT_TYPE;
		pt_frameptr->pid = currpid;
		pt_frameptr->refcount = 0;
		ptptr = (pt_t *) ( fid_2_vaddr(pt_frameptr->fid) );
		//kprintf("check ptptr: %u\n",(uint32) ptptr);
		// which pd entry should I put?
		// pd_entryn = get_pd_entryn(pd);

		// initialize pd entry
		pd[p].pd_pres = 1;
		pd[p].pd_write = 1;
		pd[p].pd_user = 0;
		pd[p].pd_pwt = 0;
		pd[p].pd_pcd = 0;
		pd[p].pd_acc = 0;
		pd[p].pd_mbz = 0;
		pd[p].pd_fmb = 0;
		pd[p].pd_global = 0;
		pd[p].pd_avail = 0;
		pd[p].pd_base = ( (uint32) ptptr)/PAGECHUNKSIZE;
		//kprintf("pd_base: %u\n",pd[0].pd_base);//
		//kprintf("pd_base: %u\n",pd[p].pd_base);//


		// initialize pt frame
		for(i=0;i<PAGETABSIZE;i++){
			ptptr[i].pt_pres = 0;	
			ptptr[i].pt_write = 0;	
			ptptr[i].pt_pwt = 0;	
			ptptr[i].pt_pcd = 0;	
			//ptptr[i].pt_acc = 0;	
			//ptptr[i].pt_dirty = 0;	
			ptptr[i].pt_mbz = 0;	
			ptptr[i].pt_global = 0;	
			ptptr[i].pt_avail = 0;	
			ptptr[i].pt_base = 0;	
		}
		//kprintf("currpid: %d,create new pd entry successfully, initialize that page table done\n",currpid);
		hook_ptable_create(pt_frameptr->fid+FRAME0);// frame number
	}
	else{ // get ptptr
		ptptr = (pt_t *) (pd[p].pd_base*PAGECHUNKSIZE);	
		//kprintf("check here for pf\n");
		pt_frameptr = &frametab[vaddr_2_fid( (uint32) ptptr)];
		//kprintf("currpid: %d,Yes, it is pt_type = 2: %d; pid: %d,  pt_frameptr->refcount %d\n",currpid,pt_frameptr->ftype,pt_frameptr->pid,pt_frameptr->refcount);
		//pt_frameptr->refcount += 1;
	}

	//if(pd[p].pd_pres == 1 && ptptr[q].pt_pres == 1){
	if( ptptr[q].pt_pres == 1){
		kprintf("currpid: %d,Error here in pf, wrong\n",currpid);
	}

	// 8. bring in faulted page, it should be in back store
	if (ptptr[q].pt_pres != 1){
		//kprintf("currpid: %d, a: %u\n",currpid,a);
		store = bslookup_store(currpid,a);
		offset = bslookup_pageoffset(currpid,a);
		pt_frameptr->refcount += 1;	

		page_frameptr = obtain_free_frame();
		page_frameptr->vid = vp;// vpagenum of the page
		page_frameptr->pid = currpid;
		page_frameptr->advisor = (uint32) pt_frameptr;

		ptptr[q].pt_pres = 1;
		ptptr[q].pt_write = 1;
		ptptr[q].pt_pwt = 0;
		ptptr[q].pt_pcd = 0;
		//ptptr[q].pt_acc = 0;
		//ptptr[q].pt_dirty = 0;
		ptptr[q].pt_mbz = 0;
		ptptr[q].pt_global = 0; ptptr[q].pt_avail = 0;
		ptptr[q].pt_base = fid_2_vaddr(page_frameptr->fid)/PAGECHUNKSIZE;
		//kprintf("currpid: %d,create new pt successfully\n",currpid);


		// what if I flush all page?

		//kprintf("currpid: %d,get the store and offset in bsmap: %d, %d\n",currpid,store,offset);
		//kprintf("there 1\n");

		//kprintf("currpid: %d, fid: %d, vaddr: %u, store: %d, offset: %d\n",currpid,page_frameptr->fid, fid_2_vaddr(page_frameptr->fid),store,offset);
		if ( (open_bs(store)) == SYSERR){
			//kprintf("6\n");
			kill(currpid);
			restore(mask);
			return;
		}
		if (read_bs((char *)fid_2_vaddr(page_frameptr->fid),store,offset) == SYSERR){
			//kprintf("7\n");
			kill(currpid);
			restore(mask);
			return;
		}
		
		// to the addr of page_frame /// Problem Here
		//kprintf("currpid: %d,bs_read_success\n",currpid);
		if (close_bs(store) == SYSERR){
			//kprintf("bs_close_not_success\n");
			//kprintf("8\n");
			kill(currpid);
			restore(mask);
			return;
		}
		//kprintf("currpid: %d, there 2\n",currpid);


		hook_pfault(currpid,(void *) a,vp,page_frameptr->fid+FRAME0);
		signal(pf_sem);
		restore(mask);
		return;
	}
	else {// error
		//kprintf("currpid:%d, Error in page fault handler, pd_base %u, pt_base %u\n",currpid, pd[p].pd_base,ptptr[q].pt_base);
		signal(pf_sem);
		restore(mask);
		return;
	}
}





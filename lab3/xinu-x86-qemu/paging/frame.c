#include <xinu.h>

uint32 tmp;

void initialize_frame(){
	intmask mask;
	mask = disable();

	int i;
	for(i=0;i<NFRAMES;i++){
		frametab[i].fid = i;// never changes
		frametab[i].pid = -1;
		frametab[i].vid = -1;
		frametab[i].ftype = NO_TYPE;
		frametab[i].fstate = FRAME_FREE;
		frametab[i].refcount = 0;
		frametab[i].advisor = 0;
	}
	//kprintf("initialization %d frames succeed\n",NFRAMES);

	restore(mask);
	return;
}


// this is for pd and pt
// other attributes are assigned later
frame_t *frame_allocate(){
	intmask mask;
	mask = disable();

	int i;
	// fifo
	for(i=0;i<NFRAMES;i++){
		if(frametab[i].fstate == FRAME_FREE){
			//kprintf("currpid %d, allocate No. %d frame\n",currpid,i);
			frametab[i].fstate = FRAME_USED;
			//inverted_frametab[i].state = FRAME_USED;
			restore(mask);
			return &frametab[i];
		}
	}
	// if not found
	//kprintf("currpid %d, frame not found for pd/pt\n",currpid);
	restore(mask);
	return NULL;
}


// this is for page
// locate other things for this frame
// this is for fifo, I'd better get another without fifo
frame_t *obtain_free_frame(){
	intmask mask;
	mask = disable();

	frame_t *pg_frameptr;// this pointer is for page
	frame_t *pt_frameptr;// this pointer is for pt
	pt_t *ptptr;
	int32 i;
	for(i=0; i<NFRAMES; i++){
		if(frametab[i].fstate == FRAME_FREE){
			//kprintf("currpid: %d, obtain page frame: %d th frame\n",currpid,i);		
			frametab[i].fstate = FRAME_USED;
			frametab[i].ftype = PG_TYPE;
			insert_pageframe_intolist(i);
			restore(mask);
			return &frametab[i];
		}
	}
	// ------  else, evit current page ---------
	uint32 vp,a,p,q;
	pd_t *pd;
	pt_t *pt;
	int32 store;
	int32 offset;
	int32 pid;

	// 2. pick a page to replace
	pg_frameptr = get_pageframe_by_policy();	
	//kprintf("obtain page frame, by eviting: %d th frame\n",pg_frameptr->fid);
	vp = pg_frameptr->vid;
	pid = pg_frameptr->pid;

	//kprintf("evited frame information: vp: %d, ");//
	
	hook_pswap_out(pg_frameptr->pid,vp,pg_frameptr->fid+FRAME0);

	a = vpage_2_vaddr(vp); // this is first virtual address on page ; not the address of page frame
	p = a >> 22;
	q = (a<<10)>>22;

	pd = (pd_t *) fid_2_vaddr(proctab[pid].pr_pdid);
	pt = (pt_t *) ( ((uint32)(pd[p].pd_base))* PAGECHUNKSIZE);

	if( pid == currpid){
		tmp = a; // address of page
		asm("pushl %eax");
		asm("invlpg tmp");
		asm("popl %eax");
		//kprintf("yes pid == currpid\n");
	}
	//kprintf("Here 6\n");
	

	// write back
	if( pt[q].pt_dirty == 1){
		//kprintf("Here 8\n");
		store = bslookup_store(pid,a);
		offset = bslookup_pageoffset(pid,a);
		//kprintf("Here 9, store = %d, offset = %d\n",store, offset);
		if( ((store = bslookup_store(pid,a)) == SYSERR) || ((offset = bslookup_pageoffset(pid,a)) == SYSERR) ){
			//kprintf("Something is wrong with process %d\n",pid);
			//kprintf("1\n");
			kill(currpid);
			restore(mask);
			return (void *)SYSERR;

		}
		//kprintf("currpid: %d, Here_10\n",currpid);
		if (open_bs(store) == SYSERR){
			//kprintf("2\n");
			kill(currpid);
			restore(mask);
			return (void *)SYSERR;
		}
		//kprintf("currpid: %d,Here_11, a=%u, pid = %d, currpid = %d\n",currpid,a,pid,currpid);
		//kprintf("a %u, fid_2_vaddr: %u\n",(uint32) a, *((uint32 *)fid_2_vaddr(pg_frameptr->fid)));
		if ( (write_bs((char *) fid_2_vaddr(pg_frameptr->fid),store,offset)) == SYSERR){
			//kprintf("3\n");
			kill(currpid);
			restore(mask);
			return (void *)SYSERR;
		}
		
		// consider the possibility that pid != currpid; write from page frame's address
		//write_bs((char *) a,store,offset); // for 1 proc, this is fine
		//kprintf("currpid: %d, Here_12\n",currpid);
		if(close_bs(store) == SYSERR){
			//kprintf("something wrong with closing");
			//kprintf("4\n");
			kill(currpid);
			restore(mask);
			return (void *)SYSERR;
		}
		//kprintf("write back to back store\n");
		//pt[q].pt_dirty = 0; this can never be added !!! Why, don't know!
	}

	// after it writes back, we can initial everything
	pt[q].pt_pres = 0;

	pt_frameptr = &frametab[vaddr_2_fid( (uint32) pt)];
	pt_frameptr->refcount --;
	if(pt_frameptr->refcount == 0){
		//kprintf("delete frame, for refcount of this pt = 0\n");
		pd[p].pd_pres = 0;
		// delete pt table
		ptptr = (pt_t *) fid_2_vaddr(pt_frameptr->fid);
		for(i=0;i<PAGETABSIZE;i++){
			ptptr[i].pt_pres = 0;
			ptptr[i].pt_write = 0;
			ptptr[i].pt_user = 0;
			ptptr[i].pt_pwt = 0;
			ptptr[i].pt_pcd = 0;
			//ptptr[i].pt_acc = 0;
			//ptptr[i].pt_dirty = 0;
			ptptr[i].pt_mbz = 0;
			ptptr[i].pt_global = 0;
			ptptr[i].pt_avail = 0;
			ptptr[i].pt_base = 0;
		}
		// delete frame
		frametab[pt_frameptr->fid].pid = -1;
		frametab[pt_frameptr->fid].vid = -1;
		frametab[pt_frameptr->fid].ftype = NO_TYPE;
		frametab[pt_frameptr->fid].fstate = FRAME_FREE;
		frametab[pt_frameptr->fid].refcount = 0;
		frametab[pt_frameptr->fid].advisor = 0;
		hook_ptable_delete(pt_frameptr->fid+FRAME0);
	}

	//kprintf("Here 8\n");

	// initialize  pg_frame
	pg_frameptr->fstate = FRAME_USED;
	pg_frameptr->ftype = PG_TYPE;
	pg_frameptr->pid = currpid;
	//kprintf("should insert %d frame into fifolist\n",pg_frameptr->fid);
	insert_pageframe_intolist(pg_frameptr->fid);
	restore(mask);
	return pg_frameptr;
}

frame_t *get_pageframe_by_policy(){
	intmask mask;
	mask = disable();
	if (currpolicy == FIFO){
		restore(mask);
		return get_pageframe_by_fifo();
	}
	else if (currpolicy == GCA){
		restore(mask);
		return get_pageframe_by_gca();
	}
	else{
		//kprintf("wrong in policy, currpolicy = %d\n",currpolicy);	
		restore(mask);
		return (void *)SYSERR;
	}
}

void initialize_fifolist(){
	intmask mask;
	mask = disable();

	int i;
	fifohead = -1;	
	fifotail = -1;
	for(i=0; i< NFRAMES; i++){
		fifolist[i].next = -1;
	}
	//kprintf("iniitalize fifolist: succeed\n");
	restore(mask);
	return;
}

void initialize_gca_dirtylist(){
	intmask mask;
	mask = disable();
	
	int i;
	for(i=0; i<NFRAMES; i++){
		gca_dirtylist[i] = 0;
	}
	//kprintf("initialize gca_dirtylist: succeed\n");
	restore(mask);
	return;
}

void insert_pageframe_intolist(int32 framenum){
	intmask mask;
	mask = disable();
	//kprintf("insert framenum: %d\n",framenum);
	if (currpolicy == FIFO){
		// intert into fifolist
		if(fifohead == -1){
			fifohead = framenum;
			fifolist[framenum].next = -1;
			fifotail = framenum;
		}
		else{
			fifolist[fifotail].next = framenum;
			fifolist[framenum].next = -1;
			fifotail = framenum;
		}
	}
	restore(mask);
	return;
}

// when using this, page frame space is already full
frame_t *get_pageframe_by_fifo(){
	intmask mask;
	int32 prev_fifohead;
	mask = disable();
	frame_t *page_frameptr;	
	page_frameptr = &frametab[fifohead];
	if(fifohead == -1){
		//kprintf("error in inserting fifo frame\n");
	}
	prev_fifohead = fifohead;
	fifohead = fifolist[fifohead].next;
	fifolist[prev_fifohead].next = -1;

	//kprintf("fifohead: %d, fifotail: %d\n",fifohead,fifotail);
	// if fifohead = -1, still no need to worry fifotail
	restore(mask);
	return page_frameptr;	
}


frame_t *get_pageframe_by_gca(){
	intmask mask;
	mask = disable();

	int32 i;
	int32 gca_plus;
	int32 gca_curr;
	uint32 pgaddr, ptnum;// num of entry
	frame_t *pg_frameptr;
	frame_t *pt_frameptr;
	pt_t *ptptr;


	// gca_pin is already the next stop
	for(i=0; i< 3; i++){
		for(gca_plus=0; gca_plus < NFRAMES; gca_plus++){
			gca_curr = (gcapin+gca_plus)%NFRAMES;
			if(frametab[gca_curr].ftype == PG_TYPE){
				pg_frameptr = &frametab[gca_curr];
				pgaddr = vpage_2_vaddr(pg_frameptr->vid);// address of the page 0th entry
				pt_frameptr = (frame_t *) pg_frameptr->advisor;
				ptptr = (pt_t *) fid_2_vaddr(pt_frameptr->fid);
				ptnum = (pgaddr << 10) >> 22;
				if( (ptptr[ptnum].pt_acc==0) && (ptptr[ptnum].pt_dirty==0)){
					gca_dirtylist_reset();
					gcapin = gca_curr+1;
					//kprintf("ptnum: %d\n",ptnum);
					restore(mask);
					return pg_frameptr;	
				}
				else if( (ptptr[ptnum].pt_acc==1) && (ptptr[ptnum].pt_dirty==0)){
					ptptr[ptnum].pt_acc = 0;
				}
				else if( (ptptr[ptnum].pt_acc==1) && (ptptr[ptnum].pt_dirty==1)){
					ptptr[ptnum].pt_dirty = 0;
					gca_dirtylist[gca_curr] = 1;
				}
				else{
					//kprintf("get_page_frame_gca error, pt_acc=0, pt_dirty=1\n");
					ptptr[ptnum].pt_dirty = 0;
					gca_dirtylist[gca_curr] = 1;
					//restore(mask);
					//return (void *) SYSERR;
				}
			}
		}
	}

	//kprintf("Error: gca can not find pageframe in 3 iterations\n");
	restore(mask);
	return (void *)SYSERR;
}

// this would be only for pages (intrinsic)
void gca_dirtylist_reset(){
	intmask mask;
	mask = disable();

	int32 i;
	int32 ptnum;
	frame_t *pg_frameptr, *pt_frameptr;
	pt_t *ptptr;
	uint32 pgaddr;
	for(i=0;i<NFRAMES;i++){
		if(gca_dirtylist[i] == 1){
			pg_frameptr = &frametab[i];
			pt_frameptr = (frame_t *) pg_frameptr->advisor;
			ptptr = (pt_t *) fid_2_vaddr(pt_frameptr->fid);
			pgaddr = vpage_2_vaddr(pg_frameptr->vid);
			ptnum = (pgaddr << 10) >> 22;
			ptptr[ptnum].pt_dirty = 1;
			// reset dirtylist
			gca_dirtylist[i] = 0;
		}
	}
	restore(mask);
	return;
}






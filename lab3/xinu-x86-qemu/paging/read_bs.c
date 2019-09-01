/* read_bs.c - read_bs */

#include <xinu.h>

/*----------------------------------------------------------------------------
 *  read_bs  -  This copies the pagenum'th page from the backing store
 *  referenced by ID store to memory pointed by dst. It returns OK on success,
 *  SYSERR otherwise. The first page of a backing store is page zero.
 *----------------------------------------------------------------------------
 */
syscall read_bs (char *dst, bsd_t bs_id, uint32 page)
{
	//intmask mask;
	//mask = disable();

	int rd_blk = 0;
	char buf[RD_BLKSIZ] = {0};
	int i= 0;

	if(PAGE_SERVER_STATUS == PAGE_SERVER_INACTIVE){
		kprintf("Page server is not active\r\n");
		//restore(mask);
		return SYSERR;
	}

	if (bs_id > MAX_ID || bs_id < MIN_ID) {
		kprintf("read_bs failed for bs_id %d and page number %d\r\n",
						bs_id,
						page);
		//restore(mask);
		return SYSERR;
	}

	//kprintf("bs here 1\n");

	wait(bs_sem);

	//kprintf("bs here 2\n");

	if (bstab[bs_id].isopen == FALSE
			|| bstab[bs_id].npages <= page){
		kprintf("read_bs failed for bs_id %d and page number %d\r\n",
						bs_id,
						page);
		signal(bs_sem);
		//restore(mask);
		return SYSERR;
	}
	signal(bs_sem);
	//kprintf("bs here 3\n");
	/*
	 * The first page for a backing store is page 0
	 * FIXME : Check id read on RDISK takes blocks from 0 ...
	 */
	rd_blk = (bs_id * RD_PAGES_PER_BS + page)*8;

	for(i=0; i< 8; i++){
		//kprintf("bs here 4:   i \n",i);
		memset(buf, NULLCH, RD_BLKSIZ);
		//kprintf("bs here 44\n");
		if(read(WRDISK, buf, rd_blk+i) == SYSERR){
			//kprintf("bs here 4.0\n");
			panic("Could not read from backing store \r\n");
		}
		else{
			//kprintf("bs here 4.1\n");
			memcpy((char *)(dst+i*RD_BLKSIZ), (char *)buf, RD_BLKSIZ);
			//kprintf("bs here 4.2\n");
		}
	}
	//kprintf("bs here 5\n");

	//restore(mask);
	return OK;
}

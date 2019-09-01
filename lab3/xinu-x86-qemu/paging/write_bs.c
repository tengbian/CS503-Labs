/* write_bs.c - write_bs */

#include <xinu.h>

/*----------------------------------------------------------------------------
 *  write_bs  -  This copies a page pointed to by src to the pagenum'th page of
 *  the backing store referenced by store. It returns OK on success, SYSERR
 *  otherwise.
 *----------------------------------------------------------------------------
 */
syscall write_bs (char *src, bsd_t bs_id, uint32 page)
{
	uint32 rd_blk = 0;
	char buf[RD_BLKSIZ] = {0};
	int i=0;

	if(PAGE_SERVER_STATUS == PAGE_SERVER_INACTIVE){
		kprintf("Page server is not active\r\n");
		return SYSERR;
	}

	if (bs_id > MAX_ID || bs_id < MIN_ID) {
		kprintf("write_bs failed for bs_id %d and page number %d\r\n",
						bs_id,
						page);
		return SYSERR;
	}

	wait(bs_sem);

	if (bstab[bs_id].isopen == FALSE
			|| page >= bstab[bs_id].npages){
		kprintf("write_bs failed for bs_id %d and page number %d\r\n",
						bs_id,
						page);
		signal(bs_sem);
		return SYSERR;
	}
	signal(bs_sem);

	/*
	 * The first page for a backing store is page 0
	 * FIXME : Check id read on RDISK takes blocks from 0 ...
	 */
	rd_blk = (bs_id * RD_PAGES_PER_BS + page)*8;

	for(i=0; i< 8; i++){
		//kprintf("write_bs iteration [%d]\r\n", i);
		memcpy((char *)buf, (char *)(src+i*RD_BLKSIZ),  RD_BLKSIZ);
		if(write(WRDISK, buf, rd_blk+i) == SYSERR){
			panic("Could not write to backing store \r\n");
		}
	}

	return OK;
}

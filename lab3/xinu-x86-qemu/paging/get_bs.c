/* get_bs.c - get_bs */

#include <xinu.h>

/*-----------------------------------------------------------------------------
 *  get_bs  -  This call requests from the page server a new backing store with
 *  ID store of size npages (unit in pages, not bytes). If the page server is
 *  able to create the new backing store, then the size of the new backing
 *  store (in unit of pages) is returned.
 *
 *  Note: You don't need to invoke this directly.
 *-----------------------------------------------------------------------------
 */
syscall get_bs(bsd_t bs_id, uint32 npages)
{
        if(PAGE_SERVER_STATUS == PAGE_SERVER_INACTIVE){
                psinit();
        }

        char buf[RD_BLKSIZ] = {0};
        memset(buf, 0, RD_BLKSIZ);

        /*
                 int pagedev, ret, try = 0;
                 struct ps_header req, resp;
                 */
        if (bs_id > MAX_ID || bs_id < MIN_ID || npages == 0 || npages > MAX_PAGES_PER_BS)
                return SYSERR;

        wait(bs_sem);
        if(bstab[bs_id].isopen == TRUE){
                signal(bs_sem);
                return SYSERR;
        }
        bstab[bs_id].isopen = TRUE;
        bstab[bs_id].npages = npages;
        signal(bs_sem);

        return npages;
}

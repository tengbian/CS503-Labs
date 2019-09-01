#include <xinu.h>

int count_bs(void)
{
        /*if(PAGE_SERVER_STATUS == PAGE_SERVER_INACTIVE){
                psinit();
        }*/
	int freebs = 0;
	int bs_id;

	for(bs_id = 0; bs_id < MAX_BS_ENTRIES; bs_id++)
	{
		if(bstab[bs_id].isallocated == FALSE)  /* Found an allocated store */
			freebs++;
	}
        return freebs;
}

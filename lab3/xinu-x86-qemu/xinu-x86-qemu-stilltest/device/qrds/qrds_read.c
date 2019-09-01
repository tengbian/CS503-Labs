#include <xinu.h>

devcall	qrds_read (
	  struct dentry	*devptr,	/* Entry in device switch table	*/
	  char	*p,			/* Buffer to hold disk block	*/
	  int32	blk			/* Block number of block to read*/
	)
{
  ASSERT(blk < QRDISK_MAX_BUFF);

  struct qrds_buff *buff = &qrds[0].buff[blk];

  memcpy(p, buff->data, RD_BLKSIZ);

  return OK;
}

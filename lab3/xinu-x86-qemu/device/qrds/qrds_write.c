#include <xinu.h>

devcall	qrds_write (
  struct dentry	*devptr,	/* Entry in device switch table	*/
  char	*p,			/* Buffer that holds a disk blk	*/
  int32	blk			/* Block number to write	*/
  )
{
  ASSERT(blk < QRDISK_MAX_BUFF);

  struct qrds_buff *buff = &qrds[0].buff[blk];

  memcpy(buff->data, p, RD_BLKSIZ);
  buff->status = 1;

  return OK;
}

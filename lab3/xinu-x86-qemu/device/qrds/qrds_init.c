#include <xinu.h>

struct	qrds_blk qrds[Nrds];

devcall	qrds_init (
	  struct dentry	*devptr		/* Entry in device switch table	*/
	)
{
  for (int i=0; i<Nrds; i++) {
    struct qrds_blk *q = &qrds[i];
    for (int j=0; j<QRDISK_MAX_BUFF; j++) {
      q->buff[i].status = 0;
    }
  }

  return OK;
}

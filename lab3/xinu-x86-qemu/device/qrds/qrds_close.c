#include <xinu.h>

devcall	qrds_close (
	  struct dentry	*devptr		/* Entry in device switch table	*/
	)
{
  return OK;
}

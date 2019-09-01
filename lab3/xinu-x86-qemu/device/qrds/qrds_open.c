#include <xinu.h>

devcall	qrds_open (
	 struct	dentry	*devptr,	/* Entry in device switch table	*/
	 char	*diskid,		/* Disk ID to use		*/
	 char	*mode			/* Unused for a remote disk	*/
	)
{
  return devptr->dvnum;
}

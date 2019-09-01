/* rdscontrol.c - rdscontrol */

#include <xinu.h>

devcall	qrds_control (
  struct dentry	*devptr,	/* Entry in device switch table	*/
  int32	func,			/* The control function to use	*/
  int32	arg1,			/* Argument #1			*/
  int32	arg2			/* Argument #2			*/
  )
{
  return OK;
}

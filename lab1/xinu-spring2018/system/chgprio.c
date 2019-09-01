/* chgprio.c - chgprio */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  chgprio  -  Change the scheduling priority of a group 
 *------------------------------------------------------------------------
 */
pri16	chgprio(
	  int 		group,		/* ID of process to change	*/
	  pri16		newprio		/* New priority			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	gprocent *gptr;		/* Ptr to process's table entry	*/
	pri16	oldprio;		/* Priority to return		*/

	mask = disable();
	if (isbadgid(group)) {
		restore(mask);
		return (pri16) SYSERR;
	}
	gptr = &grouptab[group];
	oldprio = gptr->gprio_init;
	gptr->gprio_init = newprio;
	restore(mask);
	return oldprio;
}

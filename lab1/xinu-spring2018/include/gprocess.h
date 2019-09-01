/* gprocess.h - isbadgpid */

#define NGROUP 2

/* Definition of the group table */

struct gprocent{		/* Entry in the group table */
	int32 pnum;			/* Number of processes */
	pri16 gprio_init;	/* Initial group priority */
	pri16 gprio_curr;	/* Current group priority */
	pid32 g_firstid;	/* Record the first process's id in group */
	pid32 g_firstPRIOi;	/* Record the first process's PRIOi in group */
};	

extern struct gprocent grouptab[];

#define isbadgid(x) (((int)(x) < 0) || ((int)(x) >= 2))

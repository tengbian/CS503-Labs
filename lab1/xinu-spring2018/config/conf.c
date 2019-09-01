/* conf.c (GENERATED FILE; DO NOT EDIT) */

#include <xinu.h>


extern	devcall	ioerr(void);
extern	devcall	ionull(void);

/* Device independent I/O switch */

struct	dentry	devtab[NDEVS] =
{
/**
 * Format of entries is:
 * dev-number, minor-number, dev-name,
 * init, open, close,
 * read, write, seek,
 * getc, putc, control,
 * dev-csr-address, intr-handler, irq
 */

/* CONSOLE is tty */
	{ 0, 0, "CONSOLE",
	  (void *)ttyinit, (void *)ionull, (void *)ionull,
	  (void *)ttyread, (void *)ttywrite, (void *)ioerr,
	  (void *)ttygetc, (void *)ttyputc, (void *)ttycontrol,
	  (void *)0x3f8, (void *)ttydispatch, 42 },

/* QEMUCONSOLE is qty */
	{ 1, 0, "QEMUCONSOLE",
	  (void *)qty_init, (void *)ionull, (void *)ionull,
	  (void *)qty_read, (void *)qty_write, (void *)ioerr,
	  (void *)qty_getc, (void *)qty_putc, (void *)qty_control,
	  (void *)0x0, (void *)qty_dispatch, 36 },

/* NULLDEV is null */
	{ 2, 0, "NULLDEV",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)ionull, (void *)ionull, (void *)ioerr,
	  (void *)ionull, (void *)ionull, (void *)ioerr,
	  (void *)0x0, (void *)ioerr, 0 },

/* ETHER0 is eth */
	{ 3, 0, "ETHER0",
	  (void *)ethinit, (void *)ioerr, (void *)ioerr,
	  (void *)ethread, (void *)ethwrite, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ethcontrol,
	  (void *)0x0, (void *)ethdispatch, 43 },

/* NAMESPACE is nam */
	{ 4, 0, "NAMESPACE",
	  (void *)naminit, (void *)namopen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ioerr, 0 },

/* RDISK is rds */
	{ 5, 0, "RDISK",
	  (void *)rdsinit, (void *)rdsopen, (void *)rdsclose,
	  (void *)rdsread, (void *)rdswrite, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)rdscontrol,
	  (void *)0x0, (void *)ionull, 0 },

/* RAM0 is ram */
	{ 6, 0, "RAM0",
	  (void *)raminit, (void *)ramopen, (void *)ramclose,
	  (void *)ramread, (void *)ramwrite, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILESYS is rfs */
	{ 7, 0, "RFILESYS",
	  (void *)rfsinit, (void *)rfsopen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)rfscontrol,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE0 is rfl */
	{ 8, 0, "RFILE0",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE1 is rfl */
	{ 9, 1, "RFILE1",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE2 is rfl */
	{ 10, 2, "RFILE2",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE3 is rfl */
	{ 11, 3, "RFILE3",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE4 is rfl */
	{ 12, 4, "RFILE4",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE5 is rfl */
	{ 13, 5, "RFILE5",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE6 is rfl */
	{ 14, 6, "RFILE6",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE7 is rfl */
	{ 15, 7, "RFILE7",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE8 is rfl */
	{ 16, 8, "RFILE8",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* RFILE9 is rfl */
	{ 17, 9, "RFILE9",
	  (void *)rflinit, (void *)ioerr, (void *)rflclose,
	  (void *)rflread, (void *)rflwrite, (void *)rflseek,
	  (void *)rflgetc, (void *)rflputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILESYS is lfs */
	{ 18, 0, "LFILESYS",
	  (void *)lfsinit, (void *)lfsopen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILE0 is lfl */
	{ 19, 0, "LFILE0",
	  (void *)lflinit, (void *)ioerr, (void *)lflclose,
	  (void *)lflread, (void *)lflwrite, (void *)lflseek,
	  (void *)lflgetc, (void *)lflputc, (void *)lflcontrol,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILE1 is lfl */
	{ 20, 1, "LFILE1",
	  (void *)lflinit, (void *)ioerr, (void *)lflclose,
	  (void *)lflread, (void *)lflwrite, (void *)lflseek,
	  (void *)lflgetc, (void *)lflputc, (void *)lflcontrol,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILE2 is lfl */
	{ 21, 2, "LFILE2",
	  (void *)lflinit, (void *)ioerr, (void *)lflclose,
	  (void *)lflread, (void *)lflwrite, (void *)lflseek,
	  (void *)lflgetc, (void *)lflputc, (void *)lflcontrol,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILE3 is lfl */
	{ 22, 3, "LFILE3",
	  (void *)lflinit, (void *)ioerr, (void *)lflclose,
	  (void *)lflread, (void *)lflwrite, (void *)lflseek,
	  (void *)lflgetc, (void *)lflputc, (void *)lflcontrol,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILE4 is lfl */
	{ 23, 4, "LFILE4",
	  (void *)lflinit, (void *)ioerr, (void *)lflclose,
	  (void *)lflread, (void *)lflwrite, (void *)lflseek,
	  (void *)lflgetc, (void *)lflputc, (void *)lflcontrol,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILE5 is lfl */
	{ 24, 5, "LFILE5",
	  (void *)lflinit, (void *)ioerr, (void *)lflclose,
	  (void *)lflread, (void *)lflwrite, (void *)lflseek,
	  (void *)lflgetc, (void *)lflputc, (void *)lflcontrol,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE0 is pip */
	{ 25, 0, "PIPELINE0",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE1 is pip */
	{ 26, 1, "PIPELINE1",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE2 is pip */
	{ 27, 2, "PIPELINE2",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE3 is pip */
	{ 28, 3, "PIPELINE3",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE4 is pip */
	{ 29, 4, "PIPELINE4",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE5 is pip */
	{ 30, 5, "PIPELINE5",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE6 is pip */
	{ 31, 6, "PIPELINE6",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE7 is pip */
	{ 32, 7, "PIPELINE7",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE8 is pip */
	{ 33, 8, "PIPELINE8",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* PIPELINE9 is pip */
	{ 34, 9, "PIPELINE9",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)pipread, (void *)pipwrite, (void *)ioerr,
	  (void *)pipgetc, (void *)pipputc, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 }
};

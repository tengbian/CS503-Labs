/* paging.h */

#ifndef __PAGING_H_
#define __PAGING_H_

/* Structure for a page directory entry */

typedef struct {
	unsigned int pd_pres	: 1;		/* page table present?		*/
	unsigned int pd_write : 1;		/* page is writable?		*/
	unsigned int pd_user	: 1;		/* is use level protection?	*/
	unsigned int pd_pwt	: 1;		/* write through cachine for pt? */
	unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
	unsigned int pd_acc	: 1;		/* page table was accessed?	*/
	unsigned int pd_mbz	: 1;		/* must be zero			*/
	unsigned int pd_fmb	: 1;		/* four MB pages?		*/
	unsigned int pd_global: 1;		/* global (ignored)		*/
	unsigned int pd_avail : 3;		/* for programmer's use		*/
	unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {
	unsigned int pt_pres	: 1;		/* page is present?		*/
	unsigned int pt_write : 1;		/* page is writable?		*/
	unsigned int pt_user	: 1;		/* is use level protection?	*/
	unsigned int pt_pwt	: 1;		/* write through for this page? */
	unsigned int pt_pcd	: 1;		/* cache disable for this page? */
	unsigned int pt_acc	: 1;		/* page was accessed?		*/
	unsigned int pt_dirty : 1;		/* page was written?		*/
	unsigned int pt_mbz	: 1;		/* must be zero			*/
	unsigned int pt_global: 1;		/* should be zero in 586	*/
	unsigned int pt_avail : 3;		/* for programmer's use		*/
	unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

#define PAGEDIRSIZE	1024
#define PAGETABSIZE	1024
#define PAGECHUNKSIZE 4096

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/

#ifndef NFRAMES
#define NFRAMES		3072	/* number of frames		*/
#endif

#define MAP_SHARED 1
#define MAP_PRIVATE 2

#define FIFO 3
#define GCA 4

#define MAX_ID		7		/* You get 8 mappings, 0 - 7 */
#define MIN_ID		0

extern int32	currpolicy;

// defined in initialize.c
extern int32 fifohead;
extern int32 fifotail;
extern uint32 gcapin;
extern int32 num_faults;
extern sid32 pf_sem;
extern int32 gca_dirtylist[];

// frame_t, by Teng , this is for pd and pt
#define NO_TYPE	 0 
#define PD_TYPE  1
#define PT_TYPE  2
#define PG_TYPE	 3 
#define FRAME_FREE 0
#define FRAME_USED 1
typedef struct {
	int32 fid;		// for pd, pt, pg: this is useful when using pointer
	int32 pid;		// for pd, pt, pg: process number
	int32 vid;		// for pg: virtual page number (not for pt)
	int32 ftype; 	// for pd, pt, pg
	int32 fstate;	// for pd, pt, pg
	int32 refcount; // for pt
	uint32 advisor; // for pd, pt, pg: address, should be transformed to frame_t every time using
}frame_t;
extern frame_t frametab[];

// for page list
typedef struct {
	int32 next;
} fifo_entry;
extern fifo_entry fifolist[];


// bsmap
#define BS_FREE 0
#define BS_USED 1
typedef struct {
	int32 pid;
	int32 vpage;
	int32 npages;
	int32 store; // 0-7
	int32 state;
}bsmap;
extern bsmap bsmaptab[];


/*
// inverted_frame, this is only for pages
// don't care about identity paging
typedef struct {
	int32 pid;// process id, only when type = PT_TYPE
	int32 vid;// virtual page number, only when type = PT_TYPE
	int32 state;
	int32 type; // works only when type = PT_TYPE
}inverted_frame;
extern inverted_frame inverted_frametab[];
*/



// for memory
//extern struct memblk vmemlist;// for virtual heap

// this only care about frameid and frame's vpageid, vaddr (identity paging)
#define fid_2_vaddr(x) ( (uint32) ((FRAME0+x)*NBPG) )
//#define fid_2_vid(x)	   ( (int32)  (FRAME0+x) )
#define vaddr_2_fid(x) ( (int32)  (x/NBPG-FRAME0) )

#define vaddr_2_vpage(x)	((uint32) (x/NBPG))
#define vpage_2_vaddr(x)	((uint32) (x*NBPG))

#define MIN_VHEAP ( (uint32) (0x01000000) )
#define MAX_VHEAP ( (uint32) (0x8FFFFFFF) )
#define MAX_PAGENUM (0x903FFFFF/NBPG)
#define MIN_HPN (0x01000000/NBPG)
#define MAX_HPN (0x8FFFFFFF/NBPG)



#endif // __PAGING_H_



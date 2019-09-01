#include <xinu.h>


unsigned long tmp;

// from website
typedef short	STATWORD[1]; 

void set_cr4(unsigned long n) {
  /* we cannot move anything directly into
     %cr4. This must be done via %eax. Therefore
     we save %eax by pushing it then pop
     it at the end.
  */

  //STATWORD ps;
  //disable(ps);
  intmask ps=disable()

  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");		/* mov (move) value at tmp into %eax register.
					   "l" signifies long (see docs on gas assembler)	*/
  asm("movl %eax, %cr4");
  asm("popl %eax");

  restore(ps);
}

unsigned long read_cr3(void) {

  //STATWORD ps;
  unsigned long local_tmp;

  //disable(ps);

  intmask ps=disable()

  asm("pushl %eax");
  asm("movl %cr3, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(ps);

  return local_tmp;
}

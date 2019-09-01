#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>

extern bool8 xinu_debug_flag;

// always toggle
shellcmd xsh_debug(int argc, char *argv[])
{
    if (xinu_debug_flag == TRUE) {
        printf("(xsh_debug) Debug flag is turned OFF\n");
        xinu_debug_flag = FALSE;
    } else {
        printf("(xsh_debug) Debug flag is turned ON\n");
        xinu_debug_flag = TRUE;
    }
	return OK;
}

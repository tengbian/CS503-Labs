/* debug.c  -  hexdump_print, hexdump */

#include <xinu.h>
#include <stdio.h>
#include <debug.h>

bool8 xinu_debug_flag = FALSE;

static void hexdump_print(byte, byte);

/*------------------------------------------------------------------------
 *  hexdump_print   -  Print a byte in ASCII or hex
 *------------------------------------------------------------------------
 */
static void hexdump_print(
	 byte	data,			/* Item to print		*/
	 byte	mode			/* ASCII or hex mode		*/
	)
{
    switch (mode)
    {
    case DEBUG_ASCII:
        data = (' ' <= data && data <= '~') ? data : '.';
        kprintf("%c", data);
        break;
    case DEBUG_HEX:
        kprintf("%02x ", data);
        break;
    default:
        break;
    }
}


/*------------------------------------------------------------------------
 *  hexdump   -  Dump a region of memory
 *------------------------------------------------------------------------
 */
void	hexdump(
	 void	*buffer,		/* Addresss of memory area	*/
	 uint32	length,			/* Length in bytes		*/
	 bool8	canon			/* Print in ASCII or hex	*/
	)
{
    uint32 m, n, remain;

    byte *b = (byte *)buffer;

    for (n = 0; n < length; n += 0x10) {
        kprintf("%08x ", (uint32)buffer + n);

        remain = length - n;

        for (m = 0; m < remain && m < 0x10; m++) {
            if (m % 0x08 == 0) {
                kprintf(" ");
            }
            hexdump_print(b[n + m], DEBUG_HEX);
        }

        /* Pad the rest if needed */
        if (remain < 0x10) {
            for (m = 0; m < 0x10 - remain; m++) {
                if ((0 != m) && (0 == m % 0x08)) {
                    kprintf(" ");
                }
                kprintf("   ");
            }
        }

        if (canon == TRUE) {
            kprintf(" |");
            for (m = 0; m < remain && m < 0x10; m++) {
                hexdump_print(b[n + m], DEBUG_ASCII);
            }
            kprintf("%s", "|");
        }
        kprintf("\n");
    }
}

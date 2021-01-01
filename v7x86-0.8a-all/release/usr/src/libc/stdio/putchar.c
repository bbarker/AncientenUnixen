/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * A subroutine version of the macro putchar
 */
#include <stdio.h>

#undef putchar

putchar(c)
register c;
{
	putc(c, stdout);
}

/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * A subroutine version of the macro getchar.
 */
#include <stdio.h>

#undef getchar

getchar()
{
	return(getc(stdin));
}

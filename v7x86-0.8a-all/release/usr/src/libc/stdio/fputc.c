/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#include <stdio.h>

fputc(c, fp)
FILE *fp;
{
	return(putc(c, fp));
}

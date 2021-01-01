/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#include <stdio.h>

fgetc(fp)
FILE *fp;
{
	return(getc(fp));
}

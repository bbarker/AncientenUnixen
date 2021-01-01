/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#include	<stdio.h>

fprintf(iop, fmt, args)
FILE *iop;
char *fmt;
{
	_doprnt(fmt, &args, iop);
	return(ferror(iop)? EOF: 0);
}

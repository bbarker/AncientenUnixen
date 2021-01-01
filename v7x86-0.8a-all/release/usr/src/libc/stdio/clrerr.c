/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#include	<stdio.h>

clearerr(iop)
register struct _iobuf *iop;
{
	iop->_flag &= ~(_IOERR|_IOEOF);
}

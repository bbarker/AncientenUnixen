/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#include <stdio.h>
#
/* for testing only */
#include "def.h"

testreach()
	{
	VERT v;
	for (v = 0; v < nodenum; ++v)
		fprintf(stderr,"REACH(%d) = %d\n",v,REACH(v));
	}

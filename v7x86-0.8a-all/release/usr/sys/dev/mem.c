/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/RATHOLE
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"

extern int end;

mmread(dev)
{
	register k, c;

	if (minor(dev) == 2)
		return;
	k = minor(dev) == 1;
	do
		if (mmvaloff(k))
			c = *(caddr_t)((k ? 0 : PHY) + u.u_offset);
	while (u.u_error == 0 && passc(c) >= 0);
}

mmwrite(dev)
{
	register k, c;

	if (minor(dev) == 2) {
		u.u_count = 0;
		return;
	}
	k = minor(dev) == 1;
	while (u.u_error == 0 && (c = cpass()) >= 0)
		if (mmvaloff(k))
			*(caddr_t)((k ? 0 : PHY) + u.u_offset) = c;
}

mmvaloff(k)
{
	if (!k) {
		if (u.u_offset < 0x1000000)
			return 1;
	} else if ((u.u_offset >= KBASE && u.u_offset < (off_t)&end) ||
		   (u.u_offset >= (off_t)&u && u.u_offset < KSTK))
		return 1;
	u.u_error = ENXIO;
	return 0;
}

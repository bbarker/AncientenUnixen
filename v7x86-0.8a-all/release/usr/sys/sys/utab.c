/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 1999 Robert Nordier.  All rights reserved. */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/seg.h"

extern int pdir[], upt[];

#define NT  0
#define ND  1
#define NS  2
#define XX  3

int uplo;
int uphi;

/* sutab and estabut */

sureg()
{
	register i, a, n;
	int taddr, daddr;
	struct text *tp;

	for (i = uplo, uplo = u.u_utab[NT] + u.u_utab[ND]; i > uplo;)
		upt[--i] = 0;
	for (i = uphi, uphi = 1023 - u.u_utab[NS]; i < uphi;)
		upt[++i] = 0;
	taddr = daddr = u.u_procp->p_addr;
	if ((tp = u.u_procp->p_textp) != NULL)
		taddr = tp->x_caddr;
	taddr *= PGSZ;
	daddr *= PGSZ;
	i = 0;
	a = taddr;
	for (n = u.u_utab[NT]; n--; a += PGSZ)
		upt[i++] = a | u.u_utab[XX];
	a = daddr + USIZE * PGSZ;
	for (n = u.u_utab[ND]; n--; a += PGSZ)
		upt[i++] = a | 7;
	i = uphi;
	for (n = u.u_utab[NS]; n--; a += PGSZ)
		upt[++i] = a | 7;
	invd();
}

estabur(nt, nd, ns, sep, xrw)
{
	if (nt + nd + ns > 1023)
		goto err;
	if (nt + nd + ns + USIZE > maxmem)
		goto err;
	u.u_utab[NT] = nt;
	u.u_utab[ND] = nd;
	u.u_utab[NS] = ns;
	u.u_utab[XX] = xrw;
	sureg();
	return (0);
err:
	u.u_error = ENOMEM;
	return (-1);
}

clearseg(d)
{
	unsigned xd;

	xd = PHY + ctob(d);
	bzero(xd, PGSZ);
}

copyseg(s, d)
{
	unsigned xs, xd;

	if (s == d)
		return;
	xs = PHY + ctob(s);
	xd = PHY + ctob(d);
	bcopy(xs, xd, PGSZ);
}

unsigned
physaddr(addr)
unsigned addr;
{
	unsigned d, t, o, x, z;
	unsigned *pt;

	d = addr >> 22;
	t = (addr >> 12) & 1023;
	o = addr & 4095;
	x = 0;
	z = 0;
	pt = (unsigned *)(PHY + pdir[d] & ~4095);
	if (pt != NULL) {
		x = pt[t] & ~4095;
		z = x + o;
	}
	return z;
}

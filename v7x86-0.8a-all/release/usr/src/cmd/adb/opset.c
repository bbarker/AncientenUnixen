/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2007 Robert Nordier.  All rights reserved. */

#include "defs.h"
#include "optab.h"

STRING errflg;
L_INT dot;
INT dotinc;
L_INT var[];

#define EA 1
#define IM 2
#define PR 3
#define RG 4

struct op {
	int t;
	int r;
	int p;
	int s;
	int z;
	unsigned n;
};

static char masks[8] = {
	0070, 0030, 0007, 0017, 0016, 0010, 0002, 0001
};

static struct op opr[3], *op;
static int cdsz;
static int dtsz;
static int ix;
static int oq;
static int spc;
static int got;

static dis(), eaddr(), pcrel();
static int suffix(), mask(), getbyte();
static unsigned getval();

printins(f, idsp, ins)
{
	spc = idsp;
	got = -1;
	if (idsp == NSP)
		got = ins >> 8;
	ix = 1;
	dis(ins & LOBYTE);
	dotinc = ix;
}

static
dis(c)
{
	struct tabs *ts;
	struct tab *tab, *end, *te;
	int wd, cz, mo, i, x, z;

	cdsz = dtsz = 4;
	while (c == 0x66 || c == 0x67) {
		if (c == 0x66)
			cdsz = 2;
		else
			dtsz = 2;
		c = getbyte();
	}
	if (c == 0x0f) {
		c = getbyte();
		i = tabs0fz;
		ts = tabs0f;
	} else {
		i = tabsz;
		ts = tabs;
	}
	wd = c & 1;
	cz = wd ? cdsz : 1;
	while (--i > 0)
		if (c == ts[i].val)
			break;
	tab = ts[i].tab;
	end = tab + ts[i].sz;
	if (i > 0)
		c = getbyte();
	for (te = tab; te < end; te++)
		if ((c & ~te->mask) == te->val)
			break;
	oq = mo = 0;
	op = opr;
	for (i = 0; i < 4 && (x = te->cmd[i]) != 0; i++) {
		if (x >= 0) {
			z = x & 7;
			if (z < 1 || z > 5)
				z = z == 0 ? 1 + wd : z == 6 ? cdsz : cz;
			x >>= 3;
			switch (x) {
			case 0:
				if (z == 1) {
					op->t = IM;
					op->n = getval(z, cdsz);
				} else {
					op->t = EA;
					op->r = op->p = op->s = -1;
					op->n = getval(z, 0);
					op->z = z;
				}
				break;
			case 1:
				eaddr(c, z);
				break;
			case 2:
				op->t = IM;
				op->n = getval(z, 0);
				break;
			case 3:
				pcrel(z);
				break;
			default:
				op->t = RG;
				if (x >= 8)
					op->r = x & 7;
				else {
					x &= 3;
					if (x == 3) {
						x = 0;
						z += 5;
					}
					op->r = mask(c, masks[x]);
				}
				op->z = z;
			}
			op++;
		} else
			switch (x & 037) {
			default:
				mo = mask(c, masks[x & 0017]);
				break;
			case 020:
				c = getbyte();
				break;
			case 021:
				mo = cdsz == 4;
				break;
			case 022:
				opr[2].n = opr[1].n;
				opr[1].n = opr[0].n;
				opr[0].n = opr[2].n;
				break;
			}
	}
	prints(str[(te->num & 0x3ff) + mo]);
	if (te->num & FX) {
		if (!oq)
			printc(suffix(cz));
	} else if (te->num & FC)
		printc(suffix(cdsz));
	printf("%8t");
	for (i = 0; op-- > opr; i++) {
		printc(i == 0 ? ' ' : ',');
		if (te->num & FJ)
			printc('*');
		switch (op->t) {
		case EA:
			if (op->n)
				psymoff(op->n, DSYM, "");
			if (op->r != -1 || op->p != -1 || op->s != -1) {
				printc('(');
				if (op->r != -1)
					prints(str[op->z * 8 + op->r]);
				if (op->p != -1)
					printf(",%s", str[op->z * 8 + op->p]);
				if (op->s != -1)
					printf(",%d", 1 << op->s);
				printc(')');
			}
			break;
		case IM:
			printc('$');
			psymoff(op->n, DSYM, "");
			break;
		case PR:
			psymoff(op->n, ISYM, "");
			break;
		case RG:
			if (op->z <= 4)
				prints(str[op->z * 8 + op->r]);
			else
				printf(str[op->z - 4], op->r);
			break;
		}
	}
}

static int
suffix(n)
{
	switch (n) {
	case 1:
		return 'b';
	case 2:
		return 'w';
	case 4:
		return 'l';
	default:
		return '?';
	}
}

static
eaddr(c, z)
{
	static char dtp[8] = {6, 7, 6, 7, -1, -1, -1, -1};
	static char dtr[8] = {3, 3, 5, 5, 6, 7, 5, 3};
	int n, d;

	op->p = op->s = -1;
	op->n = 0;

	op->r = mask(c, 0007);
	n = mask(c, 0300);
	if (n < 3) {
		op->t = EA;
		op->z = dtsz;
		d = n == 1 ? 1 : n == 2 ? op->z : 0;
		if (op->z == 2) {
			op->p = dtp[op->r];
			op->r = dtr[op->r];
		} else if (op->r == 4) {
			c = getbyte();
			op->r = mask(c, 0007);
			op->p = mask(c, 0070);
			op->s = mask(c, 0300);
			if (op->p == 4)
				op->p = -1;
		}
		if (n == 0 && op->r == 5 && (op->z == 4 || op->p == -1)) {
			op->r = -1;
			d = op->z;
		}
		op->n = getval(d, dtsz);
	} else {
		op->t = RG;
		op->z = z;
		oq = 1;
	}
}

static
pcrel(n)
{
	unsigned x;

	x = getval(n, 0);
	switch (n) {
	case 1:
		x = (char)x;
		break;
	case 2:
		x = (short)x;
		break;
	default:
		x = (int)x;
		break;
	}
	op->t = PR;
	op->n = dot + ix + x;
}

static int
mask(n, m)
{
	n &= m;
	while ((m & 1) == 0) {
		m >>= 1;
		n >>= 1;
	}
	return n;
}

static unsigned
getval(n, sz)
{
	unsigned x;
	int i;

	x = 0;
	for (i = 0; i < n; i++)
		x |= (unsigned)getbyte() << (i << 3);
	if (sz) {
		if (n == 1)
			x = (char)x;
		else if (n == 2)
			x = (short)x;
		if (sz == 2)
			x = (unsigned short)x;
	}
	return x;
}

static int
getbyte()
{
	int c;

	if (got != -1) {
		c = got;
		got >>= 8;
	} else
		c = chkget(inkdot(ix), spc) & LOBYTE;
	ix++;
	return c;
}

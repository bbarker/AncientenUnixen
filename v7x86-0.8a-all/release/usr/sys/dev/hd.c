/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2007 Robert Nordier.  All rights reserved. */

/*
 * ATA hard disk driver
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/part.h"

extern int insw(), outsw();

/* controller ports */
#define CB1	0x1f0		/* 0x1f0 or 0x170 */
#define CB2	0x3f0		/* 0x3f0 or 0x370 */

/* controller registers */
#define DATA	(CB1+0)		/* data reg */
#define FR	(CB1+1)		/* feature reg */
#define SC	(CB1+2)		/* sector count */
#define SN	(CB1+3)		/* sector number */
#define CL	(CB1+4)		/* cylinder low */
#define CH	(CB1+5)		/* cylinder high */
#define DH	(CB1+6)		/* device head */
#define STAT	(CB1+7)		/* primary status */
#define CMD	(CB1+7)		/* command */
#define ASTAT	(CB2+6)		/* alternate status */
#define DC	(CB2+6)		/* device control */

/* status flags */
#define BSY	0x80		/* busy */
#define DF	0x20		/* device fault */
#define DRQ	0x08		/* data request */
#define ERR	0x01		/* error */

/* misc bits */
#define DEV0	0xa0		/* device 0 */
#define DEV1	0xb0		/* device 1 */
#define LBA	0x40		/* LBA mode */

/* commands */
#define RDSECT	0x20		/* read sector(s) */
#define WRSECT	0x30		/* write sector(s) */

#define MAXBLK	0xfffffff	/* maximum block number */
#define ERR1	1		/* error: timeout */
#define ERR2	2		/* error: other */
#define DK_N	0		/* monitoring device bit */

static struct {
	int dn;			/* device number */
	int bc;			/* block count */
	int rd;			/* read flag */
	daddr_t bn;		/* block number */
	caddr_t addr;		/* transfer address */
} hd;

static int drv = -1;

static struct {
	unsigned base;
	unsigned size;
} part[2][8];

static struct {
	unsigned base;
	unsigned size;
} pseudo[8] = {
	0,	10000,		/* a */
	10000,	32000,		/* b */
	0,	MAXBLK,		/* c */
	0,	0,		/* d */
	42000,	255000,		/* e */
	297000,	255000,		/* f */
	552000,	255000,		/* g */
	807000,	255000		/* h */
};

struct buf rhdbuf;
struct buf hdtab;

static unptpd(), delay();
static int doio(), await();

hdopen(dev, rw)
dev_t dev;
{
	struct buf *bp;
	struct ptent *pe;
	int un, n, i;

	un = (minor(dev) & 0100) >> 6;
	if (part[un][0].size)
		return;
	part[un][0].size = MAXBLK;
	bp = bread((dev & 0300) | 2, 0);
	if ((bp->b_flags & B_ERROR) == 0 &&
	    *(unsigned short *)(bp->b_un.b_addr + BSIZE - 2) == PTMAGIC) {
		pe = (struct ptent *)(bp->b_un.b_addr + PTOFF);
		for (n = 0, i = 1; i <= NPTE; pe++, i++) {
			part[un][i].base = pe->base;
			part[un][i].size = pe->size;
			if (pe->type == PTID) {
				n++;
				part[un][8 - n].base = pe->base;
				part[un][8 - n].size = pe->size;
			}
		}
	}
	brelse(bp);
}

hdstrategy(bp)
register struct buf *bp;
{
	unsigned sz, nb;
	int un, pt, pd;

	unptpd(bp->b_dev, &un, &pt, &pd);
	sz = part[un][pt].size;
	if (sz < pseudo[pd].base)
		sz = 0;
	else {
		sz -= pseudo[pd].base;
		if (sz > pseudo[pd].size)
			sz = pseudo[pd].size;
	}
	nb = (bp->b_bcount + BMASK) >> BSHIFT;
	if (nb > 0377 || bp->b_blkno + nb > sz) {
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	if (nb == 0) {
		bp->b_resid = 0;
		iodone(bp);
		return;
	}
	bp->av_forw = NULL;
	spl5();
	if (hdtab.b_actf == NULL)
		hdtab.b_actf = bp;
	else
		hdtab.b_actl->av_forw = bp;
	hdtab.b_actl = bp;
	if (hdtab.b_active == 0)
		hdstart();
	spl0();
}

hdstart()
{
	struct buf *bp;
	int un, pt, pd;

	if ((bp = hdtab.b_actf) == NULL)
		return;
	hdtab.b_active++;
	unptpd(bp->b_dev, &un, &pt, &pd);
	hd.dn = un;
	hd.bn = part[un][pt].base + pseudo[pd].base + bp->b_blkno;
	hd.bc = (bp->b_bcount + BMASK) >> BSHIFT;
	hd.addr = bp->b_un.b_addr;
	hd.rd = bp->b_flags & B_READ;
	dk_busy |= 1 << DK_N;
	dk_numb[DK_N] += 1;
	dk_wds[DK_N] += bp->b_bcount >> 6;
	hdio(0);
}

hdintr()
{

	if (hdtab.b_active == 0)
		return;
	dk_busy &= ~(1 << DK_N);
	hdio(1);
}

hdio(st)
{
	struct buf *bp;
	int err;

	if ((err = hdsm(st)) != -1) {
		bp = hdtab.b_actf;
		hdtab.b_active = 0;
		if (err) {
			deverror(bp, err, hd.bn);
			if (++hdtab.b_errcnt <= 10) {
				hdstart();
				return;
			}
			bp->b_flags |= B_ERROR;
		}
		hdtab.b_errcnt = 0;
		hdtab.b_actf = bp->av_forw;
		bp->b_resid = 0;
		iodone(bp);
		hdstart();
	}
}

int
hdsm(st)
{
	int x;

	switch (st) {
	case 0:
		if (await(STAT, BSY | DRQ))
			return ERR1;
		if (drv != hd.dn) {
			outb(DH, hd.dn ? DEV1 : DEV0);
			drv = hd.dn;
			delay();
			if (await(STAT, BSY | DRQ))
				return ERR1;
		}
		outb(DC, 0);
		outb(FR, 0);
		outb(SC, hd.bc);
		outb(SN, hd.bn);
		outb(CL, hd.bn >> 8);
		outb(CH, hd.bn >> 16);
		outb(DH, LBA | (drv ? DEV1 : DEV0) | hd.bn >> 24 & 0xf);
		outb(CMD, hd.rd ? RDSECT : WRSECT);
		if (!hd.rd) {
			delay();
			if (await(ASTAT, BSY))
				return ERR1;
			if (doio(inb(STAT)))
				return ERR2;
		}
		return -1;
	case 1:
		x = inb(STAT);
		if (hd.rd || hd.bc) {
			if (doio(x))
				return ERR2;
			if (!hd.rd || hd.bc)
				return -1;
			delay();
			x = inb(STAT);
		}
		if (x & (BSY | DF | DRQ | ERR))
			return ERR2;
		return 0;
	}
}

int
hdread(dev)
{

	physio(hdstrategy, &rhdbuf, dev, B_READ);
}

int
hdwrite(dev)
{

	physio(hdstrategy, &rhdbuf, dev, B_WRITE);
}

static
unptpd(dev, un, pt, pd)
dev_t dev;
int *un;
int *pt;
int *pd;
{

	*un = (minor(dev) & 0100) >> 6;
	*pt = (minor(dev) & 070) >> 3;
	*pd = minor(dev) & 07;
}

static int
doio(x)
{

	if ((x & (BSY | DRQ)) == DRQ) {
		(hd.rd ? insw : outsw)(DATA, hd.addr, BSIZE >> 1);
		hd.addr += BSIZE;
		hd.bc--;
	}
	return x & (BSY | DF | ERR) || (x & DRQ) == 0 ? -1 : 0;
}

static int
await(addr, mask)
{
	int i;

	for (i = 0; i < 0x10000000; i++)
		if ((inb(addr) & mask) == 0)
			return 0;
	return -1;
}

static
delay()
{

	inb(ASTAT);
	inb(ASTAT);
	inb(ASTAT);
	inb(ASTAT);
}

/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2007 Robert Nordier.  All rights reserved. */

/*
 * ATAPI CD-ROM driver
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/user.h"

/* controller ports */
#define CB1	0x170		/* 0x1f0 or 0x170 */
#define CB2	0x370		/* 0x3f0 or 0x370 */

/* controller registers */
#define DATA	(CB1+0) 	/* data reg */
#define FR	(CB1+1) 	/* feature reg */
#define SC	(CB1+2) 	/* sector count */
#define SN	(CB1+3) 	/* sector number */
#define CL	(CB1+4) 	/* cylinder low */
#define CH	(CB1+5) 	/* cylinder high */
#define DH	(CB1+6) 	/* device head */
#define STAT	(CB1+7) 	/* primary status */
#define CMD	(CB1+7) 	/* command */
#define ASTAT	(CB2+6) 	/* alternate status */
#define DC	(CB2+6) 	/* device control */

/* status flags */
#define BSY	0x80		/* busy */
#define DF	0x20		/* device fault */
#define DRQ	0x08		/* data request */
#define ERR	0x01		/* error */

/* misc bits */
#define DEV0	0x00		/* device 0 */
#define DEV1	0x10		/* device 1 */

/* commands */
#define CMDPKT	0xa0		/* PACKET */
#define CMDRST	0x08		/* DEVICE RESET */

/* various */
#define CDBSIZE 2048		/* block size */
#define CDSHIFT 11		/* LOG2(CDBSIZE) */
#define PKTSZ	12		/* packet size */
#define DPBC	CDBSIZE 	/* max byte count */
#define DK_N	2		/* monitoring device bit */

/* timer parameters */
#define CDTIME	(HZ/5)
#define CDTOUT	50

/* timer states */
#define TMOFF	0
#define TMDONE	1
#define TMON	2

/* driver states */
#define STCMD	1
#define STPKT	2
#define STINT	3
#define STCMDX	4
#define STPKTX	5
#define STRSTX	6
#define STENDX	7
#define STX	4

struct buf cdtab;
struct buf rcdbuf;

static struct {
	int st; 		/* state */
	int dn; 		/* device number */
	int bc; 		/* block count */
	daddr_t bn;		/* block number */
	caddr_t addr;		/* transfer address */
} cd;

static int drv = -1;
static int trq;
static int tticks;

cdtimer();
static delay();

cdstrategy(bp)
register struct buf *bp;
{

	bp->av_forw = (struct buf *)NULL;
	spl5();
	if (cdtab.b_actf == NULL)
		cdtab.b_actf = bp;
	else
		cdtab.b_actl->av_forw = bp;
	cdtab.b_actl = bp;
	if (cdtab.b_active == 0)
		cdstart();
	spl0();
}

cdstart()
{
	struct buf *bp;
	int s;

	if ((bp = cdtab.b_actf) == NULL) {
		trq = TMDONE;
		return;
	}
	cdtab.b_active++;
	s = spl6();
	if (trq == TMOFF)
		timeout(cdtimer, NULL, CDTIME);
	trq = TMON;
	splx(s);
	tticks = 0;
	cd.dn = minor(bp->b_dev) & 1;
	cd.bn = bp->b_blkno >> (CDSHIFT - BSHIFT);
	cd.bc = bp->b_bcount >> CDSHIFT;
	cd.addr = bp->b_un.b_addr;
	dk_busy |= 1 << DK_N;
	dk_numb[DK_N] += 1;
	dk_wds[DK_N] += bp->b_bcount >> 6;
	cdio(STCMD);
}

cdintr()
{

	if (cdtab.b_active == 0)
		return;
	dk_busy &= ~(1 << DK_N);
	tticks = 0;
	cdio(STINT);
}

cdtimer()
{

	if (trq == TMDONE)
		trq = TMOFF;
	else {
		timeout(cdtimer, NULL, CDTIME);
		if (cd.st && ++tticks > CDTOUT) {
			printf("cd: timeout (%d)\n", cd.st);
			cd.st = STRSTX;
			tticks = 0;
		}
		if (cd.st & STX)
			cdio(cd.st);
	}
}

cdio(st)
{
	struct buf *bp;
	int x;

	x = cdsm(st);
	cd.st = x > 0 ? x : 0;
	if (cd.st == 0) {
		bp = cdtab.b_actf;
		cdtab.b_active = NULL;
		if (x) {
			deverror(bp, -x, cd.bn);
			if (++cdtab.b_errcnt <= 10) {
				cdstart();
				return;
			}
			bp->b_flags |= B_ERROR;
		}
		cdtab.b_errcnt = 0;
		cdtab.b_actf = bp->av_forw;
		bp->b_resid = 0;
		iodone(bp);
		cdstart();
	}
}

cdsm(st)
{
	char pkt[PKTSZ];
	int x;

	switch (st) {
	case STCMD:
	case STCMDX:
		if (drv != cd.dn) {
			if (inb(STAT) & (BSY | DRQ))
				return STCMDX;
			outb(DH, cd.dn ? DEV1 : DEV0);
			drv = cd.dn;
			delay();
		}
		if (inb(STAT) & (BSY | DRQ))
			return STCMDX;
		outb(DC, 0);
		outb(FR, 0);
		outb(SC, 0);
		outb(SN, 0);
		outb(CL, DPBC & 0xff);
		outb(CH, DPBC >> 8);
		outb(DH, drv ? DEV1 : DEV0);
		outb(CMD, CMDPKT);
		delay();
	case STPKT:
	case STPKTX:
		if (inb(ASTAT) & BSY)
			return STPKTX;
		if ((inb(STAT) & (BSY | DRQ | ERR)) != DRQ)
			return -1;
		bzero(pkt, PKTSZ);
		pkt[0] = 0x28;
		pkt[2] = cd.bn >> 030;
		pkt[3] = cd.bn >> 020;
		pkt[4] = cd.bn >> 010;
		pkt[5] = cd.bn;
		pkt[6] = cd.bc >> 020;
		pkt[7] = cd.bc >> 010;
		pkt[8] = cd.bc;
		outsw(DATA, pkt, PKTSZ >> 1);
		return STINT;
	case STINT:
		x = inb(STAT);
		if ((x & (BSY | DRQ)) == 0)
			return x & (DF | ERR) ? -2 : 0;
		if ((x & (BSY | DRQ)) != DRQ)
			return -1;
		x = inb(CH) << 8 | inb(CL);
		if (x == 0)
			return -1;
		insw(DATA, cd.addr, (x + 1) >> 1);
		cd.addr += x;
		return STINT;
	case STRSTX:
		outb(CMD, CMDRST);
		delay();
	case STENDX:
		if (inb(ASTAT) & BSY)
			return STENDX;
		if (inb(STAT) & (BSY | DF | DRQ | ERR))
			return -1;
		return 0;
	}
}

cdread(dev)
{

	physio(cdstrategy, &rcdbuf, dev, B_READ);
}

static
delay()
{

	inb(ASTAT);
	inb(ASTAT);
	inb(ASTAT);
	inb(ASTAT);
}

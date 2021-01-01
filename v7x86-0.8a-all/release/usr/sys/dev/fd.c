/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 1999 Robert Nordier.  All rights reserved. */

/*
 * Floppy disk driver
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/systm.h"

#define NFDBLK	2880		/* Sectors per disk */
#define FDSPT	18		/* Sectors per track */

/* Ports */
#define FDDOR	0x3f2		/* Digital Output Register */
#define FDMSR	0x3f4		/* Main Status Register */
#define FDDSR	0x3f4		/* Data Rate Select Register */
#define FDDAT	0x3f5		/* Data Register */
#define FDDIR	0x3f7		/* Digital Input Register */
#define FDCCR	0x3f7		/* Configuration Control Register */

/* Main Status Register */
#define FDBSY	0x10		/* Busy with command */
#define FDDIO	0x40		/* Data Input/Output */
#define FDRQM	0x80		/* Request for Master */

/* Command opcodes */
#define FDCNO	0		/* No operation (dummy) */
#define FDSPC	0x03		/* Specify */
#define FDCRC	0x07		/* Recalibrate */
#define FDCSK	0x0f		/* Seek */
#define FDCWR	0xc5		/* Write Data */
#define FDCRD	0xe6		/* Read Data */
#define FDCSI	0x08		/* Sense Interrupt Status */
#define FDCRS	0xff		/* Reset (dummy) */

/* Media related */
#define FDGPL	27		/* Read/write Gap Length */

/* Errors */
#define FDECU	1		/* Controller unavailable */
#define FDECE	2		/* Controller error */
#define FDETO	3		/* Disk timeout */
#define FDESF	4		/* Seek failure */
#define FDEWP	5		/* Write-protected disk */
#define FDEOV	6		/* DMA overrun */
#define FDENF	7		/* Sector not found */
#define FDEIO	8		/* I/O error */

/* Configurable */
#define FDBUF	0x0e00		/* DMA buffer */
#define FDDEL	1000		/* Delay for RQM */
#define FDTRY	3		/* Retries on error */

#define DK_N	1		/* monitoring device bit */

struct {
	int err;		/* Error code */
	int pcn;		/* Present cylinder number */
	int mtr;		/* Motor state */
	int cmd;		/* Command opcode */
	int try;		/* Retry counter */
	int rd; 		/* Read or write */
	int cn; 		/* Cylinder number */
	int hn; 		/* Head number */
	int sn; 		/* Sector number */
	unsigned int bleft;	/* Bytes left to be transferred */
	unsigned int bpart;	/* Bytes transferred */
	caddr_t addr;		/* Transfer address */
	unsigned phys;		/* Physical transfer address */
} fd = {
	0, -1
};

static unsigned tick;		/* Timer tick */
static int trq; 		/* Timer request */

struct buf rfdbuf;
struct buf fdtab;

fdtimer();

void
fdstrategy(bp)
struct buf *bp;
{
	long sz;

	sz = (bp->b_bcount + BMASK) >> BSHIFT;
	if (bp->b_blkno + sz > NFDBLK) {
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	bp->av_forw = NULL;
	spl5();
	if (fdtab.b_actf == NULL)
		fdtab.b_actf = bp;
	else
		fdtab.b_actl->av_forw = bp;
	fdtab.b_actl = bp;
	if (fdtab.b_active == 0)
		fdstart();
	spl0();
}

fdstart()
{
	struct buf *bp;
	daddr_t bn;
	int s;

	if ((bp = fdtab.b_actf) == NULL) {
		s = spl6();
		if (fd.mtr) {
			fd.mtr = 3;
			tick = 0;
		}
		splx(s);
		return;
	}
	fdtab.b_active++;

	bn = bp->b_blkno;
	fd.cn = bn / FDSPT / 2;
	fd.hn = bn / FDSPT % 2;
	fd.sn = bn % FDSPT + 1;
	fd.bleft = bp->b_bcount;
	fd.addr = bp->b_un.b_addr;
	fd.rd = bp->b_flags & B_READ;

	/* Handle the timer and motor. */
	s = spl6();
	if (fd.mtr == 0) {
		timeout(fdtimer, NULL, HZ / 5);
		outb(FDDOR, 0x1c);
		fd.mtr = 1;
		tick = 0;
		fdcmd(FDSPC);
		outb(FDCCR, 0);
	} else if (fd.mtr == 3)
		fd.mtr = 2;
	splx(s);

	dk_busy |= 1 << DK_N;
	dk_numb[DK_N] += 1;
	dk_wds[DK_N] += bp->b_bcount >> 6;
	fdio();
}

fdintr()
{
	struct buf *bp;

	if (fdtab.b_active == 0)
		return;
	dk_busy &= ~(1 << DK_N);
	if (!fdx2()) {
		fdx1();
		return;
	}
	bp = fdtab.b_actf;
	if (fd.err) {
		deverror(bp, fd.err, (fd.cn * 2 + fd.hn) * FDSPT + (fd.sn - 1));
		if (fd.err != FDEWP && ++fdtab.b_errcnt <= 10) {
			fd.pcn = -1;
			fdstart();
			return;
		}
		bp->b_flags |= B_ERROR;
		fd.bpart = fd.bleft;
	}
	if (fd.rd && fd.phys == FDBUF)
		bcopy(PHY + fd.phys, fd.addr, fd.bpart);
	if ((fd.bleft -= fd.bpart) > 0) {
		fd.addr += fd.bpart;
		fd.cn++;
		fd.hn = 0;
		fd.sn = 1;
		fdio();
		return;
	}
	fdtab.b_active = 0;
	fdtab.b_errcnt = 0;
	fdtab.b_actf = bp->av_forw;
	bp->b_resid = 0;
	iodone(bp);
	fdstart();
}

fdio()
{
	unsigned sz;

	fd.bpart = ((2 - fd.hn) * FDSPT - (fd.sn - 1)) * BSIZE;
	if (fd.bpart > fd.bleft)
		fd.bpart = fd.bleft;
	fd.phys = physaddr((unsigned)fd.addr);
	/* Avoid doing DMA across a 64K boundary. */
	sz = (0x10000 - (fd.phys & 0xffff)) & ~(BSIZE - 1);
	if (sz == 0) {
		fd.phys = FDBUF;
		sz = BSIZE;
	}
	if (fd.bpart > sz)
		fd.bpart = sz;
	if (!fd.rd && fd.phys == FDBUF)
		bcopy(fd.addr, PHY + fd.phys, fd.bpart);
	fd.err = fd.cmd = fd.try = 0;
	fdx1();
}

fdtimer()
{

	tick++;
	if (fd.mtr == 1 && tick > 2) {
		fd.mtr = 2;
	} else if (fd.mtr == 3 && tick > 10) {
		fd.mtr = 0;
		outb(FDDOR, 0xc);
	}
	if (trq == -1 && fd.mtr == 2) {
		trq = 0;
		fdx1();
	} else if (trq == 1 && tick > 10) {
		trq = 0;
		fd.err = FDETO;
		fdx1();
	}
	if (fd.mtr)
		timeout(fdtimer, NULL, HZ / 5);
}

int
fdread(dev)
{

	physio(fdstrategy, &rfdbuf, dev, B_READ);
}

int
fdwrite(dev)
{

	physio(fdstrategy, &rfdbuf, dev, B_WRITE);
}

static
fdx1()
{

	if (!fd.err) {
		fd.cmd = FDCNO;
		if (fd.pcn < 0)
			fd.cmd = FDCRC;
		else if (fd.pcn != fd.cn)
			fd.cmd = FDCSK;
		else {
			if (fd.mtr == 1) {
				trq = -1;
				tick = 0;
				return;
			}
			fddma();
			fd.cmd = fd.rd ? FDCRD : FDCWR;
		}
		if (!fdcmd(fd.cmd)) {
			trq = 1;
			tick = 0;
		}
	}
	if (fd.err) {
		fdrst();
		fd.cmd = FDCRS;
	}
}

static
fdx2()
{
	char av[8];
	int i, e;

	trq = 0;
	switch (fd.cmd) {
	case FDCRC:
	case FDCSK:
		if (!fdsis(av))
			if (av[0] & 0xc0) {
				if (av[0] & 0x80)
					fd.err = FDECE;
				else if (fd.cmd == FDCRC && fd.pcn == -1)
					fd.pcn = -2;
				else
					fd.err = FDESF;
			} else
				fd.pcn = fd.cmd == FDCRC ? 0 : fd.cn;
		return 0;
	case FDCRD:
	case FDCWR:
		for (i = 0; i < 7; i++)
			if (fdget(&av[i]))
				return 0;
		if ((av[0] & 0xc0) == 0)
			e = 0;
		else {
			if (av[1] & 0x02)
				e = FDEWP;
			else if (av[1] & 0x10)
				e = FDEOV;
			else if (av[1] & 0x20 || av[2] & 0x20)
				e = FDEIO;
			else
				e = FDENF;
			if (e != FDEWP && ++fd.try <= FDTRY)
				return 0;
		}
		fd.err = e;
		return 1;
	case FDCRS:
		for (i = 0; i < 4; i++)
			if (fdsis(av))
				break;
		return 1;
	}
}

static
fddma()
{

	/* Set up DMA 1 channel 2. */
	outb(0x0a, 0x6);
	outb(0x0c, 0);
	outb(0x0b, fd.rd ? 0x46 : 0x4a);
	outb(0x04, fd.phys);
	outb(0x04, fd.phys >> 8);
	outb(0x81, fd.phys >> 16);
	outb(0x05, fd.bpart - 1);
	outb(0x05, (fd.bpart - 1) >> 8);
	outb(0x0a, 2);
}

static int
fdcmd(cmd)
int cmd;
{
	unsigned char av[9];
	int n, i;

	av[0] = cmd;
	switch (cmd) {
	case FDSPC:
		av[1] = 0xdf;
		av[2] = 2;
		n = 3;
		break;
	case FDCRC:
		av[1] = 0;
		n = 2;
		break;
	case FDCSK:
		av[1] = fd.hn << 2;
		av[2] = fd.cn;
		n = 3;
		break;
	case FDCRD:
	case FDCWR:
		av[1] = fd.hn << 2;
		av[2] = fd.cn;
		av[3] = fd.hn;
		av[4] = fd.sn;
		av[5] = 2;
		av[6] = FDSPT;
		av[7] = FDGPL;
		av[8] = 0xff;
		n = 9;
	}
	for (i = 0; i < n; i++)
		if (fdput(av[i]))
			return -1;
	return 0;
}

static
fdrst()
{

	outb(FDDOR, 0x10);
	outb(FDDOR, 0x1c);
}

static int
fdsis(av)
char *av;
{

	if (fdput(FDCSI) || fdget(av))
		return -1;
	if ((*av & 0xc0) == 0x80) {
		fd.err = FDECE;
		return -1;
	}
	return fdget(av + 1);
}

static int
fdput(x)
{
	int r;

	if ((r = fdrqm(0)) == 0)
		outb(FDDAT, x);
	return r;
}

static int
fdget(x)
char *x;
{
	int r;

	if ((r = fdrqm(FDDIO)) == 0)
		*x = inb(FDDAT);
	return r;
}

static int
fdrqm(m)
{
	int i, x;

	for (i = 0; i < FDDEL; i++) {
		x = inb(FDMSR);
		if (x & FDRQM) {
			if ((x & FDDIO) == m)
				return 0;
			break;
		}
	}
	fd.err = FDECU;
	return -1;
}

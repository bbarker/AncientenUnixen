/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 1999 Robert Nordier.  All rights reserved. */

/*
 * Serial driver: 8250 UART
 */

#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/systm.h"

#define NSR	2
#define SSPEED	13		/* 9600 bps */

/* 8250 registers */
#define RBR	0		/* receiver buffer register */
#define THR	0		/* transmitter hold register */
#define IER	1		/* interrupt enable register */
#define IIR	2		/* interrupt identification register */
#define LCR	3		/* line control register */
#define MCR	4		/* modem control register */
#define LSR	5		/* line status register */
#define DLL	0		/* divisor latch (lsb) */
#define DLM	1		/* divisor latch (msb) */

/* line control */
#define DATA7	0002		/* data bits: 7 */
#define DATA8	0003		/* data bits: 8 */
#define STOP1	0000		/* stop bits: 1 */
#define PARTN	0000		/* parity none */
#define PARTO	0010		/* parity odd */
#define PARTE	0030		/* parity even */
#define DLAB	0200		/* divisor latch access bit */

/* line status */
#define DR	0001		/* data ready */
#define THRE	0040		/* transmitter holding register empty */

/*
 * Speed table
 */
short srstab[] = {
	0,			/* 0 */
	2304,			/* 50 */
	1536,			/* 75 */
	1047,			/* 110 */
	857,			/* 134.5 */
	768,			/* 150 */
	576,			/* 200 */
	384,			/* 300 */
	192,			/* 600 */
	96,			/* 1200 */
	64,			/* 1800 */
	48,			/* 2400 */
	24,			/* 4800 */
	12,			/* 9600 */
	6,			/* 19200 */
	3			/* 38400 */
};

struct tty sr[NSR];

extern ttrstrt();
srstart();

sropen(dev, flag)
dev_t dev;
{
	struct tty *tp;
	int port, d;

	d = minor(dev);
	if (d >= NSR) {
		u.u_error = ENXIO;
		return;
	}
	tp = &sr[d];
	tp->t_addr = (caddr_t)(d == 0 ? 0x3f8 : 0x2f8);
	tp->t_oproc = srstart;
	if ((tp->t_state & ISOPEN) == 0) {
		tp->t_state = CARR_ON;
		tp->t_ispeed = SSPEED;
		tp->t_ospeed = SSPEED;
		tp->t_flags = RAW | ODDP | EVENP | ECHO;
		srparam(tp);
		port = (int)tp->t_addr;
		outb(port + MCR, inb(port + MCR) | 8);
		ttychars(tp);
	}
	ttyopen(dev, tp);
}

srclose(dev, flag)
dev_t dev;
{

	ttyclose(&sr[minor(dev)]);
}

srread(dev)
dev_t dev;
{

	ttread(&sr[minor(dev)]);
}

srwrite(dev)
dev_t dev;
{

	ttwrite(&sr[minor(dev)]);
}

srintr(dev)
dev_t dev;
{
	struct tty *tp;
	int port, st, c;

	tp = &sr[dev & 1];
	port = (int)tp->t_addr;
	switch (inb(port + IIR) & 7) {
	case 4:
		do {
			c = inb(port + RBR);
			ttyinput(c, tp);
		} while ((st = inb(port + LSR)) & DR);
		if ((st & THRE) == 0)
			break;
	case 2:
		tp->t_state &= ~BUSY;
		ttstart(tp);
	}
}

srioctl(dev, cmd, addr, flag)
dev_t dev;
caddr_t addr;
{
	struct tty *tp;

	tp = &sr[minor(dev)];
	if (ttioccomm(cmd, tp, addr, dev)) {
		if (cmd == TIOCSETP || cmd == TIOCSETN)
			srparam(tp);
	} else
		u.u_error = ENOTTY;
}

srstart(tp)
struct tty *tp;
{
	int port, c, i;

	if (tp->t_state & (TIMEOUT | BUSY | TTSTOP))
		return;
	if ((c = getc(&tp->t_outq)) >= 0) {
		if (c >= 0200 && (tp->t_flags & RAW) == 0) {
			tp->t_state |= TIMEOUT;
			timeout(ttrstrt, (caddr_t)tp, (c & 0177) + 6);
		} else {
			tp->t_char = c;
			tp->t_state |= BUSY;
			port = (int)tp->t_addr;
			for (i = 8192; i; i--)
				if ((inb(port + LSR) & THRE))
					break;
			if (i == 0)
				printf("timeout in srstart\n");
			outb(port + THR, c);
		}
		if (tp->t_outq.c_cc <= TTLOWAT && tp->t_state & ASLEEP) {
			tp->t_state &= ~ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
	}
}

srparam(tp)
struct tty *tp;
{
	int port, v, d, p;

	v = srstab[tp->t_ispeed];
	if (v == 0)
		return;
	p = PARTN;
	if (tp->t_flags & RAW)
		d = DATA8;
	else {
		d = DATA7;
		if (tp->t_flags & EVENP)
			p = PARTE;
		else if (tp->t_flags & ODDP)
			p = PARTO;
	}
	port = (int)tp->t_addr;
	outb(port + IER, 0);
	outb(port + LCR, DLAB);
	outb(port + DLL, v);
	outb(port + DLM, v >> 8);
	outb(port + LCR, d | p | STOP1);
	outb(port + IIR, 0x81);
	inb(port + LSR);
	inb(port + RBR);
	outb(port + IER, 3);
}

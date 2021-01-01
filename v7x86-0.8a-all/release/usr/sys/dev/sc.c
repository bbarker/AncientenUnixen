/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2007 Robert Nordier.  All rights reserved. */

/*
 * System console driver
 */

#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/systm.h"

#ifdef MONO
#define SCRBUF	0xb0000
#define SCRPRT	0x3b4
#else
#define SCRBUF	0xb8000
#define SCRPRT	0x3d4
#endif

/* ports */
#define KDT	0x60		/* data */
#define KST	0x64		/* status */

/* status register */
#define KOB	0x01		/* output buffer */
#define KIB	0x02		/* input buffer */
#define KTO	0x40		/* timeout */
#define KPE	0x80		/* parity error */

/* commands */
#define KWL	0xed		/* write leds */
#define KSD	0xf6		/* set defaults */

/* scan codes */
#define CTRL	0x1d		/* ctrl */
#define LSHIFT	0x2a		/* left shift */
#define RSHIFT	0x36		/* right shift */
#define ALT	0x38		/* alt */
#define CAPS	0x3a		/* caps lock */
#define NUM	0x45		/* num lock */
#define SCROLL	0x46		/* scroll lock */

#define BRK	0x80		/* break flag */

/* LED flags */
#define LSCR	0x1		/* scroll lock */
#define LNUM	0x2		/* num lock */
#define LCAP	0x4		/* caps lock */

#define TOV	1048576 	/* timeout */

#define ROWS	25		/* number of rows */
#define COLS	80		/* number of columns */
#define TAB	8		/* tab size */

#define CHRS	(ROWS*COLS)	/* screen size */
#define LROW	(CHRS-COLS)	/* last row */
#define LCOL	(COLS-1)	/* last column */
#define LTAB	(COLS-TAB)	/* last tab */

/* video attributes */
#define ATNL	0x0700		/* normal low */
#define ATNH	0x0f00		/* normal high */
#define ATRL	0x7000		/* reverse low */
#define ATRH	0xf000		/* reverse high */

/* video mode flags */
#define VMSO	0x01		/* standout mode */
#define VMUS	0x02		/* underscore mode */

/* beep parameters */
#define BEEPPT	(0x1234dd/453)	/* pitch */
#define BEEPTM	(HZ/10) 	/* duration */

#define clear(p,n)	scset(scr+(p),va|' ',(n))

/* keyboard maps */
static char map[3][96] = {
	{
		0377, 0033, 0061, 0062, 0063, 0064, 0065, 0066,
		0067, 0070, 0071, 0060, 0055, 0075, 0010, 0011,
		0161, 0167, 0145, 0162, 0164, 0171, 0165, 0151,
		0157, 0160, 0133, 0135, 0012, 0377, 0141, 0163,
		0144, 0146, 0147, 0150, 0152, 0153, 0154, 0073,
		0047, 0140, 0377, 0134, 0172, 0170, 0143, 0166,
		0142, 0156, 0155, 0054, 0056, 0057, 0377, 0052,
		0377, 0040, 0377, 0321, 0322, 0323, 0324, 0325,
		0326, 0327, 0330, 0331, 0332, 0377, 0377, 0310,
		0301, 0306, 0055, 0304, 0377, 0303, 0053, 0305,
		0302, 0307, 0311, 0177, 0377, 0377, 0377, 0377,
		0377, 0377, 0377, 0377, 0377, 0377, 0377, 0377
	},
	{
		0377, 0033, 0041, 0100, 0043, 0044, 0045, 0136,
		0046, 0052, 0050, 0051, 0137, 0053, 0377, 0377,
		0121, 0127, 0105, 0122, 0124, 0131, 0125, 0111,
		0117, 0120, 0173, 0175, 0012, 0377, 0101, 0123,
		0104, 0106, 0107, 0110, 0112, 0113, 0114, 0072,
		0042, 0176, 0377, 0174, 0132, 0130, 0103, 0126,
		0102, 0116, 0115, 0074, 0076, 0077, 0377, 0052,
		0377, 0040, 0377, 0377, 0377, 0377, 0377, 0377,
		0377, 0377, 0377, 0377, 0377, 0377, 0377, 0067,
		0070, 0071, 0055, 0064, 0065, 0066, 0053, 0061,
		0062, 0063, 0060, 0056, 0377, 0377, 0377, 0377,
		0377, 0377, 0377, 0377, 0377, 0377, 0377, 0377
	},
	{
		0377, 0377, 0377, 0000, 0377, 0377, 0377, 0036,
		0377, 0377, 0377, 0377, 0037, 0377, 0377, 0377,
		0021, 0027, 0005, 0022, 0024, 0031, 0025, 0011,
		0017, 0020, 0033, 0035, 0012, 0377, 0001, 0023,
		0004, 0006, 0007, 0010, 0012, 0013, 0014, 0377,
		0377, 0377, 0377, 0034, 0032, 0030, 0003, 0026,
		0002, 0016, 0015, 0377, 0377, 0377, 0377, 0377,
		0377, 0000, 0377, 0377, 0377, 0377, 0377, 0377,
		0377, 0377, 0377, 0377, 0377, 0377, 0377, 0377,
		0377, 0377, 0377, 0377, 0377, 0377, 0377, 0377,
		0377, 0377, 0377, 0377, 0377, 0377, 0377, 0377,
		0377, 0377, 0377, 0377, 0377, 0377, 0377, 0377
	}
};

/* caps/num lock table */
static char type[96] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 0, 0, 0, 0, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 1, 1, 1, 1,
	1, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 2,
	2, 2, 0, 2, 2, 2, 0, 2,
	2, 2, 2, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

/* alternate character set */
static unsigned char acs[32] = {
	0000, 0001, 0002, 0003, 0004, 0005, 0006, 0007,
	0010, 0011, 0012, 0013, 0014, 0015, 0016, 0017,
	0020, 0021, 0022, 0023, 0332, 0304, 0277, 0263,
	0331, 0300, 0032, 0033, 0034, 0035, 0036, 0037
};

struct tty sc;

int ttrstrt();
int scstart();

static int init;
static int cmd;
static int led;
static int tout;
static int curs;

short *scr = (short *)(0x7ff00000 + SCRBUF);

static int attr[] = {
	ATNL, ATNH, ATRL, ATRH
};
static int va = ATNL;

static int beepon;

char	*msgbufp = msgbuf;

scopen(dev, flag)
dev_t dev;
{
	register struct tty *tp;

	if (minor(dev) > 0) {
		u.u_error = ENXIO;
		return;
	}
	if (!init) {
		scinit();
		init++;
	}
	tp = &sc;
	tp->t_addr = (caddr_t)0;
	tp->t_oproc = scstart;
	if ((tp->t_state & ISOPEN) == 0) {
		tp->t_state = ISOPEN | CARR_ON;
		tp->t_flags = ECHO | XTABS;
		ttychars(tp);
	}
	ttyopen(dev, tp);
}

scclose(dev, flag)
dev_t dev;
{
	ttyclose(&sc);
}

scread(dev)
dev_t dev;
{
	ttread(&sc);
}

scwrite(dev)
dev_t dev;
{
	ttwrite(&sc);
}

scxint(dev)
dev_t dev;
{
	register struct tty *tp;

	tout = 0;
	tp = &sc;
	ttstart(tp);
	if (tp->t_state & ASLEEP && tp->t_outq.c_cc <= TTLOWAT) {
		tp->t_state &= ~ASLEEP;
		wakeup((caddr_t) & tp->t_outq);
	}
}

scrint()
{
	static int ctrl, shift, lock;
	int st, sn, ch, x, i;

	for (i = 1024; i != 0 && (st = inb(KST)) & KOB; i--) {
		sn = inb(KDT);
		if ((st & (KTO | KPE)) != 0)
			continue;
		switch (sn) {
		case 0x00:
		case 0xff:
			beep();
			break;
		case 0xfa:
			if (cmd == 2) {
				cmd++;
				kbput(KDT, led);
			} else {
				if (cmd == 0)
					printf("sc: ack without command\n");
				cmd = 0;
			}
			break;
		case 0xfe:
			printf("sc: nack\n");
			cmd = 0;
			break;
		case CTRL:
			ctrl = 1;
			break;
		case BRK | CTRL:
			ctrl = 0;
			break;
		case LSHIFT:
		case RSHIFT:
			shift = 1;
			break;
		case BRK | LSHIFT:
		case BRK | RSHIFT:
			shift = 0;
			break;
		case CAPS:
			lock ^= LCAP;
			break;
		case NUM:
			lock ^= LNUM;
			break;
		case SCROLL:
			lock ^= LSCR;
			break;
		default:
			if (sn < 0x60) {
				if (ctrl)
					x = 2;
				else {
					x = shift;
					if (lock & LCAP && type[sn] == 1 ||
					    lock & LNUM && type[sn] == 2)
						x ^= 1;
				}
				ch = map[x][sn];
				if (ch != 0xff) {
					if (ch & 0x80) {
						ttyinput(033, &sc);
						ch &= 0x7f;
					}
					ttyinput(ch, &sc);
				}
			}
		}
		if (led != lock && !cmd) {
			led = lock;
			cmd = 2;
			kbput(KDT, 0xed);
		}
	}
}

scioctl(dev, cmd, addr, flag)
caddr_t addr;
dev_t dev;
{
	if (ttioccomm(cmd, &sc, addr, dev) == 0)
		u.u_error = ENOTTY;
}

scstart(tp)
register struct tty *tp;
{
	register i, c;

	for (i = 0; (c = getc(&tp->t_outq)) >= 0; i++)
		if (tp->t_flags & RAW || c <= 0177)
			scputc(c);
	if (i && !tout) {
		tout = 1;
		timeout(scxint, NULL, 2);
	}
}

scinit()
{
	led = -1;
	cmd = 1;
	kbput(KDT, 0xf6);
}

scputc(ch)
{
	static int sm, vm, p0, st;
	static int pos = LROW;
	int col, x;

	ch &= 0177;
	col = pos % COLS;
	switch (st) {
	case 0:
		switch (ch) {
		case 0:
			return;
		case 7:
			beep();
			return;
		case 8:
			if (pos > 0)
				scr[--pos] = va | ' ';
			break;
		case 9:
			if (pos < LTAB)
				for (x = TAB - (col & TAB - 1); x; x--)
					scr[pos++] = va | ' ';
			break;
		case 10:
			pos += COLS;
			if (!curs)
				pos -= col;
			break;
		case 11:
			return;
		case 12:
			pos = 0;
			clear(0, CHRS);
			break;
		case 13:
			pos -= col;
			break;
		case 27:
			st = 1;
			return;
		case 20: case 21: case 22:
		case 23: case 24: case 25:
			ch = acs[ch];
		default:
			scr[pos++] = va | ch;
		}
		break;
	case 1:
		switch (ch) {
		case '@':	/* reset */
			curs = sm = vm = p0 = 0;
			break;
		case 'A':	/* cursor up */
			if (pos >= COLS)
				pos -= COLS;
			break;
		case 'B':	/* cursor down */
			pos += COLS;
			break;
		case 'C':	/* cursor right */
			pos++;
			break;
		case 'D':	/* cursor left */
			if (col > 0)
				pos--;
			break;
		case 'E':	/* clear screen */
			pos = 0;
			clear(0, CHRS);
			break;
		case 'F':	/* alt char set start */
			break;
		case 'G':	/* alt char set end */
			break;
		case 'H':	/* cursor home */
			pos = 0;
			break;
		case 'I':	/* back tab */
			if (col > 0)
				pos = (pos - 1) & ~(TAB - 1);
			break;
		case 'J':	/* clear to end of display */
			clear(pos, CHRS - pos);
			break;
		case 'K':	/* clear to end of line */
			clear(pos, COLS - col);
			break;
		case 'L':	/* insert line */
			vscroll(pos - col, 1);
			break;
		case 'M':	/* delete line */
			vscroll(pos - col, 0);
			break;
		case 'N':	/* delete character */
			hscroll(pos, col, 0);
			break;
		case 'O':	/* insert character */
			hscroll(pos, col, 1);
			break;
		case 'P':	/* scroll forward */
			vscroll(0, 0);
			break;
		case 'Q':	/* scroll reverse */
			vscroll(0, 1);
			break;
		case 'R':	/* reserved */
			break;
		case 'S':	/* standout start */
			vm |= VMSO;
			break;
		case 'T':	/* standout end */
			vm &= ~VMSO;
			break;
		case 'U':	/* underscore start */
			vm |= VMUS;
			break;
		case 'V':	/* underscore end */
			vm &= ~VMUS;
			break;
		case 'W':	/* cursor addressing mode start */
			curs = 1;
			break;
		case 'X':	/* cursor addressing mode end */
			curs = 0;
			break;
		case 'Y':	/* cursor motion (part 1) */
			st = 2;
			return;
		case 'Z':	/* special mode (part 1) */
			st = 4;
			return;
		}
		va = attr[vm];
		break;

	case 2: 	/* cursor motion (part 2) */
		p0 = ch;
		st++;
		return;
	case 3: 	/* cursor motion (part 3) */
		p0 -= ' ';
		ch -= ' ';
		if (p0 >= 0 && p0 < ROWS && ch >= 0 && ch < COLS)
			pos = p0 * COLS + ch;
		break;
	case 4: 	/* special mode (part 2) */
		sm = ch - ' ';
		break;
	}
	st = 0;
	if (pos >= CHRS) {
		vscroll(0, 0);
		pos -= COLS;
	}
	/* position hardware cursor */
	outb(SCRPRT, 0xe);
	outb(SCRPRT + 1, pos >> 8);
	outb(SCRPRT, 0xf);
	outb(SCRPRT + 1, pos);
}

kbput(prt, cmd)
{
	int i;

	for (i = TOV; i && (inb(KST) & KIB) != 0; i--);
	if (i == 0)
		printf("sc: keyboard write timeout\n");
	outb(prt, cmd);
}

vscroll(pos, rev)
{
	int n;

	n = CHRS - pos - COLS;
	if (n)
		if (rev)
			scmov(&scr[pos], &scr[pos + COLS], n);
		else
			scmov(&scr[pos + COLS], &scr[pos], n);
	clear(rev ? pos : LROW, COLS);
}

hscroll(pos, col, ins)
{
	int n;

	n = COLS - col - 1;
	if (n)
		if (ins)
			scmov(&scr[pos], &scr[pos + 1], n);
		else
			scmov(&scr[pos + 1], &scr[pos], n);
	scr[ins ? pos : pos + n] = va | ' ';
}

beepoff()
{
	/* 8255 ppi: speaker off */
	outb(0x61, inb(0x61) & ~3);
	beepon = 0;
}

beep()
{
	int s;

	s = spl7();
	if (!beepon) {
		beepon = 1;
		/* 8253 pit counter 2 mode 3 */
		cli();
		outb(0x43, 0xb6);
		outb(0x42, BEEPPT);
		outb(0x42, BEEPPT >> 8);
		sti();
		/* 8255 ppi: speaker on */
		outb(0x61, inb(0x61) | 3);
		timeout(beepoff, NULL, BEEPTM);
	}
	splx(s);
}

scmov(s, d, n)
short *s;
short *d;
{
	if (s < d)
		for (s += n, d += n; n--;)
			*--d = *--s;
	else
		while (n--)
			*d++ = *s++;
}

scset(p, c, n)
short *p;
{
	while (n--)
		*p++ = c;
}

/*
 * Kernel printf output ends up here.  It is buffered for
 * later retrieval by dmesg, and gets printed on the console
 * if we're not in cursor addressing mode.
 */
putchar(c)
{
	if (c && c != '\r') {
		*msgbufp = c;
		if (++msgbufp >= &msgbuf[MSGBUFS])
			msgbufp = msgbuf;
	}
	if (!curs)
		scputc(c);
}

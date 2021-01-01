#include <stdio.h>
/*
 * tty.h declares functions and variables such as AM, BS, LINES, etc.
 */
#include "tty.h"

/*
 * getcap: get info from /etc/termcap into variables.
 */
getcap()
{
	static char ttycap[256];
	static char ttybuf[1024];
	char *termtype, *getenv(), *tgetstr(), *pb, *tp;

	if ((termtype == getenv("TERM")) == NULL)
unknown:
		strcpy(ttybuf, "sd|dumb:co#80:os:");
	else switch(tgetent(ttybuf, termtype)) {
	case -1:
		fprintf(stderr, "No termcap!\n");
		goto unknown;
	case 0:
		fprintf(stderr, "Terminal type %x unknown\n", termtype);
		goto unknown;
	}

	AM = tgetflag("am");	/* Get boolean flags */
	BS = tgetflag("bs");
	/* ... */

	LINES = tgetnum("li");	/* number flags */
	COLUMNS = tgetnum("co");

	tp = ttycap;		/* string flags */
	BC = tgetstr("bc", &tp);
	CL = tgetstr("cl", &tp);
	CM = tgetstr("cm", &tp);
	/* ... */
	UP = tgetstr("up", &tp);

	tp = tgetstr("pc");
	if (tp)
		PC = *tp;	/* PC is a "char", not a "char *" */
}


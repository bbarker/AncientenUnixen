/*
 * Routines to put terminal into and out of screen mode.
 * All screen oriented work should be done in this mode.
 * It sets cbreak mode, initializes the terminal, etc.
 */
#include <sgtty.h>
short ospeed;
static struct sgttyb ttybuf, ottybuf;

/*
 * put terminal into screen mode.
 */
ttyinit()
{
	gtty(2, &ttybuf);
	UPPERCASE = (ttybuf.sg_flags & LCASE) != 0;
	HASTABS = (ttybuf.sg_flags & XTABS) != 0;
	NONL = (ttybuf.sg_flags & CRMOD) == 0;
	ottybuf = ttybuf;
	ttybuf.sg_flags &= ~(ECHO|CRMOD);
	ttybuf.sg_flags |= CBREAK;
	ioctl(2, TIOCSETN, &ttybuf);	/* or stty */

	putpad(TI);
	putpad(KS);	/* only needed if you use keypad */
}

/*
 * restore terminal to normal mode (upon exit, for shell escapes).
 */
ttyrestore()
{
	putpad(tgoto(CM, 0, LINES-1));	/* go to lower left corner */
	putpad(KE);
	putpad(TE);
	ttybuf = ottybuf;
	ioctl(2, TIOCSETN, &ttybuf);
}

/*
 * Putpad is called to output a string with simple padding.
 */
putch(c)
char c;
{
	putchar(c);
}

putpad(str)
char *str;
{
	if (str)
		tputs(str, 1, putch);
}

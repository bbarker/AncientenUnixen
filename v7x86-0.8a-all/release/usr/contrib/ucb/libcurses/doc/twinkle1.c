# include	<curses.h>
# include	<signal.h>

/*
 * the idea for this program was a product of the imagination of
 * Kurt Schoens.  Not responsible for minds lost or stolen.
 */

# define	NCOLS	80
# define	NLINES	24
# define	MAXPATTERNS	4

struct locs {
	char	y, x;
};

typedef struct locs	LOCS;

LOCS	Layout[NCOLS * NLINES];	/* current board layout */

int	Pattern,		/* current pattern number */
	Numstars;		/* number of stars in pattern */

main() {

	char		*getenv();
	int		die();

	srand(getpid());		/* initialize random sequence */

	initscr();
	signal(SIGINT, die);
	noecho();
	nonl();
	leaveok(stdscr, TRUE);
	scrollok(stdscr, FALSE);

	for (;;) {
		makeboard();		/* make the board setup */
		puton('*');		/* put on '*'s */
		puton(' ');		/* cover up with ' 's */
	}
}

/*
 * On program exit, move the cursor to the lower left corner by
 * direct addressing, since current location is not guaranteed.
 * We lie and say we used to be at the upper right corner to guarantee
 * absolute addressing.
 */
die() {

	signal(SIGINT, SIG_IGN);
	mvcur(0, COLS-1, LINES-1, 0);
	endwin();
	exit(0);
}


/*
 * Make the current board setup.  It picks a random pattern and
 * calls ison() to determine if the character is on that pattern
 * or not.
 */
makeboard() {

	reg int		y, x;
	reg LOCS	*lp;

	Pattern = rand() % MAXPATTERNS;
	lp = Layout;
	for (y = 0; y < NLINES; y++)
		for (x = 0; x < NCOLS; x++)
			if (ison(y, x)) {
				lp->y = y;
				lp++->x = x;
			}
	Numstars = lp - Layout;
}

/*
 * Return TRUE if (y, x) is on the current pattern.
 */
ison(y, x)
reg int	y, x; {

	switch (Pattern) {
	  case 0:	/* alternating lines */
		return !(y & 01);
	  case 1:	/* box */
		if (x >= LINES && y >= NCOLS)
			return FALSE;
		if (y < 3 || y >= NLINES - 3)
			return TRUE;
		return (x < 3 || x >= NCOLS - 3);
	  case 2:	/* holy pattern! */
		return ((x + y) & 01);
	  case 3:	/* bar across center */
		return (y >= 9 && y <= 15);
	}
	/* NOTREACHED */
}

puton(ch)
reg char	ch; {

	reg LOCS	*lp;
	reg int		r;
	reg LOCS	*end;
	LOCS		temp;

	end = &Layout[Numstars];
	for (lp = Layout; lp < end; lp++) {
		r = rand() % Numstars;
		temp = *lp;
		*lp = Layout[r];
		Layout[r] = temp;
	}

	for (lp = Layout; lp < end; lp++) {
		mvaddch(lp->y, lp->x, ch);
		refresh();
	}
}

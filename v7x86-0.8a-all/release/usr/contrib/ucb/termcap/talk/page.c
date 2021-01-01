#include <stdio.h>
#include <curses.h>
#include <signal.h>

main(argc, argv)
char **argv;
{
	FILE *fd;
	char linebuf[512];
	int line;
	int done();

	fd = fopen(argv[1], "r");	/* should do lots of checking */
	signal(SIGINT, done);		/* die gracefully */

	initscr();			/* initialize curses */
	noecho();			/* turn off tty echo */
	crmode();			/* enter cbreak mode */

	for (;;) {			/* for each screen full */
		move(0, 0);
		for (line=0; line<LINES-1; line++) {
			if (fgets(linebuf, sizeof linebuf, fd) == NULL) {
				done();
			}
			mvprintw(line, 0, "%s", linebuf);
		}
		mvprintw(LINES-1, 0, "--More--");
		refresh();		/* sync screen */
		if(getch() == 'q')	/* wait for user to read it */
			done();
	}
}

/*
 * Clean up and exit.
 */
done()
{
	move(LINES-1,0);		/* to lower left corner */
	clrtoeol();			/* clear bottom line */
	refresh();			/* flush out everything */
	endwin();			/* curses cleanup */
	exit(0);
}

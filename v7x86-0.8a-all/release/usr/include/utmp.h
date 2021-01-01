/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

struct utmp {
	char	ut_line[8];		/* tty name */
	char	ut_name[8];		/* user id */
	long	ut_time;		/* time on */
};

static char *sccsid = "@(#)whereis.c	4.1 (Berkeley) 10/1/80";
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/dir.h>
#include <whoami.h>

static char *bindirs[] = {
	"/etc",
	"/bin",
	"/usr/bin",
	"/usr/games",
#ifdef CSVAX
	"/lib",
	"/usr/ucb",
	"/usr/lib",
	"/usr/local",
	"/usr/new",
	"/usr/old",
#endif
#ifdef CORY
	"/usr/bin/eecs",
	"/usr/bin/new",
	"/usr/bin/v7",
	"/usr/bin/old",
	"/usr/bin/UNSUPPORTED",
#endif
	0
};
static char *mandirs[] = {
	"/usr/man/man1",
	"/usr/man/man2",
	"/usr/man/man3",
	"/usr/man/man4",
	"/usr/man/man5",
	"/usr/man/man6",
	"/usr/man/man7",
	"/usr/man/man8",
#ifdef CORY
	"/usr/man/manu",
	"/usr/man/manc",
	"/usr/man/manv7",
	"/usr/bin/eecs/mane",
#endif
	0
};
static char *srcdirs[]  = {
	"/usr/src/cmd",
	"/usr/src/games",
	"/usr/src/libc/gen",
	"/usr/src/libc/stdio",
#ifdef CSVAX
	"/usr/src/libc/sys",
	"/usr/src/new",
	"/usr/src/old",
	"/usr/src/local",
	"/usr/src/undoc",
#endif
#ifdef CORY
	"/usr/bin/eecs/src",
	"/usr/src/cmd/v7",
	"/usr/src/cmd/new",
	"/usr/src/cmd/old",
	"/usr/src/cmd/UNSUPPORTED",
#endif
	0
};

char	sflag = 1;
char	bflag = 1;
char	mflag = 1;
char	**Sflag;
int	Scnt;
char	**Bflag;
int	Bcnt;
char	**Mflag;
int	Mcnt;
char	uflag;
/*
 * whereis name
 * look for source, documentation and binaries
 */
main(argc, argv)
	int argc;
	char *argv[];
{

#ifdef CORY
	if (getuid() == 0)
		nice(-20);
	if (((getuid() >> 8) & 0377) > 10)
		setuid(getuid());
#endif
	argc--, argv++;
	if (argc == 0) {
usage:
		fprintf(stderr, "whereis [ -sbmu ] [ -SBM dir ... -f ] name...\n");
		exit(1);
	}
	do
		if (argv[0][0] == '-') {
			register char *cp = argv[0] + 1;
			while (*cp) switch (*cp++) {

			case 'f':
				break;

			case 'S':
				getlist(&argc, &argv, &Sflag, &Scnt);
				break;

			case 'B':
				getlist(&argc, &argv, &Bflag, &Bcnt);
				break;

			case 'M':
				getlist(&argc, &argv, &Mflag, &Mcnt);
				break;

			case 's':
				zerof();
				sflag++;
				continue;

			case 'u':
				uflag++;
				continue;

			case 'b':
				zerof();
				bflag++;
				continue;

			case 'm':
				zerof();
				mflag++;
				continue;

			default:
				goto usage;
			}
			argv++;
		} else
			lookup(*argv++);
	while (--argc > 0);
}

getlist(argcp, argvp, flagp, cntp)
	char ***argvp;
	int *argcp;
	char ***flagp;
	int *cntp;
{

	(*argvp)++;
	*flagp = *argvp;
	*cntp = 0;
	for ((*argcp)--; *argcp > 0 && (*argvp)[0][0] != '-'; (*argcp)--)
		(*cntp)++, (*argvp)++;
	(*argcp)++;
	(*argvp)--;
}


zerof()
{

	if (sflag && bflag && mflag)
		sflag = bflag = mflag = 0;
}
int	count;
int	print;


lookup(cp)
	register char *cp;
{
	register char *dp;

	for (dp = cp; *dp; dp++)
		continue;
	for (; dp > cp; dp--) {
		if (*dp == '.') {
			*dp = 0;
			break;
		}
	}
	for (dp = cp; *dp; dp++)
		if (*dp == '/')
			cp = dp + 1;
	if (uflag) {
		print = 0;
		count = 0;
	} else
		print = 1;
again:
	if (print)
		printf("%s:", cp);
	if (sflag) {
		looksrc(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (bflag) {
		lookbin(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (mflag) {
		lookman(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	if (print)
		printf("\n");
}

looksrc(cp)
	char *cp;
{
	if (Sflag == 0) {
		find(srcdirs, cp);
	} else
		findv(Sflag, Scnt, cp);
}

lookbin(cp)
	char *cp;
{
	if (Bflag == 0)
		find(bindirs, cp);
	else
		findv(Bflag, Bcnt, cp);
}

lookman(cp)
	char *cp;
{
	if (Mflag == 0) {
		find(mandirs, cp);
	} else
		findv(Mflag, Mcnt, cp);
}

findv(dirv, dirc, cp)
	char **dirv;
	int dirc;
	char *cp;
{

	while (dirc > 0)
		findin(*dirv++, cp), dirc--;
}

find(dirs, cp)
	char **dirs;
	char *cp;
{

	while (*dirs)
		findin(*dirs++, cp);
}

findin(dir, cp)
	char *dir, *cp;
{
	register FILE *d;
	struct direct direct;

	d = fopen(dir, "r");
	if (d == NULL)
		return;
	while (fread(&direct, sizeof direct, 1, d) == 1) {
		if (direct.d_ino == 0)
			continue;
		if (itsit(cp, direct.d_name)) {
			count++;
			if (print)
				printf(" %s/%.14s", dir, direct.d_name);
		}
	}
	fclose(d);
}

itsit(cp, dp)
	register char *cp, *dp;
{
	register int i = 14;

	if (dp[0] == 's' && dp[1] == '.' && itsit(cp, dp+2))
		return (1);
	while (*cp && *dp && *cp == *dp)
		cp++, dp++, i--;
	if (*cp == 0 && *dp == 0)
		return (1);
	while (isdigit(*dp))
		dp++;
	if (*cp == 0 && *dp++ == '.') {
		--i;
		while (i > 0 && *dp)
			if (--i, *dp++ == '.')
				return (*dp++ == 'C' && *dp++ == 0);
		return (1);
	}
	return (0);
}

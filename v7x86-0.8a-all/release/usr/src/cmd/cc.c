/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 2006 Robert Nordier. All rights reserved. */

#
# include <stdio.h>
# include <signal.h>

/* cc command */

# define MAXFIL 100
# define MAXLIB 100
# define MAXOPT 100
char	*tmp0;
char	*tmp1;
char	*tmp2;
char	*tmp3;
char	*outfile;
# define CHSPACE 1000
char	ts[CHSPACE+50];
char	*tsa = ts;
char	*tsp = ts;
char	*av[50];
char	*clist[MAXFIL];
char	*llist[MAXLIB];
int	pflag;
int	sflag;
int	cflag;
int	eflag;
int	oflag;
int	vflag;
int	proflag;
char	pass0[] = "/lib/cc/em_cemcom";
char	pass1[] = "/lib/cc/em_opt";
char	pass2[] = "/lib/cc/i386cg";
char	*pref = "/lib/crt0.o";
char	*copy();
char	*setsuf();

main(argc, argv)
char *argv[]; 
{
	char *t;
	char *savetsp;
	char *assource;
	char **pv, *ptemp[MAXOPT], **pvt;
	int nc, nl, i, j, c, nxo, na;
	int idexit();

	i = nc = nl = nxo = 0;
	setbuf(stdout, (char *)NULL);
	pv = ptemp;
	while(++i < argc) {
		if(*argv[i] == '-') switch (argv[i][1]) {
		default:
			goto passa;
		case 'S':
			sflag++;
			cflag++;
			break;
		case 'o':
			if (++i < argc) {
				outfile = argv[i];
				if ((c=getsuf(outfile))=='c'||c=='o') {
					error("Would overwrite %s", outfile);
					exit(8);
				}
			}
			break;
		case 'O':
			oflag++;
			break;
		case 'p':
			proflag++;
			break;
		case 'P':
			pflag++;
		case 'c':
			cflag++;
			break;
		case 'v':
			vflag++;
			break;

		case 'D':
		case 'I':
		case 'U':
		case 'C':
			*pv++ = argv[i];
			if (pv >= ptemp+MAXOPT) {
				error("Too many DIUC options", (char *)NULL);
				--pv;
			}
			break;
		} 
		else {
passa:
			t = argv[i];
			if((c=getsuf(t))=='c' || c=='s') {
				clist[nc++] = t;
				if (nc>=MAXFIL) {
					error("Too many source files", (char *)NULL);
					exit(1);
				}
				t = setsuf(t, 'o');
			}
			if (nodup(llist, t)) {
				llist[nl++] = t;
				if (nl >= MAXLIB) {
					error("Too many object/library files", (char *)NULL);
					exit(1);
				}
				if (getsuf(t)=='o')
					nxo++;
			}
		}
	}
	if (proflag)
		pref = "/lib/mcrt0.o";
	if(nc==0)
		goto nocom;
	if (pflag==0) {
		tmp0 = copy("/tmp/ctm0a");
		while (access(tmp0, 0)==0)
			tmp0[9]++;
		while((creat(tmp0, 0400))<0) {
			if (tmp0[9]=='z') {
				error("cc: cannot create temp", NULL);
				exit(1);
			}
			tmp0[9]++;
		}
	}
	else
		tmp0 = "";
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, idexit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, idexit);
	(tmp1 = copy(tmp0))[8] = '1';
	(tmp2 = copy(tmp0))[8] = '2';
	(tmp3 = copy(tmp0))[8] = '3';
	pvt = pv;
	for (i=0; i<nc; i++) {
		if (nc>1)
			printf("%s:\n", clist[i]);
		if (getsuf(clist[i])=='s') {
			assource = clist[i];
			goto assemble;
		} 
		else
			assource = tmp3;
		if (pflag)
			tmp1 = setsuf(clist[i], 'i');
		savetsp = tsp;
		av[0] = "em_cemcom";
		av[1] = "-Dunix";
		av[2] = "-Di386";
		av[3] = "-D__ACK";
		av[4] = "-D__V7x86__";
		av[5] = "-Vw4.4i4.4p4.4f4.4s2.2l4.4d8.4";
		av[6] = "-Vr";
		av[7] = "-L";
		na = 8;
		for(pv=ptemp; pv <pvt; pv++)
			av[na++] = *pv;
		av[na++] = "-I/usr/include";
		av[na++] = clist[i];
		av[na++] = tmp1;
		av[na] = 0;
		if (callsys(pass0, av)) {
			cflag++;
			eflag++;
			continue;
		}
		tsp = savetsp;
		if (pflag) {
			cflag++;
			continue;
		}
		av[0] = "em_opt";
		av[1] = "-m10";
		av[2] = tmp1;
		av[3] = tmp2;
		av[4] = 0;
		if(callsys(pass1, av)) {
			cflag++;
			eflag++;
			continue;
		}
		if (sflag)
			assource = tmp3 = setsuf(clist[i], 's');
		av[0] = "i386cg";
		av[1] = "-Ffltused";
		na = 2;
		if (proflag)
			av[na++] = "-P";
		av[na++] = tmp2;
		av[na++] = tmp3;
		av[na] = 0;
		if (callsys(pass2, av)) {
			cflag++;
			eflag++;
			continue;
		} 
		if (sflag)
			continue;
assemble:
		av[0] = "as";
		av[1] = "-A";
		av[2] = "-o";
		av[3] = setsuf(clist[i], 'o');
		av[4] = assource;
		av[5] = 0;
		cunlink(tmp1);
		cunlink(tmp2);
		if (callsys("/bin/as", av) > 1) {
			cflag++;
			eflag++;
			continue;
		}
	}
nocom:
	if (cflag==0 && nl!=0) {
		i = 0;
		av[0] = "ld";
		av[1] = pref;
		j = 2;
		if (outfile) {
			av[j++] = "-o";
			av[j++] = outfile;
		}
		while(i<nl)
			av[j++] = llist[i++];
		av[j++] = "-lc";
		av[j++] = "-lem";
		av[j++] = 0;
		eflag |= callsys("/bin/ld", av);
		if (nc==1 && nxo==1 && eflag==0)
			cunlink(setsuf(clist[0], 'o'));
	}
	dexit();
}

idexit()
{
	eflag = 100;
	dexit();
}

dexit()
{
	if (!pflag) {
		cunlink(tmp1);
		cunlink(tmp2);
		if (sflag==0)
			cunlink(tmp3);
		cunlink(tmp0);
	}
	exit(eflag);
}

error(s, x)
char *s, *x;
{
	fprintf(stdout, s, x);
	putc('\n', stdout);
	cflag++;
	eflag++;
}




getsuf(as)
char as[];
{
	register int c;
	register char *s;
	register int t;

	s = as;
	c = 0;
	while(t = *s++)
		if (t=='/')
			c = 0;
		else
			c++;
	s -= 3;
	if (c<=14 && c>2 && *s++=='.')
		return(*s);
	return(0);
}

char *
setsuf(as, ch)
char *as;
{
	register char *s, *s1;

	s = s1 = copy(as);
	while(*s)
		if (*s++ == '/')
			s1 = s;
	s[-1] = ch;
	return(s1);
}

callsys(f, v)
char f[], *v[]; 
{
	int i, t, status;

	if (vflag) {
		printf("%s", f);
		for (i = 1; v[i]; i++)
			printf(" %s", v[i]);
		putchar('\n');
	}
	if ((t=fork())==0) {
		execv(f, v);
		printf("Can't find %s\n", f);
		exit(100);
	} else
		if (t == -1) {
			printf("Try again\n");
			return(100);
		}
	while(t!=wait(&status))
		;
	if (t = status&0377) {
		if (t!=SIGINT) {
			printf("Fatal error in %s\n", f);
			eflag = 8;
		}
		dexit();
	}
	return((status>>8) & 0377);
}

char *
copy(as)
char *as;
{
	char *malloc();
	register char *otsp, *s;

	otsp = tsp;
	s = as;
	while (*tsp++ = *s++)
		;
	if (tsp > tsa+CHSPACE) {
		tsp = tsa = malloc(CHSPACE+50);
		if (tsp==NULL) {
			error("no space for file names", (char *)NULL);
			dexit();
		}
	}
	return(otsp);
}

nodup(l, os)
char **l, *os;
{
	register char *t, *s;
	register int c;

	s = os;
	if (getsuf(s) != 'o')
		return(1);
	while(t = *l++) {
		while(c = *s++)
			if (c != *t++)
				break;
		if (*t=='\0' && c=='\0')
			return(0);
		s = os;
	}
	return(1);
}

cunlink(f)
char *f;
{
	if (f==NULL)
		return;
	unlink(f);
}

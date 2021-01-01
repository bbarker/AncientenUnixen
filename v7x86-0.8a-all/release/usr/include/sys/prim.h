/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#define	NOSLEEP	0400
#define	FORCE	01000
#define	NORM	02000
#define	KEEP	04000
#define	CLR	010000

int	bwaiting,wcount;

char *getepack();

/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 2007 Robert Nordier. All rights reserved. */

#
/*
 *	UNIX/i386 debugger
 */

/* unix parameters */
#define DBNAME "adb\n"
#define LPRMODE "%R"
#define OFFMODE "+%R"
#define TXTRNDSIZ 4096L

TYPE	unsigned TXTHDR[8];
TYPE	unsigned SYMV;

/* symbol table in a.out file */
struct symtab {
	char	symc[8];
	char	symf;
	char	sympad[3];
	SYMV	symv;
};
#define SYMTABSIZ (sizeof (struct symtab))

#define SYMCHK 057
#define SYMTYPE(symflg) ((symflg)&1 && (symflg)!=037 ? DSYM : NSYM)

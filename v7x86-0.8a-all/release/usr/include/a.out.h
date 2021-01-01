/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

struct	exec {	/* a.out header */
	int     	a_magic;	/* magic number */
	unsigned	a_text; 	/* size of text segment */
	unsigned	a_data; 	/* size of initialized data */
	unsigned	a_bss;  	/* size of uninitialized data */
	unsigned	a_syms; 	/* size of symbol table */
	unsigned	a_entry; 	/* entry point */
	unsigned	a_trsize;	/* size of text relocation */
	unsigned	a_drsize; 	/* size of data relocation */
};

#define	A_MAGIC1	0407       	/* normal */
#define	A_MAGIC2	0410       	/* read-only text */
#define	A_MAGIC3	0413       	/* demand load format */
#define	A_MAGIC4	0405       	/* overlay */

struct	nlist {	/* symbol table entry */
	char    	n_name[8];	/* symbol name */
	char     	n_type;    	/* type flag */
	char		n_other;
	short		n_desc;
	unsigned	n_value;	/* value */
};

		/* values for type flag */
#define	N_UNDF	0	/* undefined */
#define	N_ABS	02	/* absolute */
#define	N_TEXT	04	/* text symbol */
#define	N_DATA	06	/* data symbol */
#define	N_BSS	010	/* bss symbol */
#define	N_TYPE	036
#define	N_FN	037	/* file name symbol */
#define	N_EXT	01	/* external bit, or'ed in */
#define	FORMAT	"%08x"	/* to print a value */

/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2007 Robert Nordier.  All rights reserved. */

#define FX	0x8000
#define FC	0x4000
#define FJ	0x2000

struct tab {
	unsigned char mask, val;
	short num;
	char cmd[4];
};

struct tabs {
	unsigned val;
	short sz;
	struct tab *tab;
};

extern struct tabs tabs[], tabs0f[];
extern int tabsz, tabs0fz;
extern char *str[];

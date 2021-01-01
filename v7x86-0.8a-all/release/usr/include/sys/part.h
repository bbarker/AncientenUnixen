/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2006 Robert Nordier.  All rights reserved. */

#define PTMAGIC		0xaa55	/* last two byte of mbr */
#define PTOFF		0x1be	/* partition table offset in mbr */
#define PTBOOT		0x80	/* bootable flag */
#define PTID		0x72	/* V7/x86 partition id */
#define NPTE		4	/* number of partition table entries */

struct hsc {
	unsigned char h, s, c;	/* chs in jumbled form */
};

/* partition table entry */
struct ptent {
	unsigned char boot;	/* boot indicator (active) */
	struct hsc s;		/* starting chs */
	unsigned char type;	/* system type */
	struct hsc e;		/* ending chs */
	unsigned base;		/* offset (lba) */
	unsigned size;		/* size in sectors */
};

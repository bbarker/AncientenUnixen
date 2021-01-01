/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2007 Robert Nordier.  All rights reserved. */

/* Minimal ISO 9660 CD-ROM utility for V7/x86 */

#include <stdio.h>

#define SECSIZ 2048
#define NAMLEN 255

#define sw(x,y) ((x)<<8|(y))
#define cv4(x) (*(unsigned *)(x))

/* ISO 9660 Primary Volume Descriptor */
static char cdmagic[] = {1, 'C', 'D', '0', '0', '1', 1, 0};

struct cddir {
	unsigned char len_dr;	/* length of directory record */
	unsigned char len_ear;	/* extended attribute record length */
	unsigned char ext[8];	/* location of extent */
	unsigned char size[8];	/* data length */
	unsigned char time[7];	/* recording date and time */
	unsigned char flags;	/* file flags */
	unsigned char fus;	/* file unit size */
	unsigned char gap;	/* interleave gap size */
	unsigned char vsn[4];	/* volume sequence number */
	unsigned char len_fi;	/* length of file identifier */
	unsigned char fi[1];	/* file identifier ... */
};

struct dir {
	unsigned ext;		/* starting block number */
	unsigned size;		/* file size */
	int type;		/* file type ('d' or '-') */
	char name[NAMLEN + 1];	/* file name */
};

static int nflag;
static char *fn;		/* special file name */
static int fd;			/* special file descriptor */

int
main(argc, argv)
char **argv;
{
	char *path;
	int e;

	if (argv[1][0] == '-' && argv[1][1] == 'n' && argv[1][2] == 0) {
		nflag = 1;
		argv++;
		argc--;
	}
	if (argc != 2 && argc != 3) {
		fprintf(stderr, "usage: cdcat special [path]\n");
		exit(2);
	}
	fn = argv[1];
	if ((fd = open(argv[1], 0)) == -1)
		error("cannot open");
	path = argv[2] ? argv[2] : "";
	if ((e = cdcat(path)) != 0)
		fprintf(stderr, "cdcat: %s: Not found\n", path);
	return e;
}

int
cdcat(path)
char *path;
{
	unsigned char buf[SECSIZ];
	char name[NAMLEN + 1];
	struct cddir *dp, *tp;
	struct dir xd;
	char *p, *q;
	unsigned ext, size, bx, bn, x, i;
	int type, n;

	/* 
	 * find primary volume descriptor
	 * and thence root directory
	 */
	bx = 64;
	for (bn = 16; bn < bx; bn++) {
		readblk(buf, bn);
		if (strcmp(buf, cdmagic) == 0)
			break;
	}
	if (bn == bx)
		error("Invalid argument");
	dp = (struct cddir *)&buf[156];
	loaddir(dp, &xd);

	/*
	 * lookup, list, print ...
	 */
	for (p = path; dp; p = q) {
		while (*p == '/')
			p++;
		for (q = p; *q && *q != '/'; q++);
		if ((n = q - p)) {
			if (n > NAMLEN)
				n = NAMLEN;
			memcpy(name, p, n);
			name[n] = 0;
		}
		ext = xd.ext;
		size = xd.size;
		type = xd.type;
		dp = NULL;
		bx = ext + (size + (SECSIZ - 1)) / SECSIZ;
		for (bn = ext; !dp && bn < bx; bn++) {
			readblk(buf, bn);
			if (type == 'd')
				for (i = 0; !dp && buf[i]; i += buf[i]) {
					tp = (struct cddir *)(buf + i);
					loaddir(tp, &xd);
					if (n == 0)
						printf("%10u %c %s\n",
						    xd.size, xd.type, xd.name);
					else if (strcmp(name, xd.name) == 0)
						dp = tp;
				}
			else if (!nflag) {
				x = size < SECSIZ ? size : SECSIZ;
				for (i = 0; i < x; i++)
					putchar(buf[i]);
				size -= x;
			}
		}
	}
	return n != 0;
}

/*
 * Gather together the directory information that interests us.
 * Any and all of this may be altered by a suitable SUSP field.
 */
loaddir(dp, xp)
struct cddir *dp;
struct dir *xp;
{
	int c;

	xp->ext = cv4(dp->ext);
	xp->size = cv4(dp->size);
	xp->type = dp->flags & 2 ? 'd' : '-';
	xp->name[0] = 0;
	if (dp->fi[0] != 0) {
		c = dp->len_fi | 1;
		susp(dp->fi + c, dp->len_dr - 33 - c, xp);
	}
	if (xp->name[0] == 0)
		if (dp->fi[0] == 0 || dp->fi[0] == 1)
			strcpy(xp->name, dp->fi[0] == 0 ? "." : "..");
		else {
			memcpy(xp->name, dp->fi, dp->len_fi);
			xp->name[dp->len_fi] = 0;
		}
}

/*
 * SUSP/RRIP support: allowing UNIX-style file names and directories
 * nested more than eight deep (among other things).
 */
susp(sp, n, xp)
unsigned char *sp;
struct dir *xp;
{
	unsigned char buf[SECSIZ];
	unsigned char *p;
	int i, j;

	for (p = sp; p < sp + n && *p;) {
		if (p[3] != 1)
			return;
		switch (sw(p[0], p[1])) {
		/* continuation area */
		case sw('C', 'E'):
			readblk(buf, cv4(&p[4]));
			sp = buf + cv4(&p[12]);
			n = cv4(&p[20]);
			p = sp;
			continue;
		/* child link */
		case sw('C', 'L'):
			xp->ext = cv4(&p[4]);
			xp->size = SECSIZ;
			xp->type = 'd';
			break;
		/* alternate name */
		case sw('N', 'M'):
			for (j = 0; xp->name[j]; j++);
			for (i = 5; i < p[2]; i++)
				xp->name[j++] = p[i];
			xp->name[j] = 0;
			break;
		}
		p += p[2];
	}
}

readblk(buf, blkno)
char *buf;
unsigned blkno;
{
	lseek(fd, blkno * SECSIZ, 0);
	if (read(fd, buf, SECSIZ) != SECSIZ)
		error("read error");
}

memcpy(dst, src, n)
char *dst;
char *src;
{
	while (n--)
		*dst++ = *src++;
}

error(msg)
char *msg;
{
	fprintf(stderr, "cdcat: %s: %s\n", fn, msg);
	exit(2);
}

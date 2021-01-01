/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2006 Robert Nordier.  All rights reserved. */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/part.h>
#include <ctype.h>
#include <stdio.h>

extern double atof();

#define BSIZE   512
#define BSHIFT  9

struct chs {
    int c, h, s;                /* cylinders, heads, sectors */
};

static int fflag;               /* force */
static int zflag;               /* display sizes in Ks, Ms, or Gs */
static struct chs geom;         /* user-supplied disk geometry */
static unsigned dsize;          /* disk size from geometry */
static char *barg = "/boot/mbr"; /* boot code */

char *fmtsize(), *descr();

int
main(argc, argv)
char **argv;
{
    char *p, *q;
    int e, i, c;

    e = 0;
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        p = &argv[i][1];
        if (p[0] == 0)
            break;
        if (p[0] == '-' && p[1] == 0) {
            i++;
            break;
        }
        do
            switch (c = *p++) {
            case 'f':
                fflag = 1;
                break;
            case 'z':
                zflag = 1;
                break;
            case 'b':
            case 'g':
                if (*p)
                    q = p;
                else if (++i < argc)
                    q = argv[i];
                else {
                    error(0, "option %c needs an argument", c);
                    q = NULL;
                    e = 1;
                    break;
                }
                switch (c) {
                case 'b':
                    barg = q;
                    break;
                case 'g':
                    if (getgeom(q, &geom)) {
                        error(0, "bad geometry");
                        e = 1;
                    } else
                        dsize = geom.c * geom.h * geom.s;
                    break;
                }
                p = "";
                break;
            default:
                error(0, "unknown option %c", c);
                e = 1;
            }
        while (*p);
    }
    if (e || argc - i < 1)
        usage();
    ptdisk(argv + i);
    return 0;
}

ptdisk(av)
char **av;
{
    char buf[BSIZE];
    struct stat sb;
    struct ptent *pt;
    char *dname, *s;
    int cmd, slot, type, size;
    int ro, fd, ok, x, i, j;

    dname = *av++;
    cmd = 'p';
    slot = 0;
    type = 0x72;
    size = 0;
    if (*av) {
        cmd = *av[0];
        if ((*av++)[1] || !isalpha(cmd))
            error(2, "command expected");
    }
    if (index("adt", cmd)) {
        if (*av == NULL)
            error(2, "partition not specified");
        slot = atoi(*av++);
        if (slot < 1 || slot > 4)
            error(2, "bad slot");
    }
    if (index("ct", cmd)) {
        if (*av == NULL)
            error(2, "type not specified");
        type = atoi(*av++);
        if (type < 1 || type > 255)
            error(2, "bad type");
    }
    if (cmd == 'c' && *av) {
        s = *av++;
        size = getsize(s);
        if (size == 0)
            error(2, "bad size");
    }
    if (*av)
        error(2, "too many arguments");
    ro = index("fpx", cmd) != NULL;
    fd = open(dname, ro ? 0 : 2);
    if (fd == -1)
        error(1, "%s: cannot open", dname);
    if (read(fd, buf, BSIZE) != BSIZE)
        error(1, "%s: read error", dname);
    ok = *(unsigned short *)(buf + BSIZE - 2) == PTMAGIC;
    if (!ok && cmd != 'i')
        error(1, "%s: no partition table", dname);
    pt = (struct ptent *)(buf + PTOFF);
    if (index("cfx", cmd)) {
        if (dsize == 0)
            error(2, "Geometry not specified");
        cktab(pt);
    }
    if (index("adt", cmd) && pt[slot - 1].type == 0)
        error(1, "Slot %d is empty", slot);
    switch (cmd) {
    /* flag partition as active */
    case 'a':
        for (i = 0; i < NPTE; i++)
            pt[i].boot = i == slot - 1 ? PTBOOT : 0;
        break;
    /* create partition */
    case 'c':
        {
            unsigned bs, sz, ex;

            sz = size;
            avail(pt, &bs, &sz);
            if (sz == 0)
                error(1, "No space");
            if (size) {
                if (sz < size)
                    error(1, "Not enough space");
                sz = size;
                ex = (bs + sz) % (geom.h * geom.s);
                if (ex) {
                    sz -= ex < sz ? ex : sz;
                    if (sz == 0)
                        error(1, "Size too small");
                }
            }
            for (i = 0; i < NPTE; i++)
                if (pt[i].type == 0)
                    break;
            if (i == NPTE)
                error(1, "No free slots");
            if (!confirm("Create partition"))
                return;
            pt[i].boot = 0;
            pt[i].type = type;
            pt[i].base = bs;
            pt[i].size = sz;
            mkhsc(pt[i].base, &pt[i].s);
            mkhsc(pt[i].base + pt[i].size - 1, &pt[i].e);
        }
        break;
    /* delete partition */
    case 'd':
        if (!confirm("Delete partition %d", slot))
            return;
        zero(&pt[slot - 1], sizeof(struct ptent));
        break;
    /* display free space */
    case 'f':
        avail(pt, NULL, NULL);
        break;
    /* initialize partition table */
    case 'i':
        if (ok)
            printf("Warning: a partition table already exists\n");
        if (!confirm("Initialize disk"))
            return;
        x = open(barg, 0);
        if (x == -1 || fstat(x, &sb) || sb.st_size != BSIZE ||
            read(x, buf, BSIZE) != BSIZE)
            error(1, "%s: cannot access", barg);
        close(x);
        ok = *(unsigned short *)(buf + BSIZE - 2) == PTMAGIC;
        if (!ok)
            error(1, "%s: invalid boot code", barg);
        zero(&pt, NPTE * sizeof(struct ptent));
        break;
    /* print out partition table */
    case 'p':
        printf("PART  TYPE     OFFSET       SIZE  DESCRIPTION\n");
        for (i = 0; i < NPTE; i++) {
            if (pt[i].type == 0 || pt[i].size == 0)
                continue;
            printf("  %d%c   %3d %10u %10s  %s\n",
                   1 + i, pt[i].boot ? '*' : ' ',
                   pt[i].type, pt[i].base, fmtsize(pt[i].size),
                   descr(pt[i].type));
        }
        break;
    /* change partition type */
    case 't':
        if (!confirm("Set partition %d to type %d", slot, type))
            return;
        pt[slot - 1].type = type;
        break;
    /* check partition table */
    case 'x':
        break;
    default:
        error(2, "Unrecognized command");
    };
    if (!ro) {
        lseek(fd, 0, 0);
        if (write(fd, buf, BSIZE) != BSIZE)
            error(1, "%s: write error", dname);
    }
}

/* Determine free space. */
avail(pt, base, size)
struct ptent *pt;
unsigned *base;
unsigned *size;
{
    unsigned maxsz, reqsz, bs, sz, x;
    int n, i, j;

    maxsz = 0;
    n = 0;
    if (base == NULL)
        printf("FREE     OFFSET       SIZE\n");
    else
        reqsz = *size;
    bs = geom.s;
    do {
        j = -1;
        for (i = 0; i < NPTE; i++) {
            if (pt[i].type == 0 || pt[i].size == 0)
                continue;
            if (pt[i].base >= bs &&
                (j == -1 || pt[j].base > pt[i].base))
                j = i;
        }
        if ((x = bs % geom.s) != 0)
            bs += geom.s - x;
        sz = j == -1 ? dsize : pt[j].base;
        sz -= sz < bs ? sz : bs;
        if (sz)
            if (base == NULL)
                printf("  %d  %10u %10s\n", ++n, bs, fmtsize(sz));
            else if (maxsz < sz) {
                *base = bs;
                *size = sz;
                if (reqsz && reqsz <= sz)
                    break;
                maxsz = sz;
            }
        if (j != -1)
            bs = pt[j].base + pt[j].size;
    } while (j != -1);
}

/* Check partition table. */
cktab(pt)
struct ptent *pt;
{
    int i, j;

    for (i = 0; i < NPTE; i++) {
        if (pt[i].type == 0 || pt[i].size == 0)
            continue;
        if (pt[i].base % geom.s)
            error(0, "Partition %d not aligned", 1 + i);
        if (pt[i].base >= dsize)
            error(0, "Partition %d beyond disk", 1 + i);
        if (pt[i].base + pt[i].size > dsize)
            error(0, "Partition %d overflows disk", 1 + i);
        if (ckchs(pt[i].base, &pt[i].s))
            error(0, "Partition %d bad starting chs", 1 + i);
        if (ckchs(pt[i].base + pt[i].size - 1, &pt[i].e))
            error(0, "Partition %d bad ending chs", 1 + i);
    }
    for (i = 0; i < NPTE - 1; i++) {
        if (pt[i].type == 0 || pt[i].size == 0)
            continue;
        for (j = i + 1; j < NPTE; j++) {
            if (pt[j].type == 0 || pt[j].size == 0)
                continue;
            if ((pt[j].base < pt[i].base &&
                 pt[j].base + pt[j].size > pt[i].base) ||
                (pt[j].base >= pt[i].base &&
                 pt[j].base < pt[i].base + pt[i].size))
                error(1, "partitions %d and %d overlap", 1 + i, 1 + j);

        }
    }
}

/* Check hsc values again lba. */
int
ckchs(lba, hsc)
unsigned lba;
struct hsc *hsc;
{
    struct chs chs;
    unsigned x;

    if (lba >= 1024 * geom.h * geom.s)
        return 0;
    if (hsc->h == 0xff && hsc->s == 0xff && hsc->c == 0xff)
        return 0;
    mkchs(hsc, &chs);
    x = ((chs.c * geom.h) + chs.h) * geom.s + (chs.s - 1);
    if (chs.s == 0 || x != lba)
        return -1;
    return 0;
}

/* Generate hsc values from lba. */
mkhsc(lba, hsc)
unsigned lba;
struct hsc *hsc;
{
    int c, h, s;

    if (lba >= 1024 * geom.h * geom.s) {
        hsc->c = hsc->h = hsc->s = 0xff;
        return;
    }
    s = lba % geom.s + 1;
    h = lba / geom.s % geom.h;
    c = lba / geom.s / geom.h;
    hsc->c = c;
    hsc->h = h;
    hsc->s = (c & ~0xff) >> 2 | s;
}

/* Unpack jumbled hsc to chs. */
mkchs(hsc, chs)
struct hsc *hsc;
struct chs *chs;
{
    chs->c = (hsc->s & 0xc0) << 2 | hsc->c;
    chs->h = hsc->h;
    chs->s = hsc->s & 0x3f;
}

/* Parse a geometry string. */
int
getgeom(s, g)
char *s;
struct chs *g;
{
    int x, i;

    for (i = 0; i < 3; i++) {
        x = 0;
        while (isdigit(*s))
            x = x * 10 + (*s++ - '0');
        if (x <= 0)
            return -1;
        switch (i) {
        case 0:
            g->c = x;
            break;
        case 1:
            g->h = x;
            break;
        case 2:
            g->s = x;
        }
        if (*s++ != (i == 2 ? 0 : ','))
            return -1;
    }
    if (g->s > 63 || g->h > 256)
        return -1;
    return 0;
}

/* Get size for new partition. */
int
getsize(s)
char *s;
{
    char *p;
    double f;
    int sz, x;

    for (p = s; isdigit(*p) || *p == '.'; p++);
    switch (lower(*p)) {
    case 0:
        x = 0;
        break;
    case 'k':
        x = 1 << (10 - BSHIFT);
        break;
    case 'm':
        x = 1 << (20 - BSHIFT);
        break;
    case 'g':
        x = 1 << (30 - BSHIFT);
        break;
    default:
        error(2, "bad size suffix");
    }
    if (!x)
        sz = atoi(s);
    else
        sz = atof(s) * x + 0.5;
    return sz;
}

/* Format size using Ks, Ms, or Gs. */
char *
fmtsize(sz)
unsigned sz;
{
    static char buf[16];
    static struct {
        unsigned cmp;
        unsigned div;
        int sym;
    } tab[] = {
        {8192*2, 2, 'K'},
        {8192*2*1024, 2*1024, 'M'},
        {0, 2*1024*1024, 'G'}
    };
    int i;

    if (!zflag)
        sprintf(buf, "%u", sz);
    else {
        for (i = 0; i < 2 && sz > tab[i].cmp; i++);
        sprintf(buf, "%3.1f%c", sz / (double)tab[i].div, tab[i].sym);
    }
    return buf;
}

/* Return partition description. */
char *
descr(type)
{
    switch (type) {
    case 1: case 4: case 5: case 6: case 7:
    case 11: case 12: case 14: case 15:
        return "DOS/Windows";
    case 114:
        return "V7/x86";
    case 130:
        return "Solaris or Linux";
    case 131:
        return "Linux";
    case 165:
        return "FreeBSD";
    case 166:
        return "OpenBSD";
    case 169:
        return "NetBSD";
    case 191:
        return "Solaris";
    default:
        return "Unknown type";
    }
}

/* Zero block. */
zero(b, n)
char *b;
unsigned n;
{
    while (n--)
        *b++ = 0;
}

/* Get user confirmation. */
int
confirm(msg, a1, a2)
char *msg;
{
    int ok, c;

    do {
        printf(msg, a1, a2);
        printf("? (yn) ", msg);
        fflush(stdout);
        c = getchar();
        while (getchar() != '\n')
            if (feof(stdin))
                return 0;
        c = lower(c);
        ok = c == 'n' ? 0 : c == 'y' ? 1 : -1;
    } while (ok == -1);
    return ok;
}

/* Safe version of tolower(). */
int
lower(c)
{
    return isupper(c) ? tolower(c) : c;
}

error(ex, fmt, a1, a2)
char *fmt;
{
    fprintf(stderr, "ptdisk: ");
    if (ex == 0)
        fprintf(stderr, "Warning: ");
    fprintf(stderr, fmt, a1, a2);
    putc('\n', stderr);
    if (ex)
        exit(ex);
}

usage()
{
    fprintf(stderr, "usage: ptdisk [options] special [command ...]\n");
    exit(2);
}

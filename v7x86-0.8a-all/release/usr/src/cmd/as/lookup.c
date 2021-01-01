/* Copyright (c) 1995-2007 Robert Nordier.  All rights reserved. */

#include "optbl.h"
#include "lookup.h"

static unsigned char lx_tbl[128] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     1,  2,  3,  4,  5,  6,  7,  8,
     9, 10,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25,
    26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36,  0,  0,  0,  0,  0
};

static int mncmp();
static char *bsearch();

unsigned
lookup(sym,len,tbl)
char *sym;
unsigned len;
unsigned char tbl[][37];
{
    unsigned s, x;

    s = 0;
    while (len--) {
        x = lx_tbl[(unsigned char)*sym++];
        if (x == 0)
            return 0;
        s = tbl[s][x];
        if (s == 0 || (s >= 128 && len > 0))
            return 0;
    }
    if (s < 128)
        s = tbl[s][0];
    return s >= 128 ? s : 0;
}

int
mnlu(sym,sz)
char *sym;
int *sz;
{
    char *m;
    unsigned x, y;
    int z;

    m = bsearch(sym, mntbl, mnnum, sizeof(struct mntbl), mncmp);
    if (!m)
        return -1;
    x = ((struct mntbl *)m)->tk;
    y = x & 3;
    if (*sym == 'f')
        z = y == 1 ? 4 : y == 2 ? 8 : 0;
    else
        z = y == 1 ? 1 : y == 2 ? 2 : y == 3 ? 4 : 0;
    *sz = z;
    return (int)(x >> 2);
}

static int
mncmp(p1,p2)
char *p1;
char *p2;
{
    return strcmp(p1, ((struct mntbl *)p2)->mn);
}

static char *
bsearch(key, base, nmemb, size, compar)
char *key;
char *base;
unsigned nmemb;
unsigned size;
int (*compar) ();
{
    char *ptr;
    unsigned lo, hi, ix;
    int x;

    lo = 1;
    hi = nmemb;
    while (hi >= lo) {
        ix = (lo + hi) / 2;
        ptr = base + (ix - 1) * size;
        x = compar(key, ptr);
        if (!x)
            return ptr;
        if (x < 0)
            hi = ix - 1;
        else
            lo = ix + 1;
    }
    return 0;
}

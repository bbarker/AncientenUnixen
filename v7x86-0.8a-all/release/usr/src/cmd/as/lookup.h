/* Copyright (c) 1995-2007 Robert Nordier.  All rights reserved. */

struct mntbl {
    char *mn;
    unsigned tk;
};

extern unsigned char rmtbl[][37];
extern unsigned char potbl[][37];
extern struct mntbl mntbl[];
extern unsigned mnnum;

unsigned lookup();
int mnlu();

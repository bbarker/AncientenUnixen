/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 1996 Robert Nordier.  All rights reserved. */

int
memcmp(p1, p2, n)
char *p1;
char *p2;
unsigned n;
{
    unsigned char *m1, *m2;

    for(m1 = (unsigned char *)p1, m2 = (unsigned char *)p2; n--; m1++, m2++)
        if (*m1 != *m2)
            return *m1 - *m2;
    return 0;
}

char *
memcpy(d, s, n)
char *d;
char *s;
unsigned n;
{
    char *r;

    r = d;
    while (n--)
        *d++ = *s++;
    return r;    
}

char *
memset(m, c, n)
char *m;
unsigned n;
{
    char *r;

    r = m;
    while (n--)
        *m++ = c;
    return r;
}

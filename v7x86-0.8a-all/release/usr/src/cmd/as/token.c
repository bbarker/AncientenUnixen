/* Copyright (c) 1995-2007 Robert Nordier.  All rights reserved. */

#include <ctype.h>
#include <stdio.h>
#include "lookup.h"
#include "token.h"

extern char *index();

#define NL       1              /* newline */
#define WS       2              /* white space */
#define CM       3              /* comment */
#define TK       4              /* token */
#define LS       5              /* left shift */
#define RS       6              /* right shift */
#define SQ       7              /* single quote */
#define DQ       8              /* double quote */
#define ES       9              /* escape */
#define D0      10              /* zero */
#define D7      11              /* octal */
#define D9      12              /* decimal */
#define DX      13              /* hex */
#define XX      14              /* Xx */
#define ID      15              /* identifier */
#define SC      16              /* semi-colon */
#define ST      17              /* star */

#define Z0      0x80            /* End of statement */
#define Z1      0x81            /* Simple token */
#define Z2      0x82            /* Escaped token */
#define Z3      0x83            /* Char literal */
#define Z4      0x84            /* String literal */
#define Z5      0x85            /* Number */
#define Z6      0x86            /* Symbol */
#define Z7      0x87            /* Numeric label */
#define Z8      0x88            /* Forward slash */
#define Ze      0xff            /* Error */

#define VALMAX  0xffffffff
#define lower(c) (isupper(c) ? tolower(c) : (c))

char tk_str[256];
unsigned long line;
unsigned tk_len;
unsigned tk_val;
unsigned tk;
int ch;

static unsigned char lx_tbl[257] = {
    NL,
     0,  0,  0,  0,  0,  0,  0,  0,
     0, WS, NL,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
    WS, TK, DQ,  0, TK, TK, TK, SQ,
    TK, TK, ST, TK, TK, TK, ID, CM,
    D0, D7, D7, D7, D7, D7, D7, D7,
    D9, D9, TK, SC, LS, TK, RS,  0,
    TK, DX, DX, DX, DX, DX, DX, ID,
    ID, ID, ID, ID, ID, ID, ID, ID,
    ID, ID, ID, ID, ID, ID, ID, ID,
    XX, ID, ID, TK, ES, TK,  0, ID,
     0, DX, DX, DX, DX, DX, DX, ID,
    ID, ID, ID, ID, ID, ID, ID, ID,
    ID, ID, ID, ID, ID, ID, ID, ID,
    XX, ID, ID,  0, TK,  0,  0,  0
};

static unsigned char st_tbl[27][18] = {
  /* 00 NL WS CM TK LS RS SQ DQ ES D0 D7 D9 DX XX ID SC ST */
    /* 0: initial */
    {Ze,Z0, 0,26,Z1, 2, 3, 4,12,25,19,23,23,24,24,24,Z0,Z1},
    /* 1: comment */
    { 1,Z0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    /* 2: after left shift */
    {Ze,Ze,Ze,Ze,Ze,Z1,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze},
    /* 3: after right shift */
    {Ze,Ze,Ze,Ze,Ze,Ze,Z1,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze},
    /* 4: character literal: after single quote */
    { 5,Ze, 5, 5, 5, 5, 5,Ze, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5},
    /* 5: character literal: after unescaped char */
    { 5,Ze, 5, 5, 5, 5, 5,Z3, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5},
    /* 6: character literal: after escape */
    { 5,Ze, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 5, 5,10, 5, 5, 5},
    /* 7: character literal: after \[0-7] */
    { 5,Ze, 5, 5, 5, 5, 5,Z3, 5, 6, 8, 8, 5, 5, 5, 5, 5, 5},
    /* 8: character literal: after \[0-7][0-7] */
    { 5,Ze, 5, 5, 5, 5, 5,Z3, 5, 6, 9, 9, 5, 5, 5, 5, 5, 5},
    /* 9: character literal: after \[0-7][0-7][0-7] */
    { 5,Ze, 5, 5, 5, 5, 5,Z3, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5},
    /* 10: character literal: after \[Xx] */
    { 5,Ze, 5, 5, 5, 5, 5,Ze, 5, 6,11,11,11,11, 5, 5, 5, 5},
    /* 11: character literal: after \[Xx][0-9A-Fa-f] */
    { 5,Ze, 5, 5, 5, 5, 5,Z3, 5, 6,11,11,11,11, 5, 5, 5, 5},
    /* 12: string literal: after double quote */
    {12,Ze,12,12,12,12,12,12,Z4,13,12,12,12,12,12,12,12,12},
    /* 13: string literal: after escape */
    {12,Ze,12,12,12,12,12,12,12,12,14,14,12,12,17,12,12,12},
    /* 14: string literal: after \[0-7] */
    {12,Ze,12,12,12,12,12,12,Z4,13,15,15,12,12,12,12,12,12},
    /* 15: string literal: after \[0-7][0-7] */
    {12,Ze,12,12,12,12,12,12,Z4,13,16,16,12,12,12,12,12,12},
    /* 16: string literal: after \[0-7][0-7][0-7] */
    {12,Ze,12,12,12,12,12,12,Z4,13,12,12,12,12,12,12,12,12},
    /* 17: string literal: after \[Xx] */
    {12,Ze,12,12,12,12,12,12,Ze,13,18,18,18,18,12,12,12,12},
    /* 18: string literal: after \[Xx][0-9][A-Fa-f] */
    {12,Ze,12,12,12,12,12,12,Z4,13,18,18,18,18,12,12,12,12},
    /* 19: number: after 0 */
    {Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,20,20,Z5,Z5,21,Z5,Z5,Z5},
    /* 20: number: after 0[0-7] */
    {Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,20,20,Z5,Z5,Z5,Z5,Z5,Z5},
    /* 21: number: after 0[Xx] */
    {Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,22,22,22,22,Ze,Ze,Ze,Ze},
    /* 22: number: after 0[Xx][0-9A-Fa-f] */
    {Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,22,22,22,22,Z5,Z5,Z5,Z5},
    /* 23: number: after [1-9] */
    {Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,Z5,23,23,23,Z5,Z5,Z5,Z5,Z5},
    /* 24: symbol */
    {Z6,Z6,Z6,Z6,Z6,Z6,Z6,Z6,Z6,Z6,24,24,24,24,24,24,Z6,Z6},
    /* 25: after escape */
    {Ze,Ze,Ze,Z2,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Ze,Z2},
    /* 26: after forward slash */
    {Z8,Z8,Z8, 1,Z8,Z8,Z8,Z8,Z8,Z8,Z8,Z8,Z8,Z8,Z8,Z8,Z8,Z8}
};

static unsigned gettok1();
static int ch_esc();
static int struint();

unsigned
gettok(start)
int start;
{
    tk = gettok1(start);
    return tk;
}

static unsigned
gettok1(start)
int start;
{
    unsigned st, ns, lx, x;
    int c;

    tk_len = 0;
    if (ch == '\n')
        line++;
    for (st = start;; st = ns) {
        ch = getchar();
        lx = lx_tbl[ch + 1];
        ns = st_tbl[st][lx];
        if (ns >= Z0) {
            if (ns == Z0 || ns == Z1 || ns == Z2)
                tk_val = ch;
            tk_str[tk_len] = 0;
            if (ns >= Z5)
                if (ns == Z5 && (ch == 'b' || ch == 'f'))
                    ns = Z7;
                else {
                    ungetc(ch, stdin);
                    ch = 0;
                }
        }
        switch (ns) {
        case 5: case 6: case 12: case 13:
            if (st == 0)
                break;
            if (ns == 6 || ns == 13)
                ch = 0;
            else if (st == 6 || st == 13) {
                tk_len--;
                ch = ch_esc(ch);
            }
            /* FALLTHROUGH */
        case 19: case 20: case 21: case 22: case 23: case 24:
            tk_str[tk_len++] = ch;
            if (tk_len >= sizeof(tk_str))
                return ERR;
            break;
        case 7: case 8: case 9: case 14: case 15: case 16:
            tk_str[tk_len - 1] = tk_str[tk_len - 1] * 8 + ch - '0';
            break;
        case 11: case 18:
            tk_str[tk_len - 1] = tk_str[tk_len - 1] * 16 +
                ch - (ch <= '9' ? '0' : ch <= 'F' ? ('A' - 10):
                                                    ('a' - 10));
            break;
        case 0: case 1: case 2: case 3: case 4: case 10: case 17:
        case 25: case 26:
            break;
        case Z0:
            return EOS;
        case Z1:
            if (ch == ',')
                return COM;
            if (ch == ':')
                return COL;
            if (ch == '(')
                return LBR;
            if (ch == ')')
                return RBR;
            if (ch == '$')
                return DOL;
            if (ch == '*')
                return STA;
            if (ch == '=')
                return EQU;
            if (ch == '@')
                return ATT;
            return EXP;
        case Z2:
            return EXP;
        case Z3:
            tk_val = 0;
            if (tk_len > sizeof(tk_val))
                tk_len = sizeof(tk_val);
            for (x = 0; x < tk_len; x++)
                tk_val = (tk_val << 8) + tk_str[x];
            return INT;
        case Z4:
            return STR;
        case Z5:
        case Z7:
            if (struint(tk_str, &tk_val))
                return ERR;
            if (ns == Z5)
                return INT;
            x = ch == 'b' ? NLB : NLF;
            ch = 0;
            return x;
        case Z6:
            if (*tk_str == '.')
                return SYM;
            if (tk_str[0] == 's' && tk_str[1] == 't' &&
                tk_str[2] == 0) {
                c = getchar();
                if (c != '(') {
                    ungetc(c, stdin);
                    tk_val = 24;
                    return REG;
                }
                c = getchar();
                if (c >= '0' && c <= '7') {
                    tk_val = 64 + (c - '0');
                    c = getchar();
                    if (c == ')')
                        return REG;
                }
                ungetc(c, stdin);
                return SYN;
            }
            x = lookup(tk_str, tk_len, rmtbl);
            if (x >= 128) {
                tk_val = x - 128;
                return REG;
            }
            return SYM;
        case Z8:
            tk_val = ch = '/';
            return EXP;
        default:
            return ERR;
        }
    }
}

int
ahead()
{
    int c;

    do
        c = getchar();
    while (lx_tbl[c + 1] == WS);
    ungetc(c, stdin);
    return c;
}

static int
ch_esc(c)
int c;
{
    static char chr[] = "bfnrtv";
    static char esc[] = "\b\f\n\r\t\v";
    char *s;

    if (c > 0 && (s = index(chr, c)) != NULL)
        c = esc[s - chr];
    return c;
}

static int
struint(s, v)
char *s;
unsigned *v;
{
    unsigned base, valx, digx, val, dig;
    int c;

    base = *s == '0' ? lower(s[1]) == 'x' ? 16 : 8 : 10;
    if (base == 16)
        s += 2;
    valx = VALMAX / base;
    digx = VALMAX % base;
    val = 0;
    while ((c = *s++) != 0) {
        dig = isdigit(c) ? c - '0' :
            isalpha(c) ? 10 + lower(c) - 'a' :
            base;
        if (dig >= base || val > valx || val == valx && dig > digx)
            return -1;
        val = val * base + dig;
    }
    *v = val;
    return 0;
}

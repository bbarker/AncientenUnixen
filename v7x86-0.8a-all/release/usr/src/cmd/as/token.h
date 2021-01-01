/* Copyright (c) 1995-2007 Robert Nordier.  All rights reserved. */

#define EOS  0x00               /* End of statement */
#define COL  0x01               /* Colon */
#define COM  0x02               /* Comma */
#define LBR  0x03               /* Left bracket */
#define RBR  0x04               /* Right bracket */
#define EQU  0x05               /* Equals XXX */
#define STA  0x06               /* Star */
#define DOL  0x07               /* Dollar */
#define ATT  0x08               /* At (type) */
#define STR  0x09               /* String */
#define NLB  0x0a               /* Numeric label back */
#define NLF  0x0b               /* Numeric label forward */
#define SYN  0x0f               /* (Cause syntax error) */
#define EXP  0x10               /* Expression operators */
#define INT  0x20               /* Integer */
#define SYM  0x40               /* Symbol */
#define REG  0x80               /* Register */
#define ERR  0xff               /* Error */

extern char tk_str[256];
extern unsigned long line;
extern unsigned tk_len;
extern unsigned tk_val;
extern unsigned tk;
extern int ch;

unsigned gettok();
int ahead();

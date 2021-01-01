/* Copyright (c) 1995-2007 Robert Nordier.  All rights reserved. */

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include "lookup.h"
#include "optbl.h"
#include "token.h"

extern char *malloc();

/* expression evaluation */
#define NVAL  32
#define NSTK  64

/* effective address */
#define ESP  4
#define BP   5

/* symbol table */
#define NST  1031

/* sdi handling */
#define SX  8

/* types */
#define T_UNDF  0
#define T_ABS   2
#define T_TEXT  4
#define T_DATA  6
#define T_BSS   8

#define emit1(x) emit(1, (x))

#define regsiz(x)  ((x) < 8 ? 1 : (x) < 16 ? 2 : 4)

/* error numbers */
#define E_LEX            1
#define E_SYN            2
#define E_RES            3
#define E_DUP            4
#define E_MNM            5
#define E_PAREN          6
#define E_DIV            7
#define E_EA             8
#define E_OPRNUM         9
#define E_OPRTYPE       10
#define E_OPR           11
#define E_SUFF          12
#define E_CRIT          13
#define E_EXPR          14
#define E_EXPROP        15
#define E_RANGE         16
#define E_COMP          17
#define E_MISC          18
#define E_SCALE         19
#define E_BSS           20
#define E_POW           21
#define E_PHASE         22

static char *errmsg[] = {
    NULL,
    "Lexical error",
    "Syntax error",
    "Use of reserved word as symbol",
    "Duplicate symbol",
    "Unknown instruction",
    "Unbalanced parentheses",
    "Division by zero",
    "Bad effective address",
    "Wrong number of operands",
    "Wrong operand type",
    "Bad operand",
    "Suffix required",
    "Critical expression unresolved",
    "Bad expression",
    "Wrong type in expression",
    "Value out of range",
    "Expression too complex",
    "Miscellaneous error",
    "Bad scale factor",
    "Non-zero output to .bss",
    "Not a power of two",
    "Phase error"
};

struct op1 {
    unsigned char t;
    unsigned char a;
    unsigned char c;
    unsigned char x;
    struct sym *s;
};

struct op2 {
    struct op1 o;
    unsigned d;
};

struct expr {
    struct sym *s;
    int t;
    int v;
};

struct sym {
    struct sym *next;
    struct sym *link;
    char *key;
    unsigned len;
    unsigned id;
    short type;
    unsigned char t2;
    unsigned char ex;
    unsigned val;
    unsigned long line;
    int sdi;
};

struct sdi {
    struct sdi *n;              /* next */
    int i;                      /* number */
    int t;                      /* type */
    int x;                      /* exclude */
    unsigned a;                 /* address */
    struct sym *l;              /* label */
    int c;                      /* constant */
    int i2;                     /* operand sdi id */
    int s;                      /* span */
};

struct aln {
    struct aln *n;              /* next */
    unsigned i;                 /* number */
    unsigned a;                 /* address */
    int t;                      /* type */
    int z;                      /* size */
};

struct stbl {
    unsigned i;                 /* number */
    int d;                      /* displacement */
};

struct rel {
    struct rel *n;              /* next */
    unsigned a;                 /* address */
    struct sym *s;              /* symbol */
    int z;                      /* size */
    int p;                      /* pcrel */
};

static int optA;                /* XXX hack for ACK */

static char *source;
static char *object = "a.out";

static int sect;
static int isect;
static int cdsz;
static int pass;
static int undef;
static unsigned pcs[3];
static unsigned lab[10];
static unsigned errs;

static unsigned char *bf;
static unsigned bs[3];

static struct sym *symtab[NST];
static struct sym *stlist;
static struct sym **ste = &stlist;
static unsigned symid;

static struct sdi *sdi[2];
static char *sdilong[2];
static int nsdi[2];
static unsigned *incr;

static struct aln alnend[2] = {
    {NULL, 999999, 0, 65536, 0},
    {NULL, 999999, 0, 65536, 0}
};
static struct aln *aln[2] = {&alnend[0], &alnend[1]};
static struct aln **alnp[2] = {&aln[0], &aln[1]};

static struct rel *rel[2];
static unsigned nrel[2];

static fail();
static int as();
static int parse();
static int pseudo();
static int machop();
static int gen();
static int doea();
static int expr();
static struct sym *symput();
static int symcmp();
static int sdisz();
static sdiadd();
static sdialn();
static do_sdi();
static sdichk();
static int sdiset();
static int sdiadj();
static reladd();
static unsigned relout();
static endsect();
static fixsym();
static write();
static unsigned symout();
static int setcj();
static int doreg();
static int isbyte();
static int setlab();
static emitn();
static emit();
static output();
static char *alloc();
static error();
static int bzero();
static int bcopy();

int
main(argc, argv)
int argc;
char *argv[];
{
    char *p;
    int e, i, c;

    e = 0;
    for (i = 1; i < argc && *(p = argv[i]) == '-'; i++) {
        if (*++p == 0)
            break;
        do
            switch (c = *p++) {
            case 'u':
                break;
            case 'A':
                optA = 1;
                break;
            case 'o':
                if (*p)
                    object = p;
                else if (++i < argc)
                    object = argv[i];
                else {
                    fprintf(stderr, "missing argument: -%c\n", c);
                    e++;
                }
                continue;
            default:
                fprintf(stderr, "unknown option: -%c\n", c);
                e++;
            }
        while (*p);
    }
    if (e || argc - i != 1) {
        fprintf(stderr, "usage: as [-GLu] [-o object] source\n");
        exit(1);
    }
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
        signal(SIGINT, fail);
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
        signal(SIGTERM, fail);
    source = argv[i];
    if (freopen(source, "r", stdin) == NULL) {
        fprintf(stderr, "as: %s: Cannot open input file\n", source);
        exit(1);
    }
    if (freopen(object, "w", stdout) == NULL) {
        fprintf(stderr, "as: %s: Cannot open output file\n", object);
        exit(1);
    }
    e = as();
    if (e != 0)
        fail();
    return 0;
}

static
fail()
{
    unlink(object);
    exit(1);
}

static int
as()
{
    int e, i;

    for (pass = 0; pass <= 1; pass++) {
        if (pass)
            rewind(stdin);
        for (i = 0; i < 3; i++)
            pcs[i] = bs[i];
        nsdi[0] = nsdi[1] = 0;
        sect = T_TEXT;
        isect = 0;
        cdsz = 4;
        for (i = 0; i < 10; i++)
            lab[i] = 0;
        line = 1;
        while ((e = parse()) != 0) {
            error(e);
            while (tk != EOS)
                gettok(1);
        }
        if (errs != 0)
            break;
        if (pass == 0)
            for (i = 0; i < 2; i++) {
                sdilong[i] = alloc(1 + nsdi[i], 1);
                do_sdi(i);
            }
        for (i = 0; i < 2; i++)
            endsect(i);
        if (!pass) {
            bs[1] = pcs[0];
            bs[2] = pcs[0] + pcs[1];
            fixsym();
            bf = (unsigned char *)alloc(bs[2], 0);
        } else
            write();
    }
    return errs != 0;
}

static int
parse()
{
    struct sym *sym;
    int a, e;

    do {
        gettok(0);
        if (tk == ERR)
            return E_LEX;
        if (tk == EOS)
            continue;
        a = ahead() == ':';
        if ((tk & (INT | REG | SYM)) == 0 || (!a && tk == INT))
            return E_SYN;
        if (a) {
            if (tk == INT) {
                e = setlab(2);
                if (e)
                    return e;
            } else if (tk != SYM || (tk_len == 1 && *tk_str == '.'))
                return E_RES;
            sym = symput(tk_str, tk_len);
            if (!pass) {
                if (sym->line != 0)
                    return E_DUP;
                sym->line = line;
                sym->type = sect;
                sym->val = pcs[isect];
                sym->sdi = nsdi[isect];
            } else if (sym->val != pcs[isect]) {
                fprintf(stderr, "Label \"%.*s\" was %u now %u\n",
                        (int)tk_len, tk_str, sym->val, pcs[isect]);
                return E_PHASE;
            }
            gettok(0);
            continue;
        }
        if (*tk_str == '.')
            e = pseudo();
        else
            e = machop();
        if (e)
            return e;
    } while (ch != EOF);
    return 0;
}

static int
pseudo()
{
    char id[256];
    struct expr ex1, ex2;
    struct sym *sym;
    unsigned op, x, r, b;
    int e, n;

    op = lookup(tk_str + 1, tk_len - 1, potbl);
    if (op == 0)
        return E_MNM;
    switch (op) {
    case 128:                   /* .set */
        if (gettok(0) != SYM)
            return E_OPRTYPE;
        strcpy(id, tk_str);
        if (gettok(0) != COM)
            return E_SYN;
        gettok(0);
        if ((e = expr(&ex1)) != 0)
            return e;
        if (id[0] == '.' && id[1] == 0) {
            n = ex1.v - pcs[isect];
            if (n < 0)
                return E_MISC;
            emitn(n, 0);
        } else {
            sym = symput(id, strlen(id));
            if (!pass && sym->line != 0)
                return E_DUP;
            sym->line = line;
            sym->type = ex1.t;
            sym->val = ex1.t == T_UNDF ? 0 : ex1.v;
        }
        break;
    case 129:                   /* .ascii */
    case 130:                   /* .asciz */
        do {
            if (gettok(0) != STR)
                return E_OPRTYPE;
            for (x = 0; x < tk_len; x++)
                emit(1, tk_str[x]);
            if (op == 130)
                emit(1, 0);
        } while (gettok(0) == COM);
        break;
    case 131:                   /* .byte */
    case 132:                   /* .word */
    case 133:                   /* .long */
        do {
            gettok(0);
            if ((e = expr(&ex1)) != 0)
                return e;
            x = op == 131 ? 1 : op == 132 ? 2 : 4;
            if (ex1.s != NULL && ex1.s->type != T_ABS)
                reladd(pcs[isect], ex1.s, x, 0);
            emit(x, ex1.v);
        } while (tk == COM);
        break;
    case 134:                   /* .align */
    case 135:                   /* .space */
    case 143:                   /* .p2align */
        gettok(0);
        if ((e = expr(&ex1)) != 0)
            return e;
        if (!pass && undef)
            return E_CRIT;
        if (ex1.v < 0 || (op == 143 && ex1.v > 15) || ex1.v > 32768)
            return E_RANGE;
        x = 0;
        if (tk == COM) {
            gettok(0);
            if ((e = expr(&ex2)) != 0)
                return e;
            x = ex2.v;
        }
        if (op == 134 || op == 143) {
            if (op == 134) {
                r = ex1.v == 0 ? 1 : ex1.v;
                for (b = 1; b < r; b <<= 1);
                if (b != r)
                    return E_POW;
            } else
                r = 1 << ex1.v;
            n = (pcs[isect] - bs[isect]) % r;
            if (n)
                n = r - n;
            if (r > 1 && sect != T_BSS)
                sdialn(r, n);
        } else
            n = ex1.v;
        emitn(n, x);
        break;
    case 136:                   /* .text */
    case 137:                   /* .data */
    case 138:                   /* .bss */
        sect = op == 138 ? T_BSS : op == 137 ? T_DATA : T_TEXT;
        isect = sect == T_BSS ? 2 : sect == T_DATA ? 1 : 0;
        gettok(0);
        break;
    case 139:                   /* .comm */
    case 140:                   /* .lcomm */
        if (gettok(0) != SYM)
            return E_OPRTYPE;
        if (tk_len == 1 && *tk_str == '.')
            return E_RES;
        strcpy(id, tk_str);
        if (gettok(0) != COM)
            return E_SYN;
        gettok(0);
        if ((e = expr(&ex1)) != 0)
            return e;
        if (ex1.t != T_ABS || undef)
            return E_CRIT;
        if (ex1.v < 0)
            return E_RANGE;
        sym = symput(id, strlen(id));
        if (op == 139 && (!optA || sym->ex)) {
            sym->type = T_UNDF;
            if (ex1.v < sym->val)
                return E_MISC;
            sym->val = ex1.v;
        } else {
            if (!pass && sym->line != 0)
                return E_DUP;
            sym->type = T_BSS;
            sym->val = pcs[2];
            pcs[2] += ex1.v;
        }
        sym->line = line;
        break;
    case 141:                   /* .globl */
        do {
            if (gettok(0) != SYM)
                return E_OPRTYPE;
            if (tk_len == 1 && *tk_str == '.')
                return E_RES;
            strcpy(id, tk_str);
            sym = symput(id, strlen(id));
            sym->ex = 1;
            gettok(0);
        } while (tk == COM);
        break;
    case 142:                   /* .ident */
    case 144:                   /* .file */
        if (gettok(0) != STR)
            return E_OPRTYPE;
        gettok(0);
        break;
    case 145:                   /* .type */
        if (gettok(0) != SYM)
            return E_OPRTYPE;
        strcpy(id, tk_str);
        if (gettok(0) != COM)
            return E_SYN;
        if (gettok(0) != ATT)
            return E_SYN;
        if (gettok(0) != SYM)
            return E_OPRTYPE;
        if (!strcmp(tk_str, "object"))
            x = 1;
        else if (!strcmp(tk_str, "function"))
            x = 2;
        else
            return E_SYN;
        sym = symput(id, strlen(id));
        sym->t2 = x;
        gettok(0);
        break;
    case 146:                   /* .size */
        if (gettok(0) != SYM)
            return E_OPRTYPE;
        if (tk_len == 1 && *tk_str == '.')
            return E_RES;
        strcpy(id, tk_str);
        if (gettok(0) != COM)
            return E_SYN;
        gettok(0);
        if ((e = expr(&ex1)) != 0)
            return e;
        if (ex1.t != T_ABS || undef)
            return E_CRIT;
        sym = symput(id, strlen(id));
        break;
    case 147:                   /* .code16 */
    case 148:                   /* .code32 */
        cdsz = op == 147 ? 2 : 4;
        gettok(0);
        break;
    default:
        return E_MNM;
    }
    if (tk != EOS)
        return E_SYN;
    return 0;
}

static int
machop()
{
    struct op2 op[3], xx;
    struct expr ex;
    unsigned opc, oc;
    int sz, rz, cj, dr, z, e;

    bzero(op, sizeof(op));
    sz = rz = 0;
    e = mnlu(tk_str, &sz);
    if (e < 0)
        return E_MNM;
    opc = e;
    cj = setcj(opc);
    oc = 0;
    gettok(0);
    if (tk != EOS) {
        for (;;) {
            if (oc >= 3)
                return E_OPRNUM;
            dr = cj;
            if (tk == STA) {
                if (dr == 1)
                    return E_SYN;
                dr = 1;
                gettok(0);
            } else if (tk == DOL) {
                if (dr == -1)
                    return E_SYN;
                dr = -1;
                gettok(0);
            }
            if (tk == REG) {
                if (dr != 1)
                    return E_OPR;
                doreg(&op[oc].o);
                gettok(0);
            } else if (opsfx[opc] & 0x20 && tk == LBR) {
                if (gettok(0) != REG)
                    return E_SYN;
                doreg(&op[oc].o);
                if (gettok(0) != RBR)
                    return E_SYN;
                gettok(0);
            } else if (dr == -1) {
                if ((e = expr(&ex)) != 0)
                    return e;
                op[oc].d = ex.v;
                op[oc].o.t = OT_I;
                op[oc].o.a = undef ? 0 : 3;
                op[oc].o.s = ex.s;
                z = sdisz(opc);
                if (z)
                    sdiadd(z, &ex);
            } else {
                if (dr != 1)
                    return E_OPR;
                if ((e = doea(&op[oc], &rz)) != 0)
                    return e;
            }
            oc++;
            if (tk != COM)
                break;
            gettok(0);
        }
        if (tk != EOS)
            return E_SYN;
    }
    if (oc > 1) {
        xx = op[0];
        op[0] = op[oc - 1];
        op[oc - 1] = xx;
    }
    return gen(opc, sz, rz, oc, op);
}

static int
gen(tk, sz, rz, oc, op)
unsigned tk;
int sz;
int rz;
unsigned oc;
struct op2 op[];
{
    struct optbl *opt;
    unsigned opc, wopt, x, n, i, j, k;
    int fop, bits;

    assert(rz == 0 || rz == 2 || rz == 4);
    for (x = opidx[tk]; x < opidx[tk + 1]; x++) {
        n = optbl[x].op[2].om ? 3 :
            optbl[x].op[1].om ? 2 :
            optbl[x].op[0].om ? 1 : 0;
        if (oc != n)
            continue;
        for (i = 0; i < n; i++) {
            /* check type */
            j = op[i].o.t;
            if ((op[i].o.t & optbl[x].op[i].ot) != op[i].o.t)
                break;
            /* check sub-type or size */
            if (op[i].o.t == OT_X) {
                if (op[i].o.a != optbl[x].op[i].oa)
                    break;
            } else if (optbl[x].op[i].oa) {
                if (op[i].o.t != OT_I)
                    if ((op[i].o.a & optbl[x].op[i].oa) != op[i].o.a)
                        break;
            }
            /* check mask */
            if (optbl[x].op[i].om != 0xff)
                if (op[i].o.t == OT_I) {
                    if (op[i].d != optbl[x].op[i].om)
                        break;
                } else {
                    j = op[i].o.c;
                    if (op[i].o.t == OT_M)
                        if ((rz == 2 && j == 6) || (rz == 4 && j == 5))
                            j = 6;
                        else
                            j = 8;
                    if (j > 7 || (optbl[x].op[i].om & (1 << j)) == 0)
                        break;
                }
        }
        if (i == n)
            break;
    }
    if (x >= opidx[tk + 1])
        return E_OPR;
    opt = optbl + x;
    /* XXX This needs to be sorted out better. */
    if ((opt->ox & OX_X) == 0 &&
        (opt->o0 == 0004 || opt->o0 == 0014 ||
         opt->o0 == 0024 || opt->o0 == 0034 ||
         opt->o0 == 0044 || opt->o0 == 0054 ||
         opt->o0 == 0064 || opt->o0 == 0074))
        if ((op[0].o.a == 4) && op[1].o.a != 0 &&
            (!op[1].o.s || op[1].o.s->type == T_ABS) &&
            isbyte(op[0].o.a, op[1].d))
            opt += 3;
    if (opt->o0 >= 0x70 && opt->o0 <= 0x7f && pass &&
        sdilong[isect][nsdi[isect]])
        opt++;
    fop = opt->o0 >= 0xd8 && opt->o0 <= 0xdf;
    /* sort out whether 32 or 16 bit */
    if (rz && rz != cdsz)
        emit1(0x67);
    bits = cdsz;
    if (!fop) {
        if (sz == 0) {
            x = opt->ox & 7;
            if (x == 2 || x == 3)
                sz = (x == 2) ? 2 : 4;
            else if (x >= 4) {
                if (x == 4 || x == 5)
                    sz = op[x - 4].o.a;
                if (sz == 0)
                    return E_SUFF;
            }
        }
        if ((sz == 2 || sz == 4) && sz != bits) {
            bits ^= 6;
            emit1(0x66);
        }
    }
    /* do the encoding proper */
    if (opt->ox & OX_X)
        emit1(0xf);
    opc = opt->o0;
    if (opt->ox & OX_U) {
        if (sz == 0)
            opc |= 4;
    } else if (opt->ox & OX_V) {
        if (fop) {
            if (sz == 8)
                opc |= 4;
        } else {
            if (sz == 0)
                return E_SUFF;
            if (sz != 1)
                opc |= 1;
        }
    }
    wopt = 0;
    if (sz != 1 && opt->ox & OX_W)
        if (opt->ox & OX_Y)
            wopt = !pass || sdilong[isect][nsdi[isect]] == 0;
        else if (op[n - 1].o.s == NULL ||
                 op[n - 1].o.s->type == T_ABS)
            wopt = isbyte(bits, op[n - 1].d);
    if (wopt)
        opc |= 2;
    x = (opt->ox & OX_C) >> 8;
    if (x == 0) {
        emit1(opc);
        if (opt->o1)
            emit1(opt->o1);
    } else if (x == 1) {
        i = opt->op[0].om != 0xff;
        emit1(opc | op[i].o.c);
    } else {
        unsigned ea;
        /* effective address */
        ea = (opt->ox & OX_Z) ? 1 : 0;
        assert((x & 1) == ea);
        emit1(opc);
        x = 0;
        if (opt->op[ea].om == 0xff) {
            if ((opt->op[!ea].ot & OT_R || opt->op[!ea].ot & OT_X) &&
                opt->op[!ea].om == 0xff)
                x = op[!ea].o.c;
            else
                x = opt->o1;
            x <<= 3;
            x |= op[ea].o.c;
            if (op[ea].o.t != OT_M)
                x |= 0xc0;
            emit1(x);
            if (rz == 4 && (x & 7) == 4)
                emit1(op[ea].o.x);
        }
        if (op[ea].o.t == OT_M)
            if (rz == 2 && op[ea].o.c == 6)
                x = 2;
            else if (rz == 4 && (op[ea].o.c == 5 ||
                                 (op[ea].o.c == 4 &&
                                  (op[ea].o.x & 7) == 5)))
                x = 4;
            else {
                x >>= 6;
                if (x == 2)
                    x = rz;
            }
        if (x == 1 || x == 2 || x == 4) {
            if (op[ea].o.s != NULL && op[ea].o.s->type != T_ABS)
                reladd(pcs[isect], op[ea].o.s, rz, 0);
            emit(x, op[ea].d);
        }
    }
    /* deal with immediate data */
    for (i = 0; i < 3; i++)
        if (opt->op[i].ot & OT_I && opt->op[i].om == 0xff) {
            x = op[i].d;
            k = opt->op[i].oa;
            if (k == 6 || k == 7)
                k = opt->ox & OX_V ? sz : bits;
            assert(k != 0);
            if (opt->ox & OX_Y) {
                assert(k == 1 || k == 2 || k == 4);
                x -= pcs[isect] + (wopt ? wopt : k);
            }
            j = k == 1 || wopt ? 1 : k == 2 ? 2 : 4;
            if (op[i].o.s != NULL &&
                (op[i].o.s->type == T_UNDF ||
                 ((opt->ox & OX_Y) == 0 && op[i].o.s->type != T_ABS)))
                reladd(pcs[isect], op[i].o.s, j, (opt->ox & OX_Y) != 0);
            emit(j, x);
        }
    return 0;
}

static int
doea(o, rz)
struct op2 *o;
int *rz;
{
    static unsigned char tbl[5][5] = {
        /* - bx bp si di */
        {6, 7, 6, 4, 5},        /* - */
        {7, 9, 9, 0, 1},        /* bx */
        {6, 9, 9, 2, 3},        /* bp */
        {4, 0, 2, 9, 9},        /* si */
        {5, 1, 3, 9, 9}         /* di */
    };
    struct expr ex1, ex2;
    int ad, md, bs, ix, sc, rm;
    int r[2], x, e, i;

    bzero(&ex1, sizeof(ex1));
    ad = md = 0;
    bs = ix = sc = -1;
    if (tk != LBR) {
        if ((e = expr(&ex1)) != 0)
            return e;
        ad = ex1.v;
        x = undef || (ex1.s && ex1.s->type != T_ABS);
        md = x || ad < -128 || ad > 127 ? 2 : ad ? 1 : 0;
    }
    if (tk == LBR) {
        if (gettok(0) == REG) {
            if ((*rz = regsiz(tk_val)) == 1)
                return E_EA;
            bs = tk_val & 7;
            gettok(0);
        } else if (tk == RBR)
            return E_EA;
        if (tk == COM) {
            if (gettok(0) == REG) {
                if ((x = regsiz(tk_val)) == 1)
                    return E_EA;
                if (*rz == 0)
                    *rz = x;
                else if (x != *rz)
                    return E_EA;
                ix = tk_val & 7;
                gettok(0);
                if (tk == COM) {
                    if (*rz == 2)
                        return E_EA;
                    gettok(0);
                    if (tk == RBR)
                        return E_EA;
                } else if (tk != RBR)
                    return E_EA;
            } else if (tk == RBR)
                return E_EA;
            if (tk != RBR) {
                if (*rz == 0)
                    *rz = cdsz;
                if (ix == -1 || *rz == 2)
                    return E_EA;
                if ((e = expr(&ex2)) != 0)
                    return e;
                if (undef)
                    return E_CRIT;
                x = ex2.v;
                sc = x == 1 ? 0 :
                    x == 2 ? 1 :
                    x == 4 ? 2 :
                    x == 8 ? 3 : -1;
                if (sc == -1)
                    return E_SCALE;
            }
        }
        if (tk != RBR)
            return E_EA;
        gettok(0);
    }
    if (*rz == 0)
        *rz = cdsz;
    if (bs == -1 && (*rz == 4 || ix == -1)) {
        md = 0;
        bs = BP;
    } else if (bs == BP && (*rz == 4 || ix == -1) && md == 0)
        md = 1;
    if (*rz == 2) {
        r[0] = bs, r[1] = ix;
        for (i = 0; i < 2; i++) {
            x = 0;
            if (r[i] != -1) {
                x = r[i] - 3;
                if (x == 0)
                    x++;
                else if (x < 2)
                    return E_EA;
            }
            r[i] = x;
        }
        if ((rm = tbl[r[0]][r[1]]) > 7)
            return E_EA;
    } else {
        if (ix == ESP)
            return E_EA;
        rm = (ix != -1 || sc != -1) ? ESP : bs;
        if (rm == ESP) {
            if (ix == -1)
                ix = ESP;
            if (sc == -1)
                sc = 0;
            o->o.x = sc << 6 | ix << 3 | bs;
        }
    }
    o->o.t = OT_M;
    o->o.a = 0;
    o->o.c = md << 6 | rm;
    o->o.s = ex1.s;
    o->d = ad;
    return 0;
}

static int
expr(result)
struct expr *result;
{
    struct expr val[NVAL], *p, *q;
    struct sym *sym;
    int stk[NSTK];
    int vp, sp, st, e, c, d, v;

    undef = 0;
    vp = sp = -1;
    stk[++sp] = 0;
    st = 0;
    for (;;) {
        if ((tk & (INT | SYM)) || (tk == NLB || tk == NLF)) {
            if (st++ != 0)
                return E_EXPR;
            if (++vp >= NVAL)
                return E_COMP;
            if (tk == INT) {
                val[vp].s = NULL;
                val[vp].t = T_ABS;
                val[vp].v = tk_val;
            } else if (tk == SYM && tk_len == 1 && *tk_str == '.') {
                val[vp].s = NULL;
                val[vp].t = sect;
                val[vp].v = pcs[isect];
            } else {
                if (tk != SYM) {
                    e = setlab(tk == NLF);
                    if (e)
                        return e;
                }
                sym = symput(tk_str, tk_len);
                val[vp].s = sym;
                val[vp].t = sym->type;
                val[vp].v = sym->type == T_UNDF ? 0 : sym->val;
                if (sym->line == 0 || sym->line > line)
                    undef = 1;
            }
        } else {
            if (st == 1)
                while ((d = stk[sp]) != '[' && d != 0) {
                    assert(vp > 0);
                    q = &val[vp--];
                    p = &val[vp];
                    if (p->t != T_ABS || q->t != T_ABS) {
                        if (pass && d != '+' && d != '-')
                            return E_EXPROP;
                        if (p->t != T_ABS && q->t != T_ABS) {
                            if (pass && d != '-')
                                return E_EXPROP;
                            if (pass && q->t != p->t)
                                return E_EXPROP;
                            if (p->t == T_UNDF || q->t == T_UNDF)
                                if (p->s != q->s) {
                                    if (pass)
                                        return E_EXPROP;
                                    undef = 1;
                                }
                            p->s = NULL;
                            p->t = T_ABS;
                        } else if (q->t != T_ABS) {
                            if (pass && d != '+')
                                return E_EXPROP;
                            p->s = q->s;
                            p->t = q->t;
                        }
                    }
                    v = q->v;
                    switch (d) {
                    case '*':
                        p->v *= v;
                        break;
                    case '/':
                        if (v == 0)
                            return E_DIV;
                        p->v /= v;
                        break;
                    case '%':
                        p->v %= v;
                        break;
                    case '+':
                        p->v += v;
                        break;
                    case '-':
                        p->v -= v;
                        break;
                    case '<':
                        p->v <<= v;
                        break;
                    case '>':
                        p->v >>= v;
                        break;
                    case '&':
                        p->v &= v;
                        break;
                    case '!':
                        p->v |= ~v;
                        break;
                    case '|':
                        p->v |= v;
                        break;
                    default:
                        return E_MISC;
                    }
                    sp--;
                }
            c = tk != EXP && tk != STA ? 0 : tk_val;
            if (c == '[' || c == ']' || c == 0) {
                if (st != (c != '['))
                    return E_EXPR;
            } else if (st == 0) {
                if (c != '+' && c != '-' && c != '!')
                    return E_EXPR;
                if (++vp >= NVAL)
                    return E_COMP;
                val[vp].s = NULL;
                val[vp].t = T_ABS;
                val[vp].v = 0;
            }
            if (c == ']' || c == 0) {
                if ((stk[sp--] == '[') != (c == ']'))
                    return E_PAREN;
                if (c == 0)
                    break;
            } else {
                if (++sp >= NSTK)
                    return E_COMP;
                stk[sp] = c;
                st = 0;
            }
        }
        gettok(0);
    }
    assert(vp == 0);
    *result = val[vp--];
    return 0;
}

static struct sym *
symput(key, len)
char *key;
unsigned len;
{
    struct sym **sp, *s;
    unsigned x, i;

    if (len > 8)
        len = 8;
    x = 0;
    for (i = 0; i < len; i++)
        x = (x << 3) + key[i];
    for (sp = &symtab[x % NST]; s = *sp; sp = &s->link)
        if (len == s->len && !symcmp(key, s->key, len))
            return s;
    s = (struct sym *)alloc(sizeof(struct sym), 1);
    s->key = alloc(len, 0);
    bcopy(key, s->key, len);
    s->len = len;
    if (!isdigit(*key))
        s->id = symid++;
    *ste = s;
    ste = &s->next;
    *sp = s;
    return s;
}

static int
symcmp(s1, s2, n)
char *s1;
char *s2;
unsigned n;
{
    while (n--)
        if (*s1++ != *s2++)
            return 1;
    return 0;
}

static int
sdisz(opc)
unsigned opc;
{
    static int sz[2][4] = {
        {0, 3 - 2, 4 - 2},
        {0, 5 - 2, 6 - 2}
    };
    int x;

    x = (opsfx[opc] & 0x18) >> 3;
    assert(x < 3);
    return sz[cdsz == 4][x];
}

static
sdiadd(sz, ex)
int sz;
struct expr *ex;
{
    struct sdi *s;

    assert(sz != 0);
    nsdi[isect]++;
    if (pass == 0) {
        s = (struct sdi *)alloc(sizeof(struct sdi), 0);
        s->n = sdi[isect];
        s->i = nsdi[isect];
        s->t = sz;
        s->x = 0;
        s->a = pcs[isect];
        s->l = ex->s;
        s->c = ex->v - (ex->s ? ex->s->val : pcs[isect]);
        s->i2 = 0;
        s->s = 0;
        sdi[isect] = s;
    }
}

static
sdialn(t, z)
int t;
int z;
{
    struct aln *p;

    assert(z >= 0 && z < t);
    nsdi[isect]++;
    if (pass == 0) {
        p = (struct aln *)alloc(sizeof(struct aln), 0);
        p->n = *alnp[isect];
        p->i = nsdi[isect];
        p->a = pcs[isect];
        p->t = t;
        p->z = t - z;
        *alnp[isect] = p;
        alnp[isect] = &p->n;
    }
}

static
do_sdi(is)
int is;
{
    struct sym *sym;
    struct sdi *s;
    unsigned e;
    int it, x;

    incr = (unsigned *)alloc((1 + nsdi[is]) * sizeof(unsigned), 1);
    it = is ? T_DATA : T_TEXT;
    for (s = sdi[is]; s; s = s->n) {
        if (s->l && s->l->type != it)
            s->x = 1;
        else {
            e = s->l ? s->l->val : s->a;
            s->i2 = s->l ? s->l->sdi : s->i;
            s->s = e + s->c - (s->a + 2);
            if ((s->s < -128 && e > s->a) ||
                (s->s >  127 && e < s->a))
                s->x = 1;
        }
    }
    for (s = sdi[is]; s; s = s->n)
        if (!sdilong[is][s->i])
            sdichk(is, s);

    for (sym = stlist; sym; sym = sym->next) {
        if (sym->type != it)
            continue;
        x = incr[sym->sdi];
        if (x != 0)
            sym->val += x;
    }
    pcs[is] += incr[nsdi[is]];
    free(incr);
    incr = NULL;
}

static
sdichk(is, s)
int is;
struct sdi *s;
{
    struct stbl tbl[SX];
    struct sdi *q;
    int e, x1, x2, x;

    if (s->x || s->s < -128 || s->s > 127) {
        sdilong[is][s->i] = s->t;
        e = sdiset(is, tbl, s->i, s->t);
        for (q = sdi[is]; q; q = q->n) {
            if (q->x || sdilong[is][q->i])
                continue;
            x1 = x2 = 0;
            if (q->i >= s->i && q->i < e)
                x1 = sdiadj(tbl, q->i);
            if (q->i2 >= s->i && q->i2 < e)
                x2 = sdiadj(tbl, q->i2);
            x = x2 - x1;
            if (x) {
                q->s += x;
                sdichk(is, q);
            }
        }
    }
}

static int
sdiset(is, tbl, n0, d)
int is;
struct stbl *tbl;
unsigned n0;
int d;
{
    struct aln *p;
    int n1, r, t, j, k, x;

    r = 0;
    t = d == 3 ? 1 : d;
    p = aln[is];
    j = n0;
    n1 = n0;
    for (;;) {
        tbl[r].d = d;
        tbl[r].i = n1;
        if (r++) {
            assert(r >= 0 && r < SX);
            while (j < n1) {
                assert(j >= 0 && j <= nsdi[is]);
                incr[j++] += k;
            }
        }
        if ((k = d) == 0)
            break;
        while (p->i <= n1 || p->t <= t)
            p = p->n;

        x = p->z + d;
        d = 0;
        while (x > p->t) {
            x -= p->t;
            d += p->t;
        }
        p->z = x;
        t = d;

        n1 = p->i;
        if (n1 == 999999) {
            n1 = nsdi[is] + 1;
            t = d = 0;
        }
    }
    return n1;
}

static int
sdiadj(tbl, i)
struct stbl *tbl;
unsigned i;
{
    struct stbl *e = tbl + SX;

    while (tbl[1].i <= i) {
        tbl++;
        assert(tbl < e);
    }
    return tbl->d;
}

static
reladd(addr, sym, size, pcrel)
unsigned addr;
struct sym *sym;
int size;
int pcrel;
{
    struct rel *r;

    if (!pass)
        return;
    assert(sym != NULL);
    r = (struct rel *)alloc(sizeof(struct rel), 0);
    r->n = rel[isect];
    r->a = addr - bs[isect];
    r->s = sym;
    r->z = size;
    r->p = pcrel;
    rel[isect] = r;
    nrel[isect]++;
}

static unsigned
relout(is)
int is;
{
    struct rel *r;
    unsigned x;

    for (r = rel[is]; r; r = r->n) {
        output(4, r->a);
        x = r->s->type;
        if (x == 0)
            x = 0x08000000 | r->s->id;
        x |= (r->z & ~1) << 24;
        if (r->p)
            x |= 0x01000000;
        output(4, x);
    }
    return nrel[is] * 8;
}

static
endsect(is)
int is;
{
    unsigned x;
    int i;

    i = isect;
    isect = is;
    x = ((pcs[isect] + 0xf) & ~0xf) - pcs[isect];
    emitn(x, 0);
    isect = i;
}

static
fixsym()
{
    struct sym *s;
    int x;

    for (s = stlist; s; s = s->next) {
        x = s->type == T_BSS ? 2 : s->type == T_DATA ? 1 : 0;
        if (x)
            s->val += bs[x];
    }
}

static
write()
{
    unsigned i;

    /* Write a.out header */
    output(4, 0x0107);
    for (i = 0; i < 3; i++)
        output(4, pcs[i] - bs[i]);
    output(4, symid * 16);
    output(4, 0);
    for (i = 0; i < 2; i++)
        output(4, nrel[i] * 8);
    /* Write .text and .data */
    for (i = 0; i < bs[2]; i++)
        output(1, bf[i]);
    /* Write relocation bits */
    for (i = 0; i < 2; i++)
        relout(i);
    /* Write symbol table */
    symout();
}

static unsigned
symout()
{
    struct sym *s;
    unsigned n, o, x;

    n = 0;
    o = 4;
    for (s = stlist; s; s = s->next) {
        if (isdigit(*s->key))
            continue;
        for (x = 0; x < s->len; x++)
            output(1, s->key[x]);
        while (x++ < 8)
            output(1, 0);
        x = s->type;
        if (x == 0 || s->ex)
            x++;
        output(1, x);
        output(1, s->t2);
        output(2, 0);
        output(4, s->val);
        o += s->len + 1;
        n += 16;
    }
    return n;
}

static int
setcj(tk)
unsigned tk;
{
    int x, r;

    x = opsfx[tk];
    r = (x & 0x80) ? -1 : (x & 0x40) ? 0 : 1;
    return r;
}

static int
doreg(o)
struct op1 *o;
{
    unsigned x;

    assert(tk == REG);
    x = tk_val >> 3;
    if (x < 3) {
        o->t = OT_R;
        o->a = 1 << x;
    } else {
        o->t = OT_X;
        o->a = x - 3;
    }
    o->c = tk_val & 7;
    return 0;
}

static int
isbyte(b, x)
int b;
unsigned x;
{
    int r;

    assert(b == 2 || b == 4);
    if (b == 2)
        r = (short)x >= -128 && (short)x <= 127;
    else
        r = (int)x >= -128 && (int)x <= 127;
    return r;
}

static int
setlab(opt)
int opt;
{
    int n;

    if (tk_val < 0 || tk_val > 9)
        return E_MISC;
    n = lab[tk_val];
    if (opt != 0) {
        n++;
        if (opt == 2)
            lab[tk_val] = n;
    }
    if (n > 9999999)
        return E_MISC;
    sprintf(tk_str, "%d%07d", tk_val, n);
    tk_len = 8;
    return 0;
}

static
emitn(n, x)
int n;
unsigned x;
{
    while (n-- > 0)
        emit(1, x);
}

static
emit(n, x)
int n;
unsigned x;
{
    unsigned char *p;

    assert(n == 1 || n == 2 || n == 4);
    if (pass) {
        if (sect != T_BSS) {
            p = bf + pcs[isect];
            *p++ = x & 0xff;
            if (n >= 2)
                *p++ = x >> 8 & 0xff;
            if (n == 4) {
                *p++ = x >> 16 & 0xff;
                *p++ = x >> 24 & 0xff;
            }
        } else if (x != 0)
            error(E_BSS);
    }
    pcs[isect] += n;
}

static
output(n, x)
int n;
unsigned x;
{
    assert(n == 1 || n == 2 || n == 4);
    if (pass) {
        putchar(x & 0xff);
        if (n >= 2)
            putchar(x >> 8 & 0xff);
        if (n == 4) {
            putchar(x >> 16 & 0xff);
            putchar(x >> 24 & 0xff);
        }
    }
}

static char *
alloc(size, clr)
unsigned size;
int clr;
{
    char *p;

    p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "as: No core\n");
        exit(1);
    }
    if (clr)
        bzero(p, size);
    return p;
}

static
error(e)
int e;
{
    fprintf(stderr, "%s:%lu: %s\n", source, line, errmsg[e]);
    errs++;
}

static int
bzero(d, n)
char *d;
unsigned n;
{
    while (n--)
        *d++ = 0;
}

static int
bcopy(s, d, n)
char *s;
char *d;
unsigned n;
{
    while (n--)
        *d++ = *s++;
}

/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2007 Robert Nordier.  All rights reserved. */

#include "optab.h"

#define size(x) (sizeof(x)/sizeof((x)[0]))

#define SBAD	00000
#define SES	00030
#define SFS	00034
#define SADD	00050
#define SROL	00060
#define STEST	00070
#define SXCHG	00071
#define SIMUL	00075
#define SINC	00100
#define SCALL	00102
#define SLCALL	00103
#define SJMP	00104
#define SLJMP	00105
#define SPUSH	00106
#define SPOP	00107
#define SSLDT	00110
#define SSTR	00111
#define SSGDT	00120
#define SSMSW	00124
#define SDAA	00130
#define SPUSHA	00140
#define SBOUND	00142
#define SINS	00144
#define SLES	00146
#define SLDS	00146
#define SJO	00150
#define SCBW	00170
#define SNOP	00172
#define SMOV	00200
#define SLEA	00201
#define SXLAT	00204
#define SENTER	00210
#define SRET	00212
#define SINT	00215
#define SAAM	00220
#define SIN	00222
#define SOUT	00223
#define SLOOPNZ 00224
#define SLOCK	00230
#define SFADD	00250
#define SFADDS	00260
#define SFADDL	00270
#define SFLD	00300
#define SFLDS	00310
#define SFLDL	00320
#define SFNSTSW 00327
#define SFCHS	00330
#define SFNOP	00332
#define SFUCO	00333
#define SFTST	00334
#define SFLD1	00340
#define SF2XM1	00350
#define SFPREM	00360
#define SFIADD	00370
#define SFIADDL 00400
#define SFILD	00410
#define SFILDL	00420
#define SFFREE	00430
#define SFFREEP 00431
#define SFNCLEX 00436
#define SFADDP	00440
#define SFCOMPP 00443
#define SLAR	00450
#define SCLTS	00452
#define SXADD	00453
#define SINVD	00454
#define SSHLD	00456
#define SSHRD	00457
#define SSETO	00460
#define SMOVZB	00500
#define SMOVSB	00502
#define SBT	00504
#define SBSF	00510
#define SLFS	00512
#define SLGS	00513
#define SLSS	00514
#define SCMPX	00515
#define SBSWAP	00516
#define SUD2	00517
#define SWRMSR	00520
#define SSYSENT 00524
#define SCPUID	00526
#define SRSM	00527
#define SCMOVO	00530
#define SFCMOVB 00550
#define SFCMOVN 00554
#define SFUCOMI 00560
#define SFCOMI	00561
#define SFUCOMP 00562
#define SFCOMIP 00563
#define SCBTW	00564
#define SCWTD	00566
#define SJCXZ	00570

#define SB	0001
#define AZ	0006
#define Ew	0010
#define EB	0011
#define EW	0012
#define EF	0015
#define EZ	0016
#define EC	0017
#define IB	0021
#define IW	0022
#define IZ	0026
#define IC	0027
#define PB	0031
#define PZ	0036
#define R70S	0043
#define R70Z	0046
#define R70C	0047
#define R30S	0053
#define R07B	0061
#define R07W	0062
#define R07L	0064
#define R07Z	0066
#define Rc	0071
#define Rd	0072
#define Rt	0073
#define R0W	0102
#define R0F	0105
#define R0Z	0106
#define R0C	0107
#define R1B	0111
#define R2W	0122
#define R4S	0143
#define R5S	0153
#define M70	0200
#define M30	0201
#define M07	0202
#define M17	0203
#define M16	0204
#define M10	0205
#define M02	0206
#define M01	0207
#define X0	0220
#define X1	0221
#define X2	0222

static struct tab tab[] = {
	{0x39, 0x00, SADD,	   {M70, X0, EC, R70C}},
	{0x39, 0x02, SADD,	   {M70, X0, R70C, EC}},
	{0x39, 0x04, SADD,	   {M70, R0C, IC}},
	{0x19, 0x06, SPUSH,	   {M01, R30S}},
	{0x18, 0x26, SES,	   {M30}},
	{0x18, 0x27, SDAA,	   {M30}},
	{0x0f, 0x40, SINC,	   {M10, R07Z}},
	{0x0f, 0x50, SPUSH,	   {M10, R07Z}},
	{0x01, 0x60, SPUSHA,	   {M01}},
	{0x01, 0x62, SBOUND,	   {M01, X0, EZ, R70Z}},
	{0x01, 0x64, SFS,	   {M01}},
	{0x00, 0x68, SPUSH,	   {IZ}},
	{0x00, 0x6a, SPUSH,	   {SB}},
	{0x00, 0x69, SIMUL,	   {X0, R70C, EC, IW}},
	{0x00, 0x6b, SIMUL,	   {X0, R70C, EC, SB}},
	{0x03, 0x6c, FX | SINS,    {M02}},
	{0x0f, 0x70, SJO,	   {M17, PB}},
	{0x01, 0x80, FX | SADD,    {X0, M70, EC, IC}},
	{0x00, 0x82, FX | SADD,    {X0, M70, EC, IB}},
	{0x00, 0x83, FX | SADD,    {X0, M70, EC, SB}},
	{0x03, 0x84, STEST,	   {M02, X0, EC, R70C}},
	{0x01, 0x88, SMOV,	   {X0, EC, R70C}},
	{0x01, 0x8a, SMOV,	   {X0, R70C, EC}},
	{0x00, 0x90, SNOP,	   },
	{0x07, 0x90, SXCHG,	   {R07Z, R0Z}},
	{0x00, 0x98, SCBTW,	   {X1}},
	{0x00, 0x99, SCWTD,	   {X1}},
	{0x00, 0x9a, SLCALL,	   {IZ, IW}},
	{0x07, 0x98, SCBW,	   {M07}},
	{0x01, 0xa0, SMOV,	   {R0C, AZ}},
	{0x01, 0xa2, SMOV,	   {AZ, R0C}},
	{0x01, 0xa8, STEST,	   {R0C, IC}},
	{0x0f, 0xa0, FX | SMOV,    {M16}},
	{0x07, 0xb0, SMOV,	   {R07B, IB}},
	{0x07, 0xb8, SMOV,	   {R07W, IZ}},
	{0x08, 0xc2, SRET,	   {M10, IW}},
	{0x08, 0xc3, SRET,	   {M10}},
	{0x00, 0xc8, SENTER,	   {IW, IB, X2}},
	{0x00, 0xcd, SINT,	   {IB}},
	{0x07, 0xc8, SENTER,	   {M07}},
	{0x01, 0xd4, SAAM,	   {M01, IB}},
	{0x00, 0xd7, SXLAT,	   },
	{0x00, 0xe3, SJCXZ,	   {X1, PB}},
	{0x03, 0xe0, SLOOPNZ,	   {M07, PB}},
	{0x01, 0xe4, SIN,	   {R0C, IB}},
	{0x01, 0xe6, SOUT,	   {IB, R0C}},
	{0x00, 0xe8, SCALL,	   {PZ}},
	{0x00, 0xe9, SJMP,	   {PZ}},
	{0x00, 0xea, SLJMP,	   {IZ, IW}},
	{0x00, 0xeb, SJMP,	   {PB}},
	{0x01, 0xec, SIN,	   {R0C, R2W}},
	{0x01, 0xee, SOUT,	   {R2W, R0C}},
	{0x0f, 0xf0, SLOCK,	   {M17}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tab8c[] = {
	{0xcf, 0x30, SBAD,	   },
	{0x3f, 0xc0, SMOV,	   {EZ, R70S}},
	{0xff, 0x00, FC | SMOV,    {EZ, R70S}}
};

static struct tab tab8d[] = {
	{0x3f, 0xc0, SBAD,	   },
	{0xff, 0x00, SLEA,	   {R70C, EC}}
};

static struct tab tab8e[] = {
	{0xcf, 0x30, SBAD,	   },
	{0x3f, 0xc0, SMOV,	   {R70S, EZ}},
	{0xff, 0x00, FC | SMOV,    {R70S, EZ}}
};

static struct tab tab8f[] = {
	{0x3f, 0xc0, SPOP,	   {EC}},
	{0xff, 0x00, FC | SPOP,    {EC}}
};

static struct tab tabc0[] = {
	{0xc7, 0x30, SBAD,	   },
	{0xff, 0x00, FX | SROL,    {M70, EC, IB}}
};

static struct tab tabc4[] = {
	{0x3f, 0xc0, SBAD,	   },
	{0xff, 0x00, SLES,	   {R70Z, EC}}
};

static struct tab tabc5[] = {
	{0x3f, 0xc0, SBAD,	   },
	{0xff, 0x00, SLDS,	   {R70Z, EC}}
};

static struct tab tabc6[] = {
	{0xc7, 0x00, FX | SMOV,    {EC, IC}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tabd0[] = {
	{0xc7, 0x30, SBAD,	   },
	{0xff, 0x00, FX | SROL,    {M70, EC}}
};

static struct tab tabd2[] = {
	{0xc7, 0x30, SBAD,	   },
	{0xff, 0x00, FX | SROL,    {M70, EC, R1B}}
};

static struct tab tabd8[] = {
	{0x0f, 0xd0, SFADD,	   {M70, EF}},
	{0x3f, 0xc0, SFADD,	   {M70, R0F, EF}},
	{0xff, 0x00, SFADDS,	   {M70, EC}}
};

static struct tab tabd9[] = {
	{0x00, 0xd0, SFNOP,	   },
	{0x01, 0xe0, SFCHS,	   {M01}},
	{0x01, 0xe4, SFTST,	   {M01}},
	{0x07, 0xe8, SFLD1,	   {M07}},
	{0x07, 0xf0, SF2XM1,	   {M07}},
	{0x07, 0xf8, SFPREM,	   {M07}},
	{0x0f, 0xc0, SFLD,	   {M10, EF}},
	{0x3f, 0xc0, SBAD,	   },
	{0xc7, 0x08, SBAD,	   },
	{0xff, 0x00, SFLDS,	   {M70, EC}}
};

static struct tab tabda[] = {
	{0x00, 0xe9, SFUCO,	   },
	{0x1f, 0xc0, SFCMOVB,	   {M30, R0F, EF}},
	{0x3f, 0xc0, SBAD,	   },
	{0xff, 0x00, SFIADDL,	   {M70, EF}}
};

static struct tab tabdb[] = {
	{0x01, 0xe2, SFNCLEX,	   {M01}},
	{0x1f, 0xc0, SFCMOVN,	   {M30, R0F, EF}},
	{0x07, 0xe8, SFUCOMI,	   {R0F, EF}},
	{0x07, 0xf0, SFCOMI,	   {R0F, EF}},
	{0x3f, 0xc0, SBAD,	   },
	{0xc7, 0x08, SBAD,	   },
	{0xd7, 0x20, SBAD,	   },
	{0xff, 0x00, SFILDL,	   {M70, EF}}
};

static struct tab tabdc[] = {
	{0x0f, 0xd0, SBAD,	   },
	{0x3f, 0xc0, SFADD,	   {M70, EF, R0F}},
	{0xff, 0x00, SFADDL,	   {M70, EF}}
};

static struct tab tabdd[] = {
	{0xc7, 0x08, SBAD,	   },
	{0x0f, 0xf0, SBAD,	   },
	{0x3f, 0xc0, SFFREE,	   {M70, EF}},
	{0xc7, 0x28, SBAD,	   },
	{0xff, 0x00, SFLDL,	   {M70, EF}}
};

static struct tab tabde[] = {
	{0x00, 0xd9, SFCOMPP,	   },
	{0x0f, 0xd0, SBAD,	   },
	{0x3f, 0xc0, SFADDP,	   {M70, EF, R0F}},
	{0xff, 0x00, SFIADD,	   {M70, EF}}
};

static struct tab tabdf[] = {
	{0x00, 0xe0, SFNSTSW,	   {R0W}},
	{0x07, 0xc0, SFFREEP,	   {EF}},
	{0x07, 0xe8, SFUCOMP,	   {R0F, EF}},
	{0x07, 0xf0, SFCOMIP,	   {R0F, EF}},
	{0x3f, 0xc0, SBAD,	   },
	{0xc7, 0x08, SBAD,	   },
	{0xff, 0x00, SFILD,	   {M70, EF}}
};

static struct tab tabf6[] = {
	{0xc7, 0x08, SBAD,	   },
	{0x07, 0xc0, STEST,	   {EC, IC}},
	{0xc7, 0x00, FX | STEST,   {EC, IC}},
	{0x3f, 0xc0, STEST,	   {M70, EC}},
	{0xff, 0x00, FX | STEST,   {M70, EC}}
};

static struct tab tabfe[] = {
	{0x0f, 0xc0, SINC,	   {M70, EC}},
	{0xcf, 0x00, FX | SINC,    {M70, EC}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tabff[] = {
	{0xc7, 0x38, SBAD,	   },
	{0x0f, 0xc0, SINC,	   {M70, EC}},
	{0x07, 0xf0, SINC,	   {M70, EC}},
	{0x37, 0xc8, SBAD,	   },
	{0xcf, 0x00, FX | SINC,    {M70, EC}},
	{0xc7, 0x30, FX | SINC,    {M70, EC}},
	{0xff, 0x00, FJ | SINC,    {M70, EC}}
};

struct tabs tabs[] = {
	{0x00, size(tab),   tab},
	{0x8c, size(tab8c), tab8c},
	{0x8d, size(tab8d), tab8d},
	{0x8e, size(tab8e), tab8e},
	{0x8f, size(tab8f), tab8f},
	{0xc0, size(tabc0), tabc0},
	{0xc1, size(tabc0), tabc0},
	{0xc4, size(tabc4), tabc4},
	{0xc5, size(tabc5), tabc5},
	{0xc6, size(tabc6), tabc6},
	{0xc7, size(tabc6), tabc6},
	{0xd0, size(tabd0), tabd0},
	{0xd1, size(tabd0), tabd0},
	{0xd2, size(tabd2), tabd2},
	{0xd3, size(tabd2), tabd2},
	{0xd8, size(tabd8), tabd8},
	{0xd9, size(tabd9), tabd9},
	{0xda, size(tabda), tabda},
	{0xdb, size(tabdb), tabdb},
	{0xdc, size(tabdc), tabdc},
	{0xdd, size(tabdd), tabdd},
	{0xde, size(tabde), tabde},
	{0xdf, size(tabdf), tabdf},
	{0xf6, size(tabf6), tabf6},
	{0xf7, size(tabf6), tabf6},
	{0xfe, size(tabfe), tabfe},
	{0xff, size(tabff), tabff}
};

int tabsz = size(tabs);

static struct tab tab0f[] = {
	{0x01, 0x02, SLAR,	   {M01, X0, R70Z, EW}},
	{0x00, 0x06, SCLTS,	   },
	{0x01, 0x08, SINVD,	   {M01}},
	{0x00, 0x0b, SUD2,	   },
	{0x03, 0x30, SWRMSR,	   {M07}},
	{0x01, 0x34, SSYSENT,	   {M01}},
	{0x0f, 0x40, SCMOVO,	   {M17, X0, R70Z, EZ}},
	{0x0f, 0x80, SJO,	   {M17, PZ}},
	{0x0f, 0x90, SSETO,	   {M17, X0, EB}},
	{0x00, 0xa0, SPUSH,	   {R4S}},
	{0x00, 0xa1, SPOP,	   {R4S}},
	{0x00, 0xa2, SCPUID,	   },
	{0x18, 0xa3, SBT,	   {M30, X0, EC, R70C}},
	{0x00, 0xa4, SSHLD,	   {X0, EZ, R70Z, IB}},
	{0x00, 0xa5, SSHLD,	   {X0, EZ, R70Z, R1B}},
	{0x00, 0xa8, SPUSH,	   {R5S}},
	{0x00, 0xa9, SPOP,	   {R5S}},
	{0x00, 0xaa, SRSM,	   },
	{0x00, 0xac, SSHRD,	   {X0, EZ, R70Z, IB}},
	{0x00, 0xad, SSHRD,	   {X0, EZ, R70Z, R1B}},
	{0x00, 0xaf, SIMUL,	   {X0, R70Z, EZ}},
	{0x01, 0xb0, SCMPX,	   {X0, EC, R70C}},
	{0x01, 0xb6, FC | SMOVZB,  {M01, X0, R70Z, Ew}},
	{0x01, 0xbc, SBSF,	   {M01, X0, R70Z, EZ}},
	{0x01, 0xbe, FC | SMOVSB,  {M01, X0, R70Z, Ew}},
	{0x01, 0xc0, SXADD,	   {X0, EC, R70C}},
	{0x07, 0xc8, SBSWAP,	   {R07Z}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tab0f00[] = {
	{0xc7, 0x08, SSTR,	   {EZ}},
	{0xcf, 0x30, SBAD,	   },
	{0xff, 0x00, SSLDT,	   {M70, EW}}
};

static struct tab tab0f01[] = {
	{0xc7, 0x28, SBAD,	   },
	{0xd7, 0x20, SSMSW,	   {M30, EW}},
	{0x3f, 0xc0, SBAD,	   },
	{0xff, 0x00, SSGDT,	   {M70, EC}}
};

static struct tab tab0f20[] = {
	{0x07, 0xc0, SMOV,	   {R07L, Rc}},
	{0x0f, 0xd0, SMOV,	   {R07L, Rc}},
	{0x07, 0xe0, SMOV,	   {R07L, Rc}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tab0f21[] = {
	{0x1f, 0xc0, SMOV,	   {R07L, Rd}},
	{0x0f, 0xf0, SMOV,	   {R07L, Rd}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tab0f22[] = {
	{0x07, 0xc0, SMOV,	   {Rc, R07L}},
	{0x0f, 0xd0, SMOV,	   {Rc, R07L}},
	{0x07, 0xe0, SMOV,	   {Rc, R07L}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tab0f23[] = {
	{0x1f, 0xc0, SMOV,	   {Rd, R07L}},
	{0x0f, 0xf0, SMOV,	   {Rd, R07L}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tab0f24[] = {
	{0x07, 0xd8, SMOV,	   {R07L, Rt}},
	{0x1f, 0xe0, SMOV,	   {R07L, Rt}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tab0f26[] = {
	{0x07, 0xd8, SMOV,	   {Rt, R07L}},
	{0x1f, 0xe0, SMOV,	   {Rt, R07L}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tab0fb2[] = {
	{0x3f, 0xc0, SBAD,	   },
	{0xff, 0x00, SLSS,	   {R70Z, EZ}}
};

static struct tab tab0fba[] = {
	{0x1f, 0xe0, SBT,	   {M30, EZ, IB}},
	{0xdf, 0x20, FC | SBT,	   {M30, EZ, IB}},
	{0xff, 0x00, SBAD,	   }
};

static struct tab tab0fb4[] = {
	{0x3f, 0xc0, SBAD,	   },
	{0xff, 0x00, SLFS,	   {R70Z, EZ}}
};

static struct tab tab0fb5[] = {
	{0x3f, 0xc0, SBAD,	   },
	{0xff, 0x00, SLGS,	   {R70Z, EZ}}
};

struct tabs tabs0f[] = {
	{0x00, size(tab0f),   tab0f},
	{0x00, size(tab0f00), tab0f00},
	{0x01, size(tab0f01), tab0f01},
	{0x20, size(tab0f20), tab0f20},
	{0x21, size(tab0f21), tab0f21},
	{0x22, size(tab0f22), tab0f22},
	{0x23, size(tab0f23), tab0f23},
	{0x24, size(tab0f24), tab0f24},
	{0x26, size(tab0f26), tab0f26},
	{0xb2, size(tab0fb2), tab0fb2},
	{0xb4, size(tab0fb4), tab0fb4},
	{0xb5, size(tab0fb5), tab0fb5},
	{0xba, size(tab0fba), tab0fba}
};

int tabs0fz = size(tabs0f);

char *str[] = {
	"bad",	       "st(%d)",     "cr%d",	   "dr%d",
	"tr%d",        "?",	     "?",	   "?",
	"al",	       "cl",	     "dl",	   "bl",
	"ah",	       "ch",	     "dh",	   "bh",
	"ax",	       "cx",	     "dx",	   "bx",
	"sp",	       "bp",	     "si",	   "di",
	"es",	       "cs",	     "ss",	   "ds",
	"fs",	       "gs",	     "?",	   "?",
	"eax",	       "ecx",	     "edx",	   "ebx",
	"esp",	       "ebp",	     "esi",	   "edi",
	"add",	       "or",	     "adc",	   "sbb",
	"and",	       "sub",	     "xor",	   "cmp",
	"rol",	       "ror",	     "rcl",	   "rcr",
	"shl",	       "shr",	     "?",	   "sar",
	"test",        "xchg",	     "not",	   "neg",
	"mul",	       "imul",	     "div",	   "idiv",

	"inc",	       "dec",	     "call",	   "lcall",
	"jmp",	       "ljmp",	     "push",	   "pop",
	"sldt",        "str",	     "lldt",	   "ltr",
	"verr",        "verw",	     "?",	   "?",
	"sgdt",        "sidt",	     "lgdt",	   "lidt",
	"smsw",        "?",	     "lmsw",	   "invlpg",
	"daa",	       "das",	     "aaa",	   "aas",
	"bt",	       "bts",	     "btr",	   "btc",
	"pusha",       "popa",	     "bound",	   "arpl",
	"ins",	       "outs",	     "les",	   "lds",
	"jo",	       "jno",	     "jc",	   "jnc",
	"jz",	       "jnz",	     "jna",	   "ja",
	"js",	       "jns",	     "jpe",	   "jpo",
	"jl",	       "jnl",	     "jng",	   "jg",
	"?",	       "?",	     "nop",	   "wait",
	"pushf",       "popf",	     "sahf",	   "lahf",

	"mov",	       "lea",	     "movs",	   "cmps",
	"xlatb",       "stos",	     "lods",	   "scas",
	"enter",       "leave",      "ret",	   "lret",
	"int3",        "int",	     "into",	   "iret",
	"aam",	       "aad",	     "in",	   "out",
	"loopnz",      "loopz",      "loop",	   "?",
	"lock",        "int1",	     "repnz",	   "repz",
	"hlt",	       "cmc",	     "?",	   "?",
	"clc",	       "stc",	     "cli",	   "sti",
	"cld",	       "std",	     "?",	   "?",
	"fadd",        "fmul",	     "fcom",	   "fcomp",
	"fsub",        "fsubr",      "fdiv",	   "fdivr",
	"fadds",       "fmuls",      "fcoms",	   "fcomps",
	"fsubs",       "fsubrs",     "fdivs",	   "fdivrs",
	"faddl",       "fmull",      "fcoml",	   "fcompl",
	"fsubl",       "fsubrl",     "fdivl",	   "fdivrl",

	"fld",	       "fxch",	     "fst",	   "fstp",
	"fldenv",      "fldcw",      "fnstenv",    "fnstcw",/*XXX*/
	"flds",        "?",	     "fsts",	   "fstps",
	"fldenv",      "fldcw",      "fnstenv",    "fnstcw",
	"fldl",        "?",	     "fstl",	   "fstpl",
	"frstor",      "?",	     "fnsave",	   "fnstsw",
	"fchs",        "fabs",	     "fnop",	   "fucompp",
	"ftst",        "fxam",	     "?",	   "?",
	"fld1",        "fldl2t",     "fldl2e",	   "fldpi",
	"fldlg2",      "fldln2",     "fldz",	   "bad",
	"f2xm1",       "fyl2x",      "fptan",	   "fpatan",
	"fxtract ",    "fprem1",     "fdecstp",    "fincstp",
	"fprem",       "fyl2xp1",    "fsqrt",	   "fsincos",
	"frndint",     "fscale",     "fsin",	   "fcos",
	"fiadd",       "fimul",      "ficom",	   "ficomp",
	"fisub",       "fisubr",     "fidiv",	   "fidivr",

	"fiaddl",      "fimull",     "ficoml",	   "ficompl",
	"fisubl",      "fisubrl",    "fidivl",	   "fidivrl",
	"fild",        "?",	     "fist",	   "fistp",
	"fbld",        "fildll",     "fbstp",	   "fistpll",
	"fildl",       "?",	     "fistl",	   "fistpl",
	"?",	       "fldt",	     "?",	   "fstpt",
	"ffree",       "ffreep",     "fst",	   "fstp",
	"fucom",       "fucomp",     "fnclex",	   "fninit",
	"faddp",       "fmulp",      "?",	   "fcompp",
	"fsubp",       "fsubrp",     "fdivp",	   "fdivrp",
	"lar",	       "lsl",	     "clts",	   "xadd",
	"invd",        "wbinvd",     "shld",	   "shrd",
	"seto",        "setno",      "setc",	   "setnc",
	"setz",        "setnz",      "setna",	   "seta",
	"sets",        "setns",      "setpe",	   "setpo",
	"setl",        "setnl",      "setng",	   "setg",

	"movzb",       "movzw",      "movsb",	   "movsw",
	"bt",	       "bts",	     "btr",	   "btc",
	"bsf",	       "bsr",	     "lfs",	   "lgs",
	"lss",	       "cmpxchg",    "bswap",	   "ud2",
	"wrmsr",       "rdtsc",      "rdmsr",	   "rdpmc",
	"sysenter",    "sysexit",    "cpuid",	   "rsm",
	"cmovo",       "cmovno",     "cmovc",	   "cmovnc",
	"cmovz",       "cmovnz",     "cmovna",	   "cmova",
	"cmovs",       "cmovns",     "cmovpe",	   "cmovpo",
	"cmovl",       "cmovnl",     "cmovng",	   "cmovg",
	"fcmovb",      "fcmove",     "fcmovbe",    "fcmovu",
	"fcmovnb",     "fcmovne",    "fcmovnbe",   "fcmovnu",
	"fucomi",      "fcomi",      "fucomip",    "fcomip",
	"cbtw",        "cwtl",	     "cwtd",	   "cltd",
	"jcxz",        "jecxz",      "?",	   "?"
};

/* Copyright (c) 1995-2007 Robert Nordier.  All rights reserved. */

#define OT_I    1
#define OT_M    2
#define OT_R    4
#define OT_E    6
#define OT_X    8

#define OA_0    0
#define OA_S    1
#define OA_C    2
#define OA_D    3
#define OA_T    4
#define OA_F    5
#define OA_M    6
#define OA_X    7

#define OZ_1    0x01
#define OZ_2    0x02
#define OZ_4    0x04
#define OZ_8    0x08
#define OZ_X    0x20
#define OZ_T    0x40
#define OZ_V    (OZ_4|OZ_2)

#define OX_Z    0x8000  /* EA operand */
#define OX_Y    0x4000  /* unused */
#define OX_X    0x2000  /* 0x0f prefix */
#define OX_W    0x1000  /* +2 optimization */
#define OX_V    0x800   /* variable-size */
#define OX_U    0x400   /* FP int processing */
#define OX_C    0x300   /* encoding type */

struct optbl {
    unsigned char o0;
    unsigned char o1;
    struct {
        unsigned char ot;
        unsigned char oa;
        unsigned char om;
    } op[3];
    unsigned short ox;
};

extern struct optbl optbl[];
extern unsigned short opidx[];
extern char opsfx[];

/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/* machine dependent stuff for core files */
#define TXTRNDSIZ 8192L
#define stacktop(siz) (0x10000L)
#define stackbas(siz) (0x10000L-siz)

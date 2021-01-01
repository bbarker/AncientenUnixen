/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

# include "stdio.h"
# include "refer..c"
FILE *in = stdin;
int endpush = 0;
int labels = 0;
int keywant = 0;
int sort = 0;
int bare = 0;
int authrev = 0;
char *smallcaps = "";
char *keystr = "AD";
int nmlen = 0, dtlen = 0;
char *data[NSERCH];
char **search = data;
int refnum = 0;
char reftext[NRFTXT];
char *reftable[NRFTBL];
char *rtp = reftext;
int sep = '\n';
char tfile[NTFILE];
FILE *fo = stdout;
FILE *ftemp = stdout;
char ofile[NTFILE];
char gfile[NTFILE];
char hidenam[NTFILE];
char *Ifile = "standard input";
int Iline = 0;

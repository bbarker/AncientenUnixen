/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

extern int xxindent, xxval, newflag, xxmaxchars, xxbpertab;
extern int xxlineno;		/* # of lines already output */
#define xxtop	100		/* max size of xxstack */
extern int xxstind, xxstack[xxtop], xxlablast, xxt;
struct node
	{int op;
	char *lit;
	struct node *left;
	struct node *right;
	};

/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

# include "refer..c"
getq(v)
	char *v[];
{
# define BSIZ 250
static char buff[BSIZ];
static int eof = 0;
extern char *sinput;
char *p;
int c, n = 0, las = 0;
if (eof) return(-1);
p = buff;
while ( (c = (sinput ? *sinput++ : getchar()) ) > 0)
	{
	if (c== '\n')
		break;
	if (isalpha(c) || isdigit(c))
		{
		if (las==0)
			{
			v[n++] = p;
			las=1;
			}
		if (las++ <= 6)
			*p++ = c;
		}
	else
		{
		if (las>0)
			*p++ = 0;
		las=0;
		}
	}
*p=0;
assert(p<buff+BSIZ);
if (sinput==0 && c<= 0) eof=1;
# if D1
fprintf(stderr, "no. keys %d\n",n);
for(c=0; c<n; c++)
 fprintf(stderr, "keys X%sX\n", v[c]);
# endif
return(n);
}

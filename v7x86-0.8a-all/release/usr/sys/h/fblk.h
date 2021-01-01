/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

struct fblk
{
	int    	df_nfree;
	daddr_t	df_free[NICFREE];
};

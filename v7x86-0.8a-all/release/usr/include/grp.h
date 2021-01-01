/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

struct	group { /* see getgrent(3) */
	char	*gr_name;
	char	*gr_passwd;
	int	gr_gid;
	char	**gr_mem;
};

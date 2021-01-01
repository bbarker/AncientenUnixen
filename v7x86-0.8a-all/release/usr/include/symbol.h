/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * Structure of a symbol table entry
 */

struct	symbol {
	char	sy_name[8];
	char	sy_type;
	int	sy_value;
};

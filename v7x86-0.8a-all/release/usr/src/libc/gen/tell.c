/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * return offset in file.
 */

long	lseek();

long tell(f)
{
	return(lseek(f, 0L, 1));
}

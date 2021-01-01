/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

double
fabs(arg)
double arg;
{

	if(arg < 0.)
		arg = -arg;
	return(arg);
}

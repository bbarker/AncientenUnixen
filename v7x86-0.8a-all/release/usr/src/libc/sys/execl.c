/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 1999 Robert Nordier.  All rights reserved. */

extern char **environ;

int
execl(name,args)
char *name;
{
	return execve(name, &args, environ);
}

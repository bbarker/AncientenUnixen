/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 1999 Robert Nordier.  All rights reserved. */

extern char **environ;

int
execv(path, argv)
char *path;
char **argv;
{
	return execve(path, argv, environ);
}

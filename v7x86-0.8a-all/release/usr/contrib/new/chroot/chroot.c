/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 2006 Robert Nordier.  All rights reserved. */

#include <stdio.h>

int
main(argc, argv)
char **argv;
{
	if (argc < 3) {
		fprintf(stderr, "usage: chroot path command ...\n");
		exit(1);
	}
	if (chroot(argv[1]))
		fprintf(stderr, "chroot failed\n");
	else if (chdir("/"))
		fprintf(stderr, "chdir failed\n");
	else if (execv(argv[2], &argv[2]))
		fprintf(stderr, "execv failed\n");
	return 1;
}

/* V7/x86 source code: see www.nordier.com/v7x86 for details. */
/* Copyright (c) 1998 Robert Nordier.  All rights reserved. */

#include <signal.h>

abort()
{
	kill(getpid(), SIGIOT);
}

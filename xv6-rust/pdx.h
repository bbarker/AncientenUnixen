/*
 * This file contains types and definitions for Portland State University.
 * The contents are intended to be visible in both user and kernel space.
 */

#ifndef PDX_INCLUDE
#define PDX_INCLUDE

#define TRUE 1
#define FALSE 0
#define RETURN_SUCCESS 0
#define RETURN_FAILURE -1

#define NUL 0
#ifndef NULL
#define NULL NUL
#endif  // NULL

#define TPS 1000   // ticks-per-second
#define SCHED_INTERVAL (TPS/100)  // see trap.c

#define NPROC  64  // maximum number of processes -- normally in param.h

#ifdef CS333_P2
#define DEFAULT_UID 0
#define DEFAULT_GID 0
#endif // CS333_P2
#ifdef CS333_P3P4
#define DEFAULT_BUDGET (3*TPS)
#define DEFAULT_PRIORITY 0
#define TICKS_TO_PROMOTE (20*TPS)
#define MAXPRIO 7 /* max prio. 0 <= prio <= MAXPRIO */
#endif // CS333_P3P4
#ifdef CS333_P5
#define DEFAULT_MODE 0755
#endif // CS333_P5

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#endif  // PDX_INCLUDE

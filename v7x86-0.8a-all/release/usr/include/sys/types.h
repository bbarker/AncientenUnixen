/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

typedef	long       	daddr_t;  	/* disk address */
typedef	char *     	caddr_t;  	/* core address */
typedef	unsigned short	ino_t;     	/* i-node number */
typedef	long       	time_t;   	/* a time */
typedef	int        	label_t[6]; 	/* program status */
typedef	short      	dev_t;    	/* device code */
typedef	long       	off_t;    	/* offset in file */
	/* selectors and constructor for device code */
#define	major(x)  	(int)(((unsigned)x>>8))
#define	minor(x)  	(int)(x&0377)
#define	makedev(x,y)	(dev_t)((x)<<8|(y))

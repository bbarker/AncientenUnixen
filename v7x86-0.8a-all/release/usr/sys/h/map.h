/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

struct map
{
	short	m_size;
	unsigned short m_addr;
};

struct map coremap[CMAPSIZ];	/* space for core allocation */
struct map swapmap[SMAPSIZ];	/* space for swap allocation */

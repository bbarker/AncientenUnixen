/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

# if LONG
# define ptr long
# define uptr long
# define getp getl
# define putp putl
# define MONE -1L
extern long getl();
# else
# define ptr int
# define uptr unsigned
# define getp getw
# define putp putw
# define MONE -1
# endif

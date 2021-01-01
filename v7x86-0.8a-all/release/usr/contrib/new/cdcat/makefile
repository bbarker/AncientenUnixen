CFLAGS = -O

all: cdcat

cp: all
	cp cdcat /usr/bin
	cp cdcat.1 /usr/man/man1
	rm cdcat

cmp: all
	cmp cdcat /usr/bin/cdcat
	cmp cdcat.1 /usr/man/man1/cdcat.1
	rm cdcat

cdcat: cdcat.c
	cc $(CFLAGS) -n -s -o cdcat cdcat.c

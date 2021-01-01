CFLAGS = -O

all: ptdisk

cp: all
	cp ptdisk /usr/bin
	cp ptdisk.1 /usr/man/man1
	rm ptdisk

cmp: all
	cmp ptdisk /usr/bin/ptdisk
	cmp ptdisk.1 /usr/man/man1/ptdisk.1
	rm ptdisk

ptdisk: ptdisk.c
	cc $(CFLAGS) -n -s -o ptdisk ptdisk.c

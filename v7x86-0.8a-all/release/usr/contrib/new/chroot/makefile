CFLAGS = -O

all: chroot

cp: all
	cp chroot /usr/bin
	cp chroot.1 /usr/man/man1
	rm chroot

cmp: all
	cmp chroot /usr/bin/chroot
	cmp chroot.1 /usr/man/man1/chroot.1
	rm chroot

chroot: chroot.c
	cc $(CFLAGS) -n -s -o chroot chroot.c

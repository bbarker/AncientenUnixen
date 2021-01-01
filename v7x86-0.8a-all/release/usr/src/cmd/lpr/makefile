all: lpr lpd

cp: all
	cp lpr /bin/lpr
	cp lpd /usr/lib/lpd
	rm lpr lpd

cmp: all
	cmp lpr /bin/lpr
	cmp lpd /usr/bin/lpd
	rm lpr lpd

lpr:	lpr.c spool.c
	cc -n -s -O lpr.c -o lpr

lpd:	lpd.c daemon.c daemon0.c chrtab.c
	cc -n -s -O lpd.c chrtab.c -o lpd

clean:
	rm -f lpr lpd *.o

all: cp

cp: compress
	cp compress /usr/bin
	rm -f /usr/bin/uncompress /usr/bin/zcat
	ln /usr/bin/compress /usr/bin/uncompress
	ln /usr/bin/compress /usr/bin/zcat
	cp compress.1 /usr/man/man1/compress.1
	rm -f /usr/man/man1/uncompress.1 /usr/man/man1/zcat.1
	ln /usr/man/man1/compress.1 /usr/man/man1/uncompress.1
	ln /usr/man/man1/compress.1 /usr/man/man1/zcat.1
	rm -f compress

compress : compress.c
	cc -O -DUSERMEM=4194304 -o compress compress.c

clean:
	rm -f compress

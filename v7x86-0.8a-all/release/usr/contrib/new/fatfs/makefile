CFLAGS = -O

all: fatfs

cp: all
	rm -f /usr/bin/fatdir /usr/bin/fatget /usr/bin/fatput
	cp fatfs /usr/bin/fatdir
	ln /usr/bin/fatdir /usr/bin/fatget
	ln /usr/bin/fatdir /usr/bin/fatput
	rm -f /usr/man/man1/fatdir.1
	rm -f /usr/man/man1/fatget.1
	rm -f /usr/man/man1/fatput.1
	cp fatfs.1 /usr/man/man1/fatdir.1
	ln /usr/man/man1/fatdir.1 /usr/man/man1/fatget.1
	ln /usr/man/man1/fatdir.1 /usr/man/man1/fatput.1
	@make clean

cmp: all
	cmp fatfs /usr/bin/fatdir
	cmp fatfs.1 /usr/man/man1/fatdir.1
	@make clean

fatfs: fatfs.o mem.o
	cc -n -s -o fatfs fatfs.o mem.o

clean:
	rm -f fatfs fatfs.o mem.o

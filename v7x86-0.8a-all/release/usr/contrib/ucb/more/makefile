CFLAGS = -O -I../include

all: more

cp: all
	cp more /usr/ucb
	cp more.help /usr/lib
	cp more.1 /usr/man/man1
	@make clean

more: more.o regex.o
	cc -n -s -o more more.o regex.o -ltermlib

clean:
	rm -f more *.o

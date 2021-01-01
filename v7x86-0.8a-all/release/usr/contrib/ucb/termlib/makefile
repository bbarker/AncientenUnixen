CFLAGS=	-O -I../include -DCM_N -DCM_GT -DCM_B -DCM_D

all: termlib.a

cp: all
	cp termlib.a /usr/lib/libtermlib.a
	cp termlib.3 /usr/man/man3
	@make clean

termlib.a: termcap.o tgoto.o tputs.o
	ar cr termlib.a termcap.o tgoto.o tputs.o

clean:
	rm -f termlib.a *.o

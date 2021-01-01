unix:	mch.o c.o ../sys/LIB1 ../dev/LIB2
	ld -o unix -X -n -T 7fc00000 mch.o c.o ../sys/LIB1 ../dev/LIB2 -lem

all:
	cd ../sys; cc -c -O *.c; ./mklib; rm *.o
	cd ../dev; cc -c -O *.c; ./mklib; rm *.o

mch.o:	mch.s
	as -o mch.o mch.s

allsystems: mkconf
	./mkconf <hd0conf
	make unix
	mv unix hd0unix
	./mkconf <hd1conf
	make unix
	mv unix hd1unix
	./mkconf <md0conf
	make unix
	mv unix md0unix

mkconf: mkconf.c
	cc -O -o mkconf mkconf.c

clean:
	rm -f hd0unix hd1unix md0unix
	rm -f unix mch.o c.o c.c mkconf
	rm -f ../sys/LIB1 ../sys/*.o ../dev/LIB2 ../dev/*.o

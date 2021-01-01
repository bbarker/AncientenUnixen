CFLAGS = -O

OBJS = as.o lookup.o mntbl.o optbl.o potbl.o rmtbl.o token.o

all:	as

cmp:	as
	cmp as /bin/as
	rm as *.o

cp:	as
	cp as /bin/as
	rm as *.o

as: $(OBJS)
	cc $(CFLAGS) -o as $(OBJS)

clean:
	rm -f as $(OBJS)

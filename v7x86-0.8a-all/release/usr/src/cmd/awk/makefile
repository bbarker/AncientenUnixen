CFLAGS=-O
YFLAGS=-d

all:	awk

cp:	awk
	cp awk /bin/awk
	make clean

cmp:	awk
	cmp awk /bin/awk
	make clean

FILES=awk.lx.o b.o main.o token.o tran.o lib.o run.o parse.o proctab.o freeze.o
SOURCE=awk.def awk.g.y awk.lx.l b.c lib.c main.c parse.c \
	proc.c freeze.c\
	run.c token.c tran.c

awk:	$(FILES) awk.g.o
	cc -s $(CFLAGS) -o awk awk.g.o $(FILES) -lm

y.tab.h:	awk.g.o

awk.h:	y.tab.h
	-cmp -s y.tab.h awk.h || cp y.tab.h awk.h

$(FILES):	awk.h awk.def

token.c:	awk.h
	ed - <tokenscript
	rm temp

src:	$(SOURCE) test.a tokenscript makefile README
	cp $? /usr/src/cmd/awk
	touch src

profile:	awk.g.o $(FILES) mon.o
	cc -p awk.g.o $(FILES) mon.o -lm

find:
	egrep -n "$(PAT)" *.[ylhc] awk.def

list:
	-pr $(SOURCE) makefile tokenscript README EXPLAIN

lint:
	lint -spu b.c main.c token.c tran.c run.c lib.c parse.c -lm |\
		egrep -v '^(error|free|malloc)'

proctab.c:	proc
	./proc >proctab.c

proc:	awk.h proc.o token.o
	cc -o proc proc.o token.o

clean:
	rm -f awk *.o t.* *temp* *.out *junk* y.tab.* awk.h proc proctab.c

diffs:
	-for i in $(SOURCE); do echo $$i:; diff $$i /usr/src/cmd/awk | ind; done

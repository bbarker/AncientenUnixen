CFLAGS = -O -DTELL -DBUFSIZ=1024

OBJS =	sh.o sh.dol.o sh.err.o sh.exec.o sh.exp.o sh.func.o sh.glob.o\
	sh.hist.o sh.lex.o sh.misc.o sh.parse.o sh.print.o sh.sem.o\
	sh.set.o sh.wait.o alloc.o sh.init.o printf.o

all: csh

cp: all
	cp csh /bin
	cp csh.1 /usr/man/man1
	cp doc/csh /usr/doc
	@make clean

csh: $(OBJS)
	cc -s -o csh $(OBJS)

clean:
	rm -f csh $(OBJS)

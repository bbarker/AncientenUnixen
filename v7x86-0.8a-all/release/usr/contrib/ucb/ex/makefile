VERSION = 3.6

INCS = -I../include

CFLAGS = -O $(INCS) -DTABS=8 -DCRYPT -DLISPCODE -DCHDIR -DVMUNIX

OBJS =	ex.o ex_addr.o ex_cmds.o ex_cmds2.o ex_cmdsub.o ex_data.o\
	ex_get.o ex_io.o ex_put.o ex_re.o ex_set.o ex_subr.o\
	ex_temp.o ex_tty.o ex_unix.o ex_v.o ex_vadj.o ex_vget.o\
	ex_vmain.o ex_voper.o ex_vops.o ex_vops2.o ex_vops3.o\
	ex_vput.o ex_vwind.o printf.o bcopy.o

all: ex exrecover expreserve

cp: all
	rm -f /usr/ucb/ex /usr/ucb/vi /usr/ucb/view /usr/ucb/edit
	cp ex /usr/ucb
	ln /usr/ucb/ex /usr/ucb/vi
	ln /usr/ucb/ex /usr/ucb/view
	ln /usr/ucb/ex /usr/ucb/edit
	cp exrecover /usr/lib/ex$(VERSION)recover
	cp expreserve /usr/lib/ex$(VERSION)preserve
	chmod 4711 /usr/lib/ex$(VERSION)recover /usr/lib/ex$(VERSION)preserve
	cp ex.1 /usr/man/man1
	cp vi.1 /usr/man/man1
	-mkdir /usr/doc/ex
	cp doc/* /usr/doc/ex
	@make clean

ex: $(OBJS)
	cc -n -s -o ex $(OBJS) -ltermlib

exrecover: exrecover.o
	cc -s -o exrecover exrecover.o

expreserve: expreserve.o
	cc -s -o expreserve expreserve.o

exrecover.o expreserve.o: $@
	cc -O $(INCS) -c $?

clean:
	rm -f ex exrecover expreserve $(OBJS) exrecover.o expreserve.o

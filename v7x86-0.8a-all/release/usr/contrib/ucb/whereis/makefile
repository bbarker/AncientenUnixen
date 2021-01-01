CFLAGS = -O

all: whereis

cp: all
	cp whereis /usr/ucb
	cp whereis.1 /usr/man/man1
	@make clean

whereis: whereis.c
	cc $(CFLAGS) -n -s -o whereis whereis.c

clean:
	rm -f whereis

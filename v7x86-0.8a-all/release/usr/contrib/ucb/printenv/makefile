CFLAGS = -O

all: printenv

cp: all
	cp printenv /usr/ucb
	cp printenv.1 /usr/man/man1
	@make clean

printenv: printenv.c
	cc $(CFLAGS) -n -s -o printenv printenv.c

clean:
	rm -f printenv

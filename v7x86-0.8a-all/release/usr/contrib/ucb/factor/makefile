CFLAGS = -O

all: factor

cp: all
	cp factor /usr/ucb
	@make clean

factor: factor.c
	cc $(CFLAGS) -n -s -o factor factor.c

clean:
	rm -f factor

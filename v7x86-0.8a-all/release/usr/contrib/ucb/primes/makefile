
CFLAGS = -O

all: primes

cp: all
	cp primes /usr/ucb
	@make clean

primes: primes.c
	cc $(CFLAGS) -n -s -o primes primes.c -lm

clean:
	rm -f primes

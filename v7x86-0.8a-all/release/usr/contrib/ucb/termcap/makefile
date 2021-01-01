all: termcap

cp: all
	cp termcap /etc
	cp termcap.5 /usr/man/man5
	@make clean

termcap: termcap.src
	ed - termcap.src < reorder

tabset:
	mkdir /usr/lib/tabset
	cp tabset/* /usr/lib/tabset

clean:
	rm -f termcap

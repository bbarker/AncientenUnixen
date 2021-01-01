CFLAGS = -O

OBJS =	box.o clear.o initscr.o endwin.o mvprintw.o mvscanw.o mvwin.o\
	newwin.o overlay.o overwrite.o printw.o scanw.o refresh.o\
	touchwin.o erase.o clrtobot.o clrtoeol.o cr_put.o cr_tty.o\
	longname.o delwin.o insertln.o deleteln.o scroll.o getstr.o\
	getch.o addstr.o addch.o move.o curses.o unctrl.o standout.o\
	insch.o delch.o

all: crlib

cp: all
	cp crlib /usr/lib/libcurses.a
	ranlib /usr/lib/libcurses.a
	cp curses.h unctrl.h /usr/include
	cp curses.3 /usr/man/man3
	-mkdir /usr/doc/curses
	cp doc/* /usr/doc/curses
	@make clean

crlib: $(OBJS)
	rm -f crlib
	ar cr crlib $(OBJS)
	ranlib crlib

clean:
	rm -f crlib $(OBJS)

all: mbr pcuboot

mbr:
	as -o mbr.o mbr.s
	ld -s -T 600 -o mbr.x mbr.o
	dd if=mbr.x of=mbr bs=1 skip=32
	rm mbr.x mbr.o

pcuboot:
	as -o pcuboot.o pcuboot.s
	ld -s -T 7c00 -o pcuboot pcuboot.o
	rm pcuboot.o

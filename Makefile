boot.bin: boot.o
	ld boot.o -o boot.bin -Ttext 0 --oformat binary

boot.o: boot.s
	as boot.s -o boot.o

.PHONY: clean
clean:
	rm -f boot.o boot.bin *~

mbr: mbr.asm app
	nasm mbr.asm
	dd if=app of=mbr oflag=append conv=notrunc,sync

app: app.asm
	nasm app.asm

.PHONY: clean
clean:
	rm -f mbr app

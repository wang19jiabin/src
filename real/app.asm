	dd section.size.start
	dw start
	dw section.code.start
data:	dw section.data.start
stack:	dw section.stack.start

section code align=16 vstart=0
start:	mov ax, [stack]
	mov ss, ax
	mov sp, top

	mov ax, [data]
	mov ds, ax
	mov ax, 0xb800
	mov es, ax

	mov si, s
	mov di, 0
	mov cx, 3
	call print
	hlt

print:	cld
	rep movsw
	ret

section data align=16 vstart=0
s:	db 'a', 0xc, 'p', 0xc, 'p', 0xc

section stack align=16 vstart=0
	resb 16
top:

section size

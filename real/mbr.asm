	mov ax, 0x7c0
	mov ss, ax
	mov sp, 0

	mov ax, 0x7e0
	mov ds, ax
	xor di, di
	mov si, 1
	call read_sector

	mov ax, [0]
	mov dx, [2]
	mov bx, 512
	div bx

	cmp dx, 0
	jnz .0
	dec ax
.0:	cmp ax, 0
	jz .2

	mov cx, ax
.1:	inc si
	call read_sector
	loop .1

.2:	mov bx, 6
	mov cx, 3
.3:	shr word [bx], 4
	add word [bx], 0x7e0
	add bx, 2
	loop .3

	jmp far [4]

read_sector:
	push ax
	push cx
	push dx

	mov dx, 0x1f2
	mov al, 1
	out dx, al

	inc dx
	mov ax, si
	out dx, al

	inc dx
	mov al, ah
	out dx, al

	inc dx
	mov al, 0
	out dx, al

	inc dx
	mov al, 0xe0
	out dx, al

	inc dx
	mov al, 0x20
	out dx, al

.4:	in al, dx
	and al, 0x88
	cmp al, 0x08
	jnz .4

	mov cx, 256
	mov dx, 0x1f0
.5:	in ax, dx
	mov [di], ax
	add di, 2
	loop .5

	pop dx
	pop cx
	pop ax
	ret

	times 510-($-$$) db 0
	dw 0xaa55

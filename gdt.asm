	mov ax, 0x7e0
	mov ds, ax

	xor bx, bx
	mov dword [bx], 0
	add bx, 4
	mov dword [bx], 0

	add bx, 4
	mov dword [bx], 0x7c0001ff
	add bx, 4
	mov dword [bx], 0x00409800

	add bx, 4
	mov dword [bx], 0x8000ffff
	add bx, 4
	mov dword [bx], 0x0040920b

	add bx, 4
	mov dword [bx], 0x7c00fffe
	add bx, 4
	mov dword [bx], 0x00cf9600

	mov ax, 0x7c0
	mov ds, ax
	lgdt [gdt]

	in al, 0x92
	or al, 0x02
	out 0x92, al

	cli
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	jmp dword 8:start

	bits 32
start:	mov ax, 0x10
	mov ds, ax

	mov ax, 0x18
	mov ss, ax
	xor esp, esp

	call print
	hlt

print:	mov byte [0], 'p'
	mov byte [1], 0xc
	ret

gdt:	dw 31
	dd 0x7e00

	times 510-($-$$) db 0
	dw 0xaa55

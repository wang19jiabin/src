assume cs:code, ds:data

data segment
	db 'hello'
data ends

code segment
start:
	mov ax, data
	mov ds, ax
	mov ax, 0b800h
	mov es, ax
	mov si, 0
	mov di, 0
	mov cx, 5
l:
	mov al, [si]
	mov es:[di], al
	mov byte ptr es:[di+1], 0ch
	inc si
	add di, 2
	loop l
j:
	jmp short j
	mov ax, 4c00h
	int 21h
code ends
end start

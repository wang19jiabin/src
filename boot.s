.code16
.globl _start

_start:
	movw $0x07c0, %ax
	movw %ax, %ds
	movw $0xb800, %ax
	movw %ax, %es
	movw $0, %si
	movw $0, %di
	movw $5, %cx

loop:
	movb data(%si), %al
	movb %al, %es:(%di)
	incw %si
	incw %di
	movb $0xc, %es:(%di)
	incw %di
	loop loop

stop:
	jmp stop

data:
	.ascii "hello"

.org 510
.word 0xaa55

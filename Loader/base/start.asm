section .data
extern DOSLDR_MAIN
global start
section .code
start:
	mov ax,1*8
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax
	call DOSLDR_MAIN
	jmp $
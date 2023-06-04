[BITS 32]
section .data
GLOBAL	asm_error0,asm_error1,asm_error3,asm_error4,asm_error5
GLOBAL	asm_error6,asm_error7,asm_error8,asm_error9,asm_error10
GLOBAL	asm_error11,asm_error12,asm_error13,asm_error14,asm_error16
GLOBAL	asm_error17,asm_error18
section .text
EXTERN	ERROR0,ERROR1,ERROR3,ERROR4,ERROR5,ERROR6,ERROR7,ERROR8
EXTERN	ERROR9,ERROR10,ERROR11,ERROR12,ERROR13,PF,ERROR16
EXTERN	ERROR17,ERROR18
EXTERN	KILLAPP
asm_error0:
	cli
	mov ecx,0
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR0
	iretd
asm_error1:
	cli
	mov ecx,1
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR1
	iretd
asm_error3:
	cli
	mov ecx,3
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR3
	iretd
asm_error4:
	cli
	mov ecx,4
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR4
	iretd
asm_error5:
	cli
	mov ecx,5
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR5
	iretd	
asm_error6:
	cli
	mov ecx,6
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR6
	iretd
asm_error7:
	cli
	push	eax
	mov	eax,cr0
	and	eax,~0x08
	mov	cr0,eax
	pop	eax
	sti

	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	ERROR7
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	IRETD
	;cli
	;push	7
	;mov	ax,fs
	;cmp	ax,1*8	; 是不是在程序产生的
	;jne	KILLAPP1
	;jmp	ERROR7
asm_error8:
	cli
	mov ecx,8
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR8
	iretd
asm_error9:
	iretd
asm_error10:
	cli
	push	10
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR10
	iretd
asm_error11:
	cli
	mov ecx,11
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR11
	iretd
asm_error12:
	cli
	mov ecx,12
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR12
	iretd
asm_error13:
	cli
	mov ecx,13
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR13
	iretd
asm_error14:
	cli
	push eax
	call	PF
	add esp,8
	iretd
asm_error16:
	cli
	mov ecx,16
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call ERROR16
	iretd
asm_error17:
	cli
	mov ecx,17
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR17
	iretd
asm_error18:
	cli
	mov ecx,18
	mov edx, dword [esp+4]
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	call	ERROR18
	iretd
KILLAPP1:
	cli
	push	ax
	mov	ax,1 * 8	; 切fs gs
	mov	fs,ax
	mov	gs,ax
	pop	ax
	sti
	push ecx
	push edx
	call	KILLAPP
	add	esp,12
	jmp	$

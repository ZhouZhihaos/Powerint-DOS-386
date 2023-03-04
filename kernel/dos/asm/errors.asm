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
GLOBAL  switch_stack_app,switch_stack_sys
EXTERN NowTask_asm
switch_stack_app:
	cli
	push	ebp
	push	ax
	mov	ax,1 * 8
	mov	ds,ax
	mov	es,ax
	mov	ax,ss
	; 提前切ds,es 下面NowTask用
	push	ebx
	push	eax
	call	NowTask_asm	; 获取当前任务的TASK结构体
	mov	ebx,eax
	pop	eax
	mov	[ebx+185],ax	; task->ss1 = 应用程序ss;
	mov	ax,1 * 8
	mov	ss,ax
	mov	ebp,esp	; ebp = 应用程序esp
	add	esp,10	; ebp+ax+ebx的大小
	mov	[ebx+181],esp	; task->esp1 = 应用程序esp;
	mov	esp,[ebx+177]	; esp = task->esp0（系统esp）;
	; ------从这里开始 堆栈变为系统的------
	; 注意此时fs,gs还是应用程序的 我们可以使用它来切回寄存器
	mov	ebx,[fs:ebp]
	mov	ax,[fs:ebp+4]
	mov	ebp,[fs:ebp+6]	; 切回原来的ebp
	push	ax
	mov	ax,1 * 8	; 切fs gs
	mov	fs,ax
	mov	gs,ax
	pop	ax
	sti
	ret
switch_stack_sys:
	cli
	push	ebp
	push	ax
	push	ebx
	push	eax
	call	NowTask_asm
	mov	ebx,eax
	pop	eax
	mov	ebp,esp
	mov	esp,[ebx+181]	; 切回应用程序esp
	mov	ax,[ebx+185]	; 切回应用程序ds,es,ss
	mov	ss,ax
	mov	ds,ax
	mov	es,ax
	; ------从这里开始 堆栈变为应用程序的------
	; 我们不切fs,gs 是因为要切回其它寄存器
	push	ecx
	mov	ecx,[fs:ebp]	; 原来的ebx先装到ecx里 下面我们还要用ebx切回fs,gs
	mov	ax,[fs:ebp+4]
	mov	ebp,[fs:ebp+6]
	push	ax
	mov	ax,[fs:ebx+185]	; 切回fs,gs
	mov	fs,ax
	mov	gs,ax
	pop	ax
	mov	ebx,ecx	; 切回ebx
	pop	ecx	; 切回ecx
	sti
	ret
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
	iretd
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
	;push dword 0xffffffff
	cli
	; -------
	; push	ebp
	; push	ax
	; mov	ax,1 * 8
	; mov	ds,ax
	; mov	es,ax
	; mov	ax,ss
	; ; 提前切ds,es 下面NowTask用
	; push	ebx
	; push	eax
	; call	NowTask_asm	; 获取当前任务的TASK结构体
	; mov	ebx,eax
	; pop	eax
	; mov	[ebx+185],ax	; task->ss1 = 应用程序ss;
	; mov	ax,1 * 8
	; mov	ss,ax
	; mov	ebp,esp	; ebp = 应用程序esp
	; add	esp,10	; ebp+ax+ebx的大小
	; mov	[ebx+181],esp	; task->esp1 = 应用程序esp;
	; mov	esp,[ebx+177]	; esp = task->esp0（系统esp）;
	; ; ------从这里开始 堆栈变为系统的------
	; ; 注意此时fs,gs还是应用程序的 我们可以使用它来切回寄存器
	; mov	ebx,[fs:ebp]
	; mov	ax,[fs:ebp+4]
	; mov	ebp,[fs:ebp+6]	; 切回原来的ebp
	; ----
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
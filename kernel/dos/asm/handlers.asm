[BITS 32]
section .data
GLOBAL	asm_inthandler21, asm_inthandler20
EXTERN	inthandler21,inthandler20,inthandler36,inthandler2c,loadregisters,saveregisters
GLOBAL	asm_inthandler36,asm_inthandler2c,floppy_int
section .text
global null_inthandler
null_inthandler:
	IRETD
extern taskctl
%define ADR_GDT 0x00270000
global NowTask_asm
NowTask_asm:
	pushad
	mov	esi,dword[taskctl]
	mov	al,byte[ADR_GDT+esi+4]	; base_mid
	mov	ah,byte[ADR_GDT+esi+7]	; base_high
	shl	eax,16
	mov	ax,word[ADR_GDT+esi+2]	; base_low
	sub	eax,0xc
	mov	[ss:esp+32-4],eax	; eax在pushad中的顺序
	popad
	ret

asm_inthandler36:
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
		call	saveregisters
		PUSHAD
		CALL	inthandler36
		ADD		ESP,32
		call	loadregisters
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
		IRETD
extern flint
floppy_int:
		cli
		push	ax
		mov	ax,ds
		cmp	ax,1 * 8
		je	.from_kernel
		pop	ax
		push	ebp
		push	ax
		mov	ax,1 * 8
		mov	ds,ax
		mov	es,ax
		mov	ax,ss
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
		jmp	.from_app
.from_kernel:
		pop	ax
		pushad
		mov	eax,esp
		push	eax
		call	flint
		pop	eax
		popad
		sti
        IRETD
.from_app:
		pushad
		mov	eax,esp
		push	eax
		call	flint
		pop	eax
		popad
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
		iretd

EXTERN PCNET_IRQ
GLOBAL PCNET_ASM_INTHANDLER
PCNET_ASM_INTHANDLER:
		cli
		push	ax
		mov	ax,ds
		cmp	ax,1 * 8
		je	.from_kernel
		pop	ax
		push	ebp
		push	ax
		mov	ax,1 * 8
		mov	ds,ax
		mov	es,ax
		mov	ax,ss
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
		jmp	.from_app
.from_kernel:
		pop	ax
		pushad
		mov	eax,esp
		push	eax
		call	PCNET_IRQ
		pop	eax
		popad
		sti
        IRETD
.from_app:
		pushad
		mov	eax,esp
		push	eax
		call	PCNET_IRQ
		pop	eax
		popad
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
		iretd

EXTERN RTL8139_IRQ
GLOBAL RTL8139_ASM_INTHANDLER
RTL8139_ASM_INTHANDLER:
		cli
		push	ax
		mov	ax,ds
		cmp	ax,1 * 8
		je	.from_kernel
		pop	ax
		push	ebp
		push	ax
		mov	ax,1 * 8
		mov	ds,ax
		mov	es,ax
		mov	ax,ss
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
		jmp	.from_app
.from_kernel:
		pop	ax
		pushad
		mov	eax,esp
		push	eax
		call	RTL8139_IRQ
		pop	eax
		popad
		sti
        IRETD
.from_app:
		pushad
		mov	eax,esp
		push	eax
		call	RTL8139_IRQ
		pop	eax
		popad
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
		iretd

EXTERN sb16_handler
GLOBAL asm_sb16_handler
asm_sb16_handler:
		cli
		push	ax
		mov	ax,ds
		cmp	ax,1 * 8
		je	.from_kernel
		pop	ax
		push	ebp
		push	ax
		mov	ax,1 * 8
		mov	ds,ax
		mov	es,ax
		mov	ax,ss
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
		jmp	.from_app
.from_kernel:
		pop	ax
		pushad
		mov	eax,esp
		push	eax
		call	sb16_handler
		pop	eax
		popad
		sti
        IRETD
.from_app:
		pushad
		mov	eax,esp
		push	eax
		call	sb16_handler
		pop	eax
		popad
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
		iretd
EXTERN rtc_handler
GLOBAL asm_rtc_handler
EXTERN ide_irq
GLOBAL asm_ide_irq
asm_ide_irq:
		cli
		push	ax
		mov	ax,ds
		cmp	ax,1 * 8
		je	.from_kernel
		pop	ax
		push	ebp
		push	ax
		mov	ax,1 * 8
		mov	ds,ax
		mov	es,ax
		mov	ax,ss
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
		jmp	.from_app
.from_kernel:
		pop	ax
		pushad
		mov	eax,esp
		push	eax
		call	ide_irq
		pop	eax
		popad
		sti
        IRETD
.from_app:
		pushad
		mov	eax,esp
		push	eax
		call	ide_irq
		pop	eax
		popad
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
		iretd
asm_rtc_handler:
		cli
		push	ax
		mov	ax,ds
		cmp	ax,1 * 8
		je	.from_kernel
		pop	ax
		push	ebp
		push	ax
		mov	ax,1 * 8
		mov	ds,ax
		mov	es,ax
		mov	ax,ss
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
		jmp	.from_app
.from_kernel:
		pop	ax
		pushad
		mov	eax,esp
		push	eax
		call	rtc_handler
		pop	eax
		popad
		sti
        IRETD
.from_app:
		pushad
		mov	eax,esp
		push	eax
		call	rtc_handler
		pop	eax
		popad
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
		iretd
global asm_gui_api
extern Gui_API
asm_gui_api:
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
		call	saveregisters
		PUSHAD
		CALL	Gui_API
		ADD		ESP,32
		call	loadregisters
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
		IRETD
global asm_net_api
extern net_API
asm_net_api:
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
		call	saveregisters
		PUSHAD
		CALL	net_API
		ADD		ESP,32
		call	loadregisters
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
		IRETD
asm_inthandler20:
		cli
		push	ax
		mov	ax,ds
		cmp	ax,1 * 8
		je	.from_kernel
		pop	ax
		push	ebp
		push	ax
		mov	ax,1 * 8
		mov	ds,ax
		mov	es,ax
		mov	ax,ss
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
		jmp	.from_app
.from_kernel:
		pop	ax
		pushad
		mov	eax,esp
		push	eax
		call	inthandler20
		pop	eax
		popad
		sti
        IRETD
.from_app:
		pushad
		mov	eax,esp
		push	eax
		call	inthandler20
		pop	eax
		popad
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
		iretd
asm_inthandler21:
		cli
		push	ax
		mov	ax,ds
		cmp	ax,1 * 8
		je	.from_kernel
		pop	ax
		push	ebp
		push	ax
		mov	ax,1 * 8
		mov	ds,ax
		mov	es,ax
		mov	ax,ss
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
		jmp	.from_app
.from_kernel:
		pop	ax
		pushad
		mov	eax,esp
		push	eax
		sti
		call	inthandler21
		pop	eax
		popad
        IRETD
.from_app:
		pushad
		mov	eax,esp
		push	eax
		call	inthandler21
		pop	eax
		popad
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
		iretd
asm_inthandler2c:
		cli
		push	ax
		mov	ax,ds
		cmp	ax,1 * 8
		je	.from_kernel
		pop	ax
		push	ebp
		push	ax
		mov	ax,1 * 8
		mov	ds,ax
		mov	es,ax
		mov	ax,ss
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
		jmp	.from_app
.from_kernel:
		pop	ax
		pushad
		mov	eax,esp
		push	eax
		call	inthandler2c
		pop	eax
		popad
		sti
        IRETD
.from_app:
		pushad
		mov	eax,esp
		push	eax
		call	inthandler2c
		pop	eax
		popad
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
		iretd
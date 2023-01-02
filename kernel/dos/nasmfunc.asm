[BITS 32]
		GLOBAL	io_hlt, io_cli, io_sti, io_stihlt
		GLOBAL	io_in8,  io_in16,  io_in32
		GLOBAL	io_out8, io_out16, io_out32
		GLOBAL	io_load_eflags, io_store_eflags
		GLOBAL	load_gdtr, load_idtr
		GLOBAL	asm_inthandler21, asm_inthandler20, ASM_call
		EXTERN	inthandler21,inthandler20,inthandler36,inthandler2c
		GLOBAL	asm_inthandler36,asm_inthandler2c
		GLOBAL	load_cr0, store_cr0,memtest_sub,farjmp,farcall,start_app
		GLOBAL	loadregisters,saveregisters,get_eip
		GLOBAL	asm_error0,asm_error1,asm_error3,asm_error4,asm_error5
		GLOBAL	asm_error6,asm_error7,asm_error8,asm_error9,asm_error10
		GLOBAL	asm_error11,asm_error12,asm_error13,asm_error14,asm_error16
		GLOBAL	asm_error17,asm_error18,load_tr,floppy_int
[SECTION .data]
str: db 'Yun Xing Ni Ma De Kernel Xiang Si Shi Bu Shi',0
[SECTION .text]
; 
; Protected Mode BIOS Call Functionailty v2.0 - by Napalm
; -------------------------------------------------------
; 
; This is code shows how its POSSIBLE to execute BIOS interrupts
; by switch out to real-mode and then back into protected mode.
; 
; If you wish to use all or part of this code you must agree
; to the license at the following URL.
; 
; License: http://creativecommons.org/licenses/by-sa/2.0/uk/
;         
; Notes: This file is in NASM syntax.
;        Turn off paging before calling these functions.
;        int32() resets all selectors.
;
; C Prototype:
;	void _cdelc int32(unsigned char intnum, regs16_t *regs);
; 
; Example of usage:
;   regs.ax = 0x0013;
;   int32(0x10, &regs);
;   memset((char *)0xA0000, 1, (320*200));
;   memset((char *)0xA0000 + (100*320+80), 14, 80);
;   regs.ax = 0x0000;
;   int32(0x16, &regs);
;   regs.ax = 0x0003;
;   int32(0x10, &regs);
; 
; 
;   改写（原来根本用不了）+注释翻译 By zhouzhihao 2022
;
global int32

struc regs16_t
	.di	resw 1
	.si	resw 1
	.bp	resw 1
	.sp resw 1
	.bx	resw 1
	.dx	resw 1
	.cx	resw 1
	.ax	resw 1
	.gs	resw 1
	.fs	resw 1
	.es	resw 1
	.ds	resw 1
	.ef resw 1
endstruc

%define ADR_BOTPAK 							   0x280000
%define INT32_BASE                             0x7C00	; 这个位置分页线性地址=分段线性地址
%define REBASE(x)                              (((x) - reloc) + INT32_BASE)	; 计算在INT32_BASE中的数据的偏移地址
%define DATA32                                 1 * 8
%define CODE32                                 1000 * 8
%define CODE16                                 1001 * 8
%define DATA16                                 1002 * 8
%define STACK16                                (INT32_BASE - regs16_t_size)	; 实模式堆栈只需要存寄存器就好了 所以不用多大
int32:
		cli                     			; 禁止中断
		
		; 复制reloc~int32_end的代码到INT32_BASE(0x7c00)
		mov  esi, ADR_BOTPAK + reloc
		mov  edi, INT32_BASE
		mov  ecx, (int32_end - reloc)
		cld									; 复制方向（正向
		rep  movsb
		; jmp	dword	CODE32:INT32_BASE	; 这样不好返回系统
		
		mov	eax,CODE32						; 这样调用更好返回系统
		push	eax
		mov	eax,INT32_BASE
		push	eax
		call	far	[esp]					; 相当于 call dword CODE32:INT32_BASE
		add	esp,8							; 丢弃栈中数据
		ret
reloc:                               		; by Napalm
		mov  [REBASE(stack32_ptr)], esp        ; 保存保护模式的esp
		sidt [REBASE(idt32_ptr)]               ; 保存保护模式的IDT
		lea  esi, [esp+20]                   ; esi = 保护模式堆栈中的intnum指针
		lodsd                                  ; al = intnum
		mov  byte[REBASE(ib)], al                  ; 把INT机器码补全
		mov  esi, [esi]                        ; esi = 寄存器指针
		mov  edi, STACK16                      ; edi = 实模式堆栈地址
		mov  ecx, regs16_t_size                ; ecx = 寄存器结构体的大小
		mov  esp, edi
		rep  movsb                             ; 将保护模式下堆栈中的寄存器结构体拷贝到实模式堆栈
		
		; 因为要复制（分页线性地址和分段线性地址不同）数据 所以推后关分页
		mov  eax, cr0
		and  eax, ~0x80000000				; 关闭内存分页
		mov  cr0, eax
		
		jmp  dword CODE16:REBASE(p_mode16)      ; 跳到16位保护模式（为切换回实模式做缓冲
[bits 16]
p_mode16:
		mov  ax, DATA16							; 用于缓冲的无用选择子
		mov  ds, ax
		mov  es, ax
		mov  fs, ax
		mov  gs, ax
		mov  ss, ax
		mov  eax, cr0                          ; 关闭保护模式
		and  al, ~0x01
		mov  cr0, eax
		jmp  word 0x0000:REBASE(r_mode16)      ; 跳回实模式
r_mode16:
		xor  ax, ax
		mov  ds, ax
		mov  ss, ax
		lidt [REBASE(idt16_ptr)]               ; 切换回实模式的IDT(IVT)
		mov  bx, 0x0870
		call resetpic                          ; 设置PIC成实模式对应状态
		popa                                   ; 加载复制到堆栈的寄存器
		pop  gs
		pop  fs
		pop  es
		pop  ds
		sti                                    ; 开中断
		db 0xCD                                ; INT的机器码
	ib: db 0x00
		cli                                    ; 关中断
		xor  sp, sp
		mov  ss, sp
		mov  sp, INT32_BASE                    ; 把sp设置回来（之前pop了
		pushf                                  ; 把寄存器返回到堆栈
		push ds
		push es
		push fs
		push gs
		pusha
		mov  bx, 0x2028
		call resetpic                          ; 设置PIC成保护模式对应状态
		mov  eax, cr0                          ; 打开保护模式
		inc  eax
		mov  cr0, eax
		jmp  dword CODE32:REBASE(p_mode32)     ; 切换到32位保护模式
[bits 32]
p_mode32:
		mov  ax, DATA32                        ; 设置回原来的ds
		mov  ds, ax
		mov  es, ax
		mov  fs, ax
		mov  gs, ax
		mov  ss, ax

		; 因为要复制（分页线性地址和分段线性地址不同）数据 提前开分页
		mov  eax, cr0						; 开启内存分页
		or   eax, 0x80000000
		mov  cr0, eax
		
		lidt [REBASE(idt32_ptr)]               ; 设置回原来的IDT
		mov  esp, [REBASE(stack32_ptr)]        ; 设置回原来的esp
		mov  esi, STACK16                      ; 将实模式堆栈中的寄存器复制回保护模式堆栈中的寄存器结构体（返回的数据
											   ; esi = 实模式堆栈地址
		lea  edi, [esp+24]
		mov  edi, [edi]                        ; edi = 保护模式堆栈中的寄存器结构体指针
		mov  ecx, regs16_t_size                ; ecx = 寄存器结构体的大小
		cld                                    ; 复制方向（正向
		rep  movsb
		sti                                    ; 开中断
		retf                                    ; 返回上面的call far
		
resetpic:                                  ; reset's 8259 master and slave pic vectors
		push ax                                ; expects bh = master vector, bl = slave vector
		mov  al, 0x11                          ; 0x11 = ICW1_INIT | ICW1_ICW4
		out  0x20, al                          ; send ICW1 to master pic
		out  0xA0, al                          ; send ICW1 to slave pic
		mov  al, bh                            ; get master pic vector param
		out  0x21, al                          ; send ICW2 aka vector to master pic
		mov  al, bl                            ; get slave pic vector param
		out  0xA1, al                          ; send ICW2 aka vector to slave pic
		mov  al, 0x04                          ; 0x04 = set slave to IRQ2
		out  0x21, al                          ; send ICW3 to master pic
		shr  al, 1                             ; 0x02 = tell slave its on IRQ2 of master
		out  0xA1, al                          ; send ICW3 to slave pic
		shr  al, 1                             ; 0x01 = ICW4_8086
		out  0x21, al                          ; send ICW4 to master pic
		out  0xA1, al                          ; send ICW4 to slave pic
		pop  ax                                ; restore ax from stack
		ret                                    ; return to caller
		
stack32_ptr:                               ; address in 32bit stack after we
	dd 0x00000000                          ;   save all general purpose registers
		
idt32_ptr:                                 ; IDT table pointer for 32bit access
	dw 0x0000                              ; table limit (size)
	dd 0x00000000                          ; table base address
		
idt16_ptr:                                 ; IDT table pointer for 16bit access
	dw 0x03FF                              ; table limit (size)
	dd 0x00000000                          ; table base address

int32_end:                                 ; end marker (so we can copy the code)
 
get_eip:		; int get_eip();
	mov	eax,[esp]
	ret
farjmp:		; void farjmp(int eip, int cs);
	pushad
	JMP    FAR [ESP+36]    ; eip, cs
	popad
	RET
farcall:		; void farjmp(int eip, int cs);
	pushad
	call    FAR [ESP+36]    ; eip, cs
	popad
	RET
load_tr:
	ltr [esp+4]
	ret

extern taskctl
%define ADR_GDT 0x00270000
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
;_asm_inthandler36:
;		push	es
;		push	ds
;		push	ax
;		cli
;		mov	ax,1*8
;		mov	ds,ax
;		mov	es,ax
;		sti
;		pop	ax
;		call	_saveregisters
;		PUSHAD	
;		CALL	_inthandler36
;		ADD		ESP,32
;		call	_loadregisters
;		pop	ds
;		pop	es
;		IRETD
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
;_asm_inthandler20:
;        PUSH    ES
;        PUSH    DS
;        PUSHAD
;        MOV        EAX,ESP
;        PUSH    EAX
;        MOV        AX,SS
;        MOV        DS,AX
;        MOV        ES,AX
;        CALL    _inthandler20
;        POP        EAX
;        POPAD
;        POP        DS
;        POP        ES
;        IRETD
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
		call	inthandler21
		pop	eax
		popad
		sti
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
io_hlt:	; void io_hlt(void);
		HLT
		RET

io_cli:	; void io_cli(void);
		CLI
		RET
load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET
io_sti:	; void io_sti(void);
		STI
		RET

EXTERN clear
EXTERN Print_Hex
EXTERN Clear_A_Line
testsize:	dd	0
memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		CLI
		PUSH	EDI						; （由于还要使用EBX, ESI, EDI）
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
		MOV		dword[ds:ADR_BOTPAK+testsize],1024*1024*1024	; testsize = 1024*1024*1024;
mts_loop:
		pushad
		push eax
		call Print_Hex
		add esp,4
		call Clear_A_Line
		popad
		MOV		EBX,EAX
		ADD		EBX,[ds:ADR_BOTPAK+testsize]		 	; p = i + testsize;
		SUB		EBX,4					; p -= 4;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,[ds:ADR_BOTPAK+testsize]			; i += testsize;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		
		JBE		mts_loop
		STI
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		CMP		dword[ds:ADR_BOTPAK+testsize],0x1000	; if (testsize == 0x1000) goto mts_nomore;
		JE		mts_nomore
		SHR		dword[ds:ADR_BOTPAK+testsize],2	; testsize /= 4;
		JMP		mts_loop
mts_nomore:
		STI
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET
io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS 
		POP		EAX
		RET

io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS 
		RET

load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

ASM_call:  ;移动光标
mov dx,03d4h;03d4h是索引端口
mov al,0eh;内部的0eh位置存放着光标位置的高八位
out dx,al
inc dx;03d5h是数据端口用于读写数据
in al,dx;读取光标的高八位并且放入bh
mov bh,al
 
dec dx;这儿开始读取光标位置的低八位放入bl
mov al,0fh;0fh位置存放着光标位置的低八位
out  dx,al
inc dx
in al,dx
mov bl,al
 
 
mov word bx,[esp+4]   ;获取参数中的光标位置
 
mov  dx,03d4h;这段代码将改变后的光标位置写入端口内相应的地方以便下次访问
mov al,0eh;写入光标位置高八位
out dx,al
inc dx
mov al,bh
out dx,al
 
dec dx
mov al,0fh    ;写入光标位置低八位
out dx,al
inc dx
mov al,bl
out dx,al
ret

registers:	dd	0,0,0,0,0,0,0
loadregisters:
	mov	eax,[ds:ADR_BOTPAK+registers]
	mov	ebx,[ds:ADR_BOTPAK+registers+4]
	mov	ecx,[ds:ADR_BOTPAK+registers+8]
	mov	edx,[ds:ADR_BOTPAK+registers+12]
	mov	esi,[ds:ADR_BOTPAK+registers+16]
	mov	edi,[ds:ADR_BOTPAK+registers+20]
	mov	ebp,[ds:ADR_BOTPAK+registers+24]
	ret

saveregisters:
	mov	[ds:ADR_BOTPAK+registers],eax
	mov	[ds:ADR_BOTPAK+registers+4],ebx
	mov	[ds:ADR_BOTPAK+registers+8],ecx
	mov	[ds:ADR_BOTPAK+registers+12],edx
	mov	[ds:ADR_BOTPAK+registers+16],esi
	mov	[ds:ADR_BOTPAK+registers+20],edi
	mov	[ds:ADR_BOTPAK+registers+24],ebp
	ret
GLOBAL intreturn
intreturn:		; void intreturn(int eax,int ebx,int ecx,int edx,int esi,int edi,int ebp);
; 中断返回数据
	push	eax
	mov eax,[esp+8]
	mov [ds:ADR_BOTPAK+registers],eax
	mov eax,[esp+12]
	mov	[ds:ADR_BOTPAK+registers+4],eax
	mov eax,[esp+16]
	mov	[ds:ADR_BOTPAK+registers+8],eax
	mov eax,[esp+20]
	mov	[ds:ADR_BOTPAK+registers+12],eax
	mov eax,[esp+24]
	mov	[ds:ADR_BOTPAK+registers+16],eax
	mov eax,[esp+28]
	mov	[ds:ADR_BOTPAK+registers+20],eax
	mov eax,[esp+32]
	mov	[ds:ADR_BOTPAK+registers+24],eax
	pop	eax
	ret

;;蜂鸣器驱动
gensound1:
     push ax
     push bx
     push cx
     push dx
     push di

     mov al, 0b6H
     out 43h, al
     mov dx, 12h
     mov ax, 348ch
     div di
     out 42h, al

     mov al, ah
     out 42h, al

     in al, 61h
     mov ah, al
     or al, 3
     out 61h, al
wait1:
     mov ecx, 3314
     call waitf
delay1:
     dec bx
     jnz wait1

     mov al, ah
     out 61h, al

     pop di
     pop dx
     pop cx
     pop bx
     pop ax
     ret 
;--------------------------
waitf:
      push ax
waitf1:
      in al,61h
      and al,10h
      cmp al,ah
      je waitf1
      mov ah,al
      loop waitf1
      pop ax
      ret

;--------------发声调用函数----------------
GLOBAL gensound
gensound:
      mov edi, [esp+4]
      mov ebx,[esp+8]
	  pusha
      call gensound1
	  popa
      ret

global	init_page
extern	C_init_page
%define	PDE_ADDRESS		0x400000
init_page:	; void init_page(void)
	call	C_init_page
	mov	eax,PDE_ADDRESS
	mov	cr3,eax
	mov	eax,cr0
	or	eax,0x80000000
	mov	cr0,eax	; 开启PAE分页
	ret

GLOBAL get_cpu1
get_cpu1:
	mov eax,0
	DB 0x0F,0xA2
	mov eax,ebx
	ret
GLOBAL get_cpu2
get_cpu2:
	mov eax,0
	DB 0x0F,0xA2
	mov eax,ecx
	ret
GLOBAL get_cpu3
get_cpu3:
	mov eax,0
	DB 0x0F,0xA2
	mov eax,edx
	ret

GLOBAL get_cpu4
get_cpu4:
	mov eax,[esp+4]
	DB 0x0F,0xA2
	mov eax,eax
	ret
GLOBAL get_cpu5
get_cpu5:
	mov eax,[esp+4]
	DB 0x0F,0xA2
	mov eax,ebx
	ret
GLOBAL get_cpu6
get_cpu6:
	mov eax,[esp+4]
	DB 0x0F,0xA2
	mov eax,ecx
	ret
GLOBAL get_cpu7
get_cpu7:
	mov eax,[esp+4]
	DB 0x0F,0xA2
	mov eax,edx
	ret
EXTERN	ERROR0,ERROR1,ERROR3,ERROR4,ERROR5,ERROR6,ERROR7,ERROR8
EXTERN	ERROR9,ERROR10,ERROR11,ERROR12,ERROR13,PF,ERROR16
EXTERN	ERROR17,ERROR18
EXTERN	KILLAPP
asm_error0:
	cli
	push	0
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR0
asm_error1:
	cli
	push	1
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR1
asm_error3:
	cli
	push	3
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR3
asm_error4:
	cli
	push	4
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR4
asm_error5:
	cli
	push	5
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR5
asm_error6:
	cli
	push	6
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR6
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
	push	8
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR8
asm_error9:
	cli
	push	9
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR9
asm_error10:
	cli
	push	10
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR10
asm_error11:
	cli
	push	11
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR11
asm_error12:
	cli
	push	12
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR12
asm_error13:
	cli
	push	13
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR13
asm_error14:
	cli
	push eax
	call	PF
	add esp,8
	iretd
asm_error16:
	cli
	push	16
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR16
asm_error17:
	cli
	push	17
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR17
asm_error18:
	cli
	push	18
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	ERROR18
KILLAPP1:
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	call	KILLAPP
	sti
	add	esp,12
	jmp	$

global null_inthandler
null_inthandler:
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
system_timer_fractions:  resd 1          ; Fractions of 1 ms since timer initialized
system_timer_ms:         resd 1          ; Number of whole ms since timer initialized
IRQ0_fractions:          resd 1          ; Fractions of 1 ms between IRQs
IRQ0_ms:                 resd 1          ; Number of whole ms between IRQs
IRQ0_frequency:          resd 1          ; Actual frequency of PIT
PIT_reload_value:        resw 1          ; Current PIT reload value
global __init_PIT
__init_PIT:
    pushad
	; ebx = 时钟频率
	mov ebx,8007
    ; Do some checking
 
    mov eax,0x10000                   ;eax = reload value for slowest possible frequency (65536)
    cmp ebx,18                        ;Is the requested frequency too low?
    jbe .gotReloadValue               ; yes, use slowest possible frequency
 
    mov eax,1                         ;ax = reload value for fastest possible frequency (1)
    cmp ebx,1193181                   ;Is the requested frequency too high?
    jae .gotReloadValue               ; yes, use fastest possible frequency
 
    ; Calculate the reload value
 
    mov eax,3579545
    mov edx,0                         ;edx:eax = 3579545
    div ebx                           ;eax = 3579545 / frequency, edx = remainder
    cmp edx,3579545 / 2               ;Is the remainder more than half?
    jb .l1                            ; no, round down
    inc eax                           ; yes, round up
 .l1:
    mov ebx,3
    mov edx,0                         ;edx:eax = 3579545 * 256 / frequency
    div ebx                           ;eax = (3579545 * 256 / 3 * 256) / frequency
    cmp edx,3 / 2                     ;Is the remainder more than half?
    jb .l2                            ; no, round down
    inc eax                           ; yes, round up
 .l2:
 
 
 ; Store the reload value and calculate the actual frequency
 
 .gotReloadValue:
    push eax                          ;Store reload_value for later
    mov [ds:ADR_BOTPAK+PIT_reload_value],ax         ;Store the reload value for later
    mov ebx,eax                       ;ebx = reload value
 
    mov eax,3579545
    mov edx,0                         ;edx:eax = 3579545
    div ebx                           ;eax = 3579545 / reload_value, edx = remainder
    cmp edx,3579545 / 2               ;Is the remainder more than half?
    jb .l3                            ; no, round down
    inc eax                           ; yes, round up
 .l3:
    mov ebx,3
    mov edx,0                         ;edx:eax = 3579545 / reload_value
    div ebx                           ;eax = (3579545 / 3) / frequency
    cmp edx,3 / 2                     ;Is the remainder more than half?
    jb .l4                            ; no, round down
    inc eax                           ; yes, round up
 .l4:
    mov [ds:ADR_BOTPAK+IRQ0_frequency],eax          ;Store the actual frequency for displaying later
 
 
 ; Calculate the amount of time between IRQs in 32.32 fixed point
 ;
 ; Note: The basic formula is:
 ;           time in ms = reload_value / (3579545 / 3) * 1000
 ;       This can be rearranged in the following way:
 ;           time in ms = reload_value * 3000 / 3579545
 ;           time in ms = reload_value * 3000 / 3579545 * (2^42)/(2^42)
 ;           time in ms = reload_value * 3000 * (2^42) / 3579545 / (2^42)
 ;           time in ms * 2^32 = reload_value * 3000 * (2^42) / 3579545 / (2^42) * (2^32)
 ;           time in ms * 2^32 = reload_value * 3000 * (2^42) / 3579545 / (2^10)
 
    pop ebx                           ;ebx = reload_value
    mov eax,0xDBB3A062                ;eax = 3000 * (2^42) / 3579545
    mul ebx                           ;edx:eax = reload_value * 3000 * (2^42) / 3579545
    shrd eax,edx,10
    shr edx,10                        ;edx:eax = reload_value * 3000 * (2^42) / 3579545 / (2^10)
 
    mov [ds:ADR_BOTPAK+IRQ0_ms],edx                 ;Set whole ms between IRQs
    mov [ds:ADR_BOTPAK+IRQ0_fractions],eax          ;Set fractions of 1 ms between IRQs
 
 
 ; Program the PIT channel
 
    pushfd
    cli                               ;Disabled interrupts (just in case)
 
    mov al,00110100b                  ;channel 0, lobyte/hibyte, rate generator
    out 0x43, al
 
    mov ax,[ds:ADR_BOTPAK+PIT_reload_value]         ;ax = 16 bit reload value
    out 0x40,al                       ;Set low byte of PIT reload value
    mov al,ah                         ;ax = high 8 bits of reload value
    out 0x40,al                       ;Set high byte of PIT reload value
 
    popfd
 
    popad
    ret
global init_float
init_float:
	cli
	FNINIT                                  ; load defaults to FPU
	finit
	;cvttss2si eax, [esp+4] 
	mov eax, cr0
	and ax, 0xFFFB		;clear coprocessor emulation CR0.EM
	or ax, 0x2			;set coprocessor monitoring  CR0.MP
	mov cr0, eax
	mov eax, cr4
	or ax, 3 << 9		;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
	mov cr4, eax
	sti
	ret
global check
check:
	mov eax,cs      ; 获取cs的值 如果等于0x10则是ldrldr跳转来的
	cmp eax,0x10
	jne .not
	ret
.not:
	mov edx,str
	mov eax,5
	int 36h
	mov eax,0x1e
	int 36h
.hlt:
	jmp .hlt
global setjmp
; int setjmp(jmp_buf env);
setjmp:
    mov ecx, [esp + 4]  ; ecx = env
    mov edx, [esp + 0]  ; edx = ret addr
    mov [ecx + 0], edx
    mov [ecx + 4], ebx
    mov [ecx + 8], esp
    mov [ecx + 12], ebp
    mov [ecx + 16], esi
    mov [ecx + 20], edi
    mov [ecx + 24], eax ; eax = trigblock()'s ret val

    xor eax, eax    ; setjmp ret val = 0
    ret

global longjmp
; void longjmp(jmp_buf env, int val)
longjmp:

    mov edx, [esp + 4]  ; edx = env
    mov eax, [esp + 8]  ; eax = val
    mov ecx, [edx + 0]  ; ecx = setjmp()'s ret val 
    mov ebx, [edx + 4]
    mov esp, [edx + 8]
    mov ebp, [edx + 12]
    mov esi, [edx + 16]
    mov edi, [edx + 20]
    
    ; make sure longjmp's ret val not 0
    test eax, eax   ; if eax == 0:
    jnz .1          ;   eax += 1
    inc eax         ; else: goto lable 1
.1: ; let longjmp's ret addr as setjmp's ret addr
    mov [esp + 0], ecx ; ret addr = ecx = setjmp's next code
    ret
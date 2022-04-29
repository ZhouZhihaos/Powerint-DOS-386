; naskfunc
; TAB=4
[BITS 32]						; 3制作32位模式用的机器语言
		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_asm_inthandler21, _asm_inthandler20, _ASM_call
		EXTERN	_inthandler21,_inthandler20,_inthandler36,_inthandler2c
		GLOBAL	_asm_inthandler36,_asm_inthandler2c
		GLOBAL	_load_cr0, _store_cr0,_memtest_sub,_farjmp,_farcall,_start_app
		GLOBAL	_loadregisters,_saveregisters,_get_eip
		GLOBAL	_asm_error0,_asm_error1,_asm_error3,_asm_error4,_asm_error5
		GLOBAL	_asm_error6,_asm_error7,_asm_error8,_asm_error9,_asm_error10
		GLOBAL	_asm_error11,_asm_error12,_asm_error13,_asm_error14,_asm_error16
		GLOBAL	_asm_error17,_asm_error18,_load_tr,_initialize_floppy_DMA
		GLOBAL _prepare_for_floppy_DMA_read,_prepare_for_floppy_DMA_write
		GLOBAL _floppy_int
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
global _int32

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
%define INT32_BASE                             0x7C00
%define REBASE(x)                              (((x) - reloc) + INT32_BASE)	; 计算在INT32_BASE中的数据的偏移地址
%define DATA32                                 1 * 8
%define CODE32                                 1000 * 8
%define CODE16                                 1001 * 8
%define DATA16                                 1002 * 8
%define STACK16                                (INT32_BASE - regs16_t_size)	; 实模式堆栈只需要存寄存器就好了 所以不用多大

_int32:
		cli                     			; 禁止中断
		mov  eax, cr0
		and  eax, ~0x80000000				; 关闭内存分页
											; 复制reloc~int32_end的代码到INT32_BASE(0x7c00)
		mov  cr0, eax
		mov  esi, ADR_BOTPAK + reloc
		mov  edi, INT32_BASE
		mov  ecx, (int32_end - reloc)
		cld									; 复制方向（正向
		rep  movsb
		;jmp	dword	CODE32:INT32_BASE
											; 这样调用更好返回系统
		mov	eax,CODE32
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
		lidt [REBASE(idt32_ptr)]               ; 设置回原来的IDT
		mov  esp, [REBASE(stack32_ptr)]        ; 设置回原来的esp
		mov  esi, STACK16                      ; 将实模式堆栈中的寄存器复制回保护模式堆栈中的寄存器结构体（返回的数据
											   ; esi = 实模式堆栈地址
		lea  edi, [esp+24]
		mov  edi, [edi]                        ; edi = 保护模式堆栈中的寄存器结构体指针
		mov  ecx, regs16_t_size                ; ecx = 寄存器结构体的大小
		cld                                    ; 复制方向（正向
		rep  movsb
		mov  eax, cr0						; 开启内存分页
		or   eax, 0x80000000
		mov  cr0, eax
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

extern _flint
_floppy_int:
        PUSH    ES
        PUSH    DS
        PUSHAD
        MOV        EAX,ESP
        PUSH    EAX
        MOV        AX,SS
        MOV        DS,AX
        MOV        ES,AX
        CALL    _flint
        POP        EAX
        POPAD
        POP        DS
        POP        ES
        IRETD
_get_eip:		; int get_eip();
	mov	eax,[esp]
	ret
_farjmp:		; void farjmp(int eip, int cs);
		pushad
		JMP    FAR [ESP+36]    ; eip, cs
		popad
		RET
_farcall:		; void farjmp(int eip, int cs);
		pushad
		call    FAR [ESP+36]    ; eip, cs
		popad
		RET
_load_tr:
	ltr [esp+4]
	ret
_asm_inthandler36:
		push	es
		push	ds
		push	ax
		cli
		mov	ax,1*8
		mov	ds,ax
		mov	es,ax
		sti
		pop	ax
		call	_saveregisters
		PUSHAD	
		CALL	_inthandler36
		ADD		ESP,32
		call	_loadregisters
		pop	ds
		pop	es
		IRETD
_asm_inthandler20:
        PUSH    ES
        PUSH    DS
        PUSHAD
        MOV        EAX,ESP
        PUSH    EAX
        MOV        AX,SS
        MOV        DS,AX
        MOV        ES,AX
        CALL    _inthandler20
        POP        EAX
        POPAD
        POP        DS
        POP        ES
        IRETD
_asm_inthandler21:
        PUSH    ES
        PUSH    DS
        PUSHAD
        MOV        EAX,ESP
        PUSH    EAX
        MOV        AX,SS
        MOV        DS,AX
        MOV        ES,AX
        CALL    _inthandler21
        POP        EAX
        POPAD
        POP        DS
        POP        ES
        IRETD
_asm_inthandler2c:
        PUSH    ES
        PUSH    DS
        PUSHAD
        MOV        EAX,ESP
        PUSH    EAX
        MOV        AX,SS
        MOV        DS,AX
        MOV        ES,AX
        CALL    _inthandler2c
        POP        EAX
        POPAD
        POP        DS
        POP        ES
        IRETD
_io_hlt:	; void io_hlt(void);
		HLT
		RET

_io_cli:	; void io_cli(void);
		CLI
		RET
_load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

_store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET
_io_sti:	; void io_sti(void);
		STI
		RET
EXTERN _clear
EXTERN _Print_Hex
EXTERN _Clear_A_Line
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		CLI
		PUSH	EDI						; （由于还要使用EBX, ESI, EDI）
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
		pusha
		push dword eax
		call _Print_Hex
		add esp,4
		call _Clear_A_Line
		popa
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		
		JBE		mts_loop
		STI
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		STI
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET
_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET
_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS 
		POP		EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS 
		RET

_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

_ASM_call:  ;移动光标
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

%define ADR_BOTPAK 		0x00280000
register	dd	0,0,0,0,0,0,0
_loadregisters:
	mov	eax,[ADR_BOTPAK+register]
	mov	ebx,[ADR_BOTPAK+register+4]
	mov	ecx,[ADR_BOTPAK+register+8]
	mov	edx,[ADR_BOTPAK+register+12]
	mov	esi,[ADR_BOTPAK+register+16]
	mov	edi,[ADR_BOTPAK+register+20]
	mov	ebp,[ADR_BOTPAK+register+24]
	ret

_saveregisters:
	mov	[ADR_BOTPAK+register],eax
	mov	[ADR_BOTPAK+register+4],ebx
	mov	[ADR_BOTPAK+register+8],ecx
	mov	[ADR_BOTPAK+register+12],edx
	mov	[ADR_BOTPAK+register+16],esi
	mov	[ADR_BOTPAK+register+20],edi
	mov	[ADR_BOTPAK+register+24],ebp
	ret
GLOBAL _intreturn
_intreturn:		; void intreturn(int eax,int ebx,int ecx,int edx,int esi,int edi,int ebp);
; 中断返回数据
	push	eax
	mov eax,[esp+8]
	mov [ADR_BOTPAK+register],eax
	mov eax,[esp+12]
	mov	[ADR_BOTPAK+register+4],eax
	mov eax,[esp+16]
	mov	[ADR_BOTPAK+register+8],eax
	mov eax,[esp+20]
	mov	[ADR_BOTPAK+register+12],eax
	mov eax,[esp+24]
	mov	[ADR_BOTPAK+register+16],eax
	mov eax,[esp+28]
	mov	[ADR_BOTPAK+register+20],eax
	mov eax,[esp+32]
	mov	[ADR_BOTPAK+register+24],eax
	pop	eax
	ret

;;蜂鸣器驱动
gensound:
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
GLOBAL _gensound
_gensound:
      mov edi, [esp+4]
      mov ebx,[esp+8]
	  pusha
      call gensound
	  popa
      ret

global	_init_page
extern	_C_init_page
%define	PDE_ADDRESS		0x400000
_init_page:	; void init_page(void)
	call	_C_init_page
	mov	eax,PDE_ADDRESS
	mov	cr3,eax
	mov	eax,cr0
	or	eax,0x80000000
	mov	cr0,eax	; 开启PAE分页
	ret
	
GLOBAL _get_cpu2

GLOBAL _get_cpu1
_get_cpu1:
	mov eax,0
	DB 0x0F,0xA2
	mov eax,ebx
	ret
_get_cpu2:
	mov eax,0
	DB 0x0F,0xA2
	mov eax,ecx
	ret
GLOBAL _get_cpu3
_get_cpu3:
	mov eax,0
	DB 0x0F,0xA2
	mov eax,edx
	ret

GLOBAL _get_cpu4
_get_cpu4:
	mov eax,[esp+4]
	DB 0x0F,0xA2
	mov eax,eax
	ret
GLOBAL _get_cpu5
_get_cpu5:
	mov eax,[esp+4]
	DB 0x0F,0xA2
	mov eax,ebx
	ret
GLOBAL _get_cpu6
_get_cpu6:
	mov eax,[esp+4]
	DB 0x0F,0xA2
	mov eax,ecx
	ret
GLOBAL _get_cpu7
_get_cpu7:
	mov eax,[esp+4]
	DB 0x0F,0xA2
	mov eax,edx
	ret
EXTERN	_ERROR0,_ERROR1,_ERROR3,_ERROR4,_ERROR5,_ERROR6,_ERROR7,_ERROR8
EXTERN	_ERROR9,_ERROR10,_ERROR11,_ERROR12,_ERROR13,_ERROR14,_ERROR16
EXTERN	_ERROR17,_ERROR18
EXTERN	_KILLAPP
_asm_error0:
	cli
	push	0
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR0
_asm_error1:
	cli
	push	1
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR1
_asm_error3:
	cli
	push	3
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR3
_asm_error4:
	cli
	push	4
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR4
_asm_error5:
	cli
	push	5
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR5
_asm_error6:
	cli
	push	6
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR6
_asm_error7:
	cli
	push	7
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR7
_asm_error8:
	cli
	push	8
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR8
_asm_error9:
	cli
	push	9
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR9
_asm_error10:
	cli
	push	10
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR10
_asm_error11:
	cli
	push	11
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR11
_asm_error12:
	cli
	push	12
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR12
_asm_error13:
	cli
	push	13
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR13
_asm_error14:
	cli
	push	14
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR14
_asm_error16:
	cli
	push	16
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR16
_asm_error17:
	cli
	push	17
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR17
_asm_error18:
	cli
	push	18
	mov	ax,fs
	cmp	ax,1*8	; 是不是在程序产生的
	jne	KILLAPP1
	jmp	_ERROR18
KILLAPP1:
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	call	_KILLAPP
	sti
	add	esp,12
	jmp	$
global _null_inthandler
_null_inthandler:
	IRETD
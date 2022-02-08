; naskfunc
; TAB=4
[FORMAT "WCOFF"]				; 制作目标文件的模式	
[INSTRSET "i486p"]				; 使用到486为止的指令
[BITS 32]						; 3制作32位模式用的机器语言
[FILE "naskfunc.nas"]			; 文件名

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
		GLOBAL	_asm_error17,_asm_error18
[SECTION .text]
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
_start_app:		; void start_app(int eip,int cs,int ds);
	pushad
	mov	ebx,[esp+36]	; eip
	mov	ecx,[esp+40]	; cs
	mov	eax,[esp+44]	; ds
	cli
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	sti
	call	far	[esp+36]
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	sti
	popad
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
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; （由于还要使用EBX, ESI, EDI）
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
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
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
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

register	dd	0,0,0,0,0,0,0
_loadregisters:
	mov	eax,[register]
	mov	ebx,[register+4]
	mov	ecx,[register+8]
	mov	edx,[register+12]
	mov	esi,[register+16]
	mov	edi,[register+20]
	mov	ebp,[register+24]
	ret

_saveregisters:
	mov	[register],eax
	mov	[register+4],ebx
	mov	[register+8],ecx
	mov	[register+12],edx
	mov	[register+16],esi
	mov	[register+20],edi
	mov	[register+24],ebp
	ret
GLOBAL _intreturn
_intreturn:		; void intreturn(int eax,int ebx,int ecx,int edx,int esi,int edi,int ebp);
; 中断返回数据
	mov eax,[esp+4]
	mov [register],eax
	mov eax,[esp+8]
	mov	[register+4],eax
	mov eax,[esp+12]
	mov	[register+8],eax
	mov eax,[esp+16]
	mov	[register+12],eax
	mov eax,[esp+20]
	mov	[register+16],eax
	mov eax,[esp+24]
	mov	[register+20],eax
	mov eax,[esp+28]
	mov	[register+24],eax
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
     mov cx, 3314
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
_asm_error0:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR0
_asm_error1:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR1
_asm_error3:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR3
_asm_error4:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR4
_asm_error5:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR5
_asm_error6:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR6
_asm_error7:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR7
_asm_error8:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR8
_asm_error9:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR9
_asm_error10:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR10
_asm_error11:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR11
_asm_error12:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR12
_asm_error13:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR13
_asm_error14:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR14
_asm_error16:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR16
_asm_error17:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR17
_asm_error18:
	cli
	mov	ax,1*8
	mov	ds,ax
	mov	es,ax
	sti
	call	_ERROR18
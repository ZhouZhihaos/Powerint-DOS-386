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
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD			; 用于保存的PUSH
		
		MOV		AX,SS
		MOV			DS,AX ; 将操作系统用段地址存入DS和ES
		MOV		ES,AX
		call saveregisters
		CALL	inthandler36
		ADD		ESP,32
		call loadregisters
		POP		ES
		POP		DS
		IRETD
extern flint
floppy_int:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	flint
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

EXTERN PCNET_IRQ
GLOBAL PCNET_ASM_INTHANDLER
PCNET_ASM_INTHANDLER:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	PCNET_IRQ
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

EXTERN RTL8139_IRQ
GLOBAL RTL8139_ASM_INTHANDLER
RTL8139_ASM_INTHANDLER:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	RTL8139_IRQ
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

EXTERN sb16_handler
GLOBAL asm_sb16_handler
asm_sb16_handler:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	sb16_handler
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
EXTERN rtc_handler
GLOBAL asm_rtc_handler
EXTERN ide_irq
GLOBAL asm_ide_irq
asm_ide_irq:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	ide_irq
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
asm_rtc_handler:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	rtc_handler
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
global asm_gui_api
extern Gui_API
asm_gui_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD			; 用于保存的PUSH
		PUSHAD			; 用于向Gui_API传值的PUSH
		MOV		AX,SS
		MOV			DS,AX ; 将操作系统用段地址存入DS和ES
		MOV		ES,AX
		call saveregisters
		CALL	Gui_API
		ADD		ESP,32
		POPAD
		call loadregisters
		POP		ES
		POP		DS
		IRETD
global asm_net_api
extern net_API
asm_net_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD			; 用于保存的PUSH
		PUSHAD			; 用于向net_API传值的PUSH
		MOV		AX,SS
		MOV			DS,AX ; 将操作系统用段地址存入DS和ES
		MOV		ES,AX
		call saveregisters
		CALL	net_API
		ADD		ESP,32
		POPAD
		call loadregisters
		POP		ES
		POP		DS
		IRETD
asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		add EAX,32
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		MOV EAX,0
		MOV   AX,CS
		PUSH  EAX
		CALL	inthandler20
		pop eax
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
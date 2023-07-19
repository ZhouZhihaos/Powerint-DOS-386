[BITS 32]
section .data
GLOBAL	asm_inthandler21, asm_inthandler20
EXTERN	inthandler21,inthandler20,inthandler36,inthandler2c
GLOBAL	asm_inthandler36,asm_inthandler2c,floppy_int
section .text
global null_inthandler
null_inthandler:
	IRETD

asm_inthandler36:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD			; 用于保存的PUSH
		PUSHAD
		MOV		AX,SS
		MOV		DS,AX ; 将操作系统用段地址存入DS和ES
		MOV		ES,AX
		CALL	inthandler36
		ADD		ESP,32
		POPAD
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
		IRETD
global asm_gui_api
extern Gui_API
asm_gui_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD			; 用于保存的PUSH
		PUSHAD
		MOV		AX,SS
		MOV		DS,AX ; 将操作系统用段地址存入DS和ES
		MOV		ES,AX
		CALL	Gui_API
		ADD		ESP,32
		POPAD
		POP		ES
		POP		DS
		IRETD
global asm_net_api
extern net_api
asm_net_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD			; 用于保存的PUSH
		PUSHAD
		MOV		AX,SS
		MOV		DS,AX ; 将操作系统用段地址存入DS和ES
		MOV		ES,AX
		CALL	net_api
		ADD		ESP,32
		POPAD
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

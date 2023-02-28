; boot.asm
; Copyright (C) zhouzhihao 2020-2022
bootseg		equ		7c0h
dataseg		equ		800h
readsize	equ		90			; DOSLDR.BIN的大小
HRBAT		equ		0x100000	; HRB格式文件的装载地址

jmp	short	start
; FAT12文件系统定义
	db	0x90
	db	"POWERINT"
	dw	512
	db	1
	dw	1
	db	2
rotentcnt:
	dw	224
	dw	2880
	db	0xf0
	dw	9
	dw	18
	dw	2
	dd	0
	dd	2880
drvnum:
	db	0
	db  0
	db  0x29
	dd	0xffffffff
	db	"POWERINTDOS"
	db	"FAT12   "

start:
; main
	
	mov	ax,bootseg
	mov	ds,ax
	mov	ax,dataseg
	mov	es,ax
	mov byte [drvnum],dl
	
	mov bx,[rotentcnt]
	mov ax,1
.longdiv:
	sub bx,16
	cmp bx,16
	jb .longdiv.end
	inc ax
	jmp .longdiv
.longdiv.end:
	add ax,19
	cmp byte[drvnum],0x80
	jne .chs
	mov [packet.lba],ax
	call read1sector
.chs:
	mov bl,2*18
	div bl
	mov ch,al	; cyline
	mov al,ah
	mov ah,0
	mov bl,18
	div bl
	mov dh,al	; header
	mov cl,ah	; sector
	inc cl
.readloop:
	call read1sector
	inc cl
	cmp cl,18+1
	jne .next
	mov cl,1
	inc dh
	cmp dh,1+1
	jne .next
	mov dh,0
	inc ch
.next:
	mov ax,es
	add ax,20h
	mov es,ax
	inc byte[read]
	cmp byte[read],readsize
	jne .readloop

.hrb:	; HRB格式文件
	; 1.让CPU支持1M以上内存、设置A20GATE
	in	al,92h
	or	al,00000010b
	out	92h,al

	; 2.禁止PIC外部中断
	cli
	
	; 3.设置临时GDT、切换保护模式
	mov	eax,0
	mov	ax,ds
	shl	eax,4
	mov	dword[GDTR0+2],GDT0
	add	[GDTR0+2],eax
	mov	ax,ds
	shl	ax,4
	add	ax,inprotectmode
	mov	[GDT0+3*8+2],ax	; 初始化inprotectmode代码GDT
	lgdt	[GDTR0]
	mov	eax,cr0
	or	eax,1
	mov	cr0,eax

	; 4.跳转到保护模式
	jmp	dword	3*8:0

read1sector:
; 读取1个扇区的通用程序
	cmp byte[drvnum],0
	je .floppy
	cmp byte[drvnum],0x80
	je .hard
.floppy:
	mov	di,0
.retry:
	mov	ah,02h
	mov	al,1
	mov	bx,0
	mov	dl,0
	int	13h
	jnc	.readok
	inc	di
	mov	ah,00h
	mov	dl,0
	int	13h
	cmp	di,5
	jne	.retry
	jmp $
.readok:
	ret
.hard:
	mov ah,42h
	mov dl,80h
	mov si,packet
	int 13h
	ret

[BITS 32]
inprotectmode:
	mov	ax,DataGDT
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax

	cmp byte[0x8001],0x30
	jne $
	
	; 1.传输HRB数据
	mov	eax,0
	mov	ax,dataseg	; 取出文件段地址
	shl	eax,4	; 段地址*16+偏移地址=物理地址

	mov	esi,eax
	mov	edi,HRBAT
	mov	ecx,readsize*512/4
	call	memcpy
	
	; 3.HRB剩余任务
	MOV	EBX,HRBAT
	MOV	ECX,[EBX+16]
	ADD	ECX,3			; ECX += 3;
	SHR	ECX,2			; ECX /= 4;
	JZ	.skip			; 传输完成
	MOV	ESI,[EBX+20]	; 源
	ADD	ESI,EBX
	MOV	EDI,[EBX+12]	; 目标
	CALL	memcpy
.skip:
	MOV	ESP,[EBX+12]	; 堆栈的初始化
	; 4.跳转
	finit
	;cvttss2si eax, [esp+4] 
	mov eax, cr0
	and ax, 0xFFFB		;clear coprocessor emulation CR0.EM
	or ax, 0x2			;set coprocessor monitoring  CR0.MP
	mov cr0, eax
	mov eax, cr4
	or ax, 3 << 9		;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
	mov cr4, eax
	jmp	dword	4*8:0x0000001b

memcpy:
	mov	eax,[esi]
	mov	[edi],eax
	add	esi,4
	add	edi,4
	dec	ecx
	jnz	memcpy
	ret

read db	0
packet:
	db	10h
	db	0
	dw	readsize
.off:
	dw	0
.seg:
	dw	dataseg
.lba:
	dd	0
	dd	0
; 临时用GDT
GDT0:
NullGDT	equ	$-GDT0
	dd	0,0
DataGDT	equ	$-GDT0
	; 全局数据GDT
	dw	0xffff
	dw	0
	db	0
	db	0x92
	db	0xcf
	db	0
Code1GDT	equ	$-GDT0
	; 跳转代码GDT
	dd 0,0
Code2GDT	equ	$-GDT0
	; inprotectmode代码GDT
	dw	0xffff
	dw	0
	db	0
	db	0x9a
	db	0xcf
	db	0
Code3GDT	equ	$-GDT0
	; 跳转代码GDT
	dw	0xffff
	dw	0
	db	0x10
	db	0x9a
	db	0x47
	db	0
GDTR0:
	dw	GDTR0-GDT0	; 临时GDT的大小
	dd	GDT0	; 临时GDT的地址

times	510-($-$$)	db	0
db	0x55,0xaa

; boot.asm
; Copyright (C) zhouzhihao 2020-2022
; Floppy Disk Drive
numsector	equ		18	; 最大扇区
numheader	equ		1	; 最大磁头
numcylind	equ		39	; 最大柱面
bootseg		equ		7c0h
dataseg		equ		800h

jmp	short	start	
; FAT12文件系统定义
	db	0x90
	db	"POWERINT"
	dw	512
	db	1
	dw	1
	db	2
	dw	224
	dw	2880
	db	0xf0
	dw	9
	dw	18
	dw	2
	dd	0
	dd	2880
	db	0,0,0x29
	dd	0xffffffff
	db	"POWERINTDOS"
	db	"FAT12   "

start:
; main
	mov	ax,bootseg
	mov	ds,ax
	mov	ax,dataseg
	mov	es,ax

	call	floppyload	; 读软盘
	call	findloader	; 巡查文件
	
	call	newline
	
	jmp	far	[jmpseg]
	; 将控制权交给jmpseg处（DOSLDR）

floppyload:
; 读软盘
	call	read1sector
	mov	ax,es
	add	ax,20h
	mov	es,ax
	; 读扇区
	inc	byte[sector]
	cmp	byte[sector],numsector+1
	jne	floppyload
	mov	byte[sector],1
	; 读磁头
	inc	byte[header]
	cmp	byte[header],numheader+1
	jne	floppyload
	mov	byte[header],0
	; 读柱面
	inc byte[cylind]
	cmp	byte[cylind],numcylind
	jne	floppyload
	
	ret

read1sector:
; 读取1个扇区的通用程序
	mov	ch,[cylind]
	mov	dh,[header]
	mov	cl,[sector]
	
	mov	di,0
.retry:
	mov	ah,02h
	mov	al,1
	mov	bx,0
	mov	dl,80h
	int	13h
	jnc	.readok
	inc	di
	mov	ah,00h
	mov	dl,80h
	int	13h
	cmp	di,5
	jne	.retry

	mov	si,readerror
	call	putstr
	call	newline

.readok:
	ret

findloader:
; 巡查是否有DOSLDR
	mov	ax,0a60h
	mov	es,ax
	mov	si,0
	mov	cx,11
.cmp:
	mov	al,[es:si]
	mov	ah,[loaderbin+si]
	cmp	al,ah
	jne	.nextfile
	inc	si
	loop	.cmp
	mov	ax,es
	add	ax,1h
	mov	es,ax
	mov	cx,[es:10]
	sub	ax,ax
.mul:
	add	ax,20h
	loop	.mul
	add	ax,0be0h
	mov	[jmpseg+2],al
	mov	[jmpseg+3],ah
	ret
.nextfile:
	mov	ax,es
	add	ax,2h
	mov	es,ax
	sub	si,si
	mov	al,[es:si]
	cmp	al,0
	je	.end
	mov	cx,11
	jmp	.cmp
.end:
	call	newline
	mov	si,errormsg
	call	putstr
	jmp	$	; 如果没有就死循环

putstr:
; 打印字符串
	mov	al,[si]
	cmp	al,0	; 如果[SI]=0
	je	.end	; 就结束
	mov	ah,0eh
	int	10h
	inc	si
	jmp	putstr
.end:
	ret

newline:
; 换行
	mov	ah,0eh
	mov	al,0dh
	int	10h
	mov	al,0ah
	int	10h
	ret

loaderbin	db	'DOSLDR     '
cylind		db	0
header		db	0
sector		db	1
errormsg	db	'Boot Error: No DOSLDR in Drive C.',0
readerror	db	'Read Hard Disk Error.',0
jmpseg		db	0,0,0,0
db	numcylind
db	'A'

;times	508-($-$$)	db	0
times	510-($-$$)	db	0
;db	numcylind
;db	'C'
db	0x55,0xaa
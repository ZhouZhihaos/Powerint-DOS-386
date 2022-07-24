; dosldr.asm
; Copyright (C) zhouzhihao 2022
DISKIMG0	equ	0x8000		; 实模式下磁盘数据装载地址
DISKIMG1	equ	0x100000	; 保护模式下磁盘数据装载地址
HRBAT		equ	0x280000	; HRB格式文件的装载地址
COFFAT		equ	0x35000		; COFF格式文件的装载地址
jmp	short	start	; 两字节
start:
	mov	ax,cs
	mov	ds,ax
	mov	ax,0
	mov	es,ax
	
	; 参数的初始化
	mov	word[menulstaddr],0
	mov	word[sinum],0
	mov	byte[lastflag],0
	mov	byte[number],0
	mov	byte[systemnum],31h

	mov	ah,0
	mov	al,3
	int	10h
	; 加载MENU.LST
	mov	di,menulst
	call	findfile
	mov	[menulstaddr],ax
	cmp	word[menulstaddr],0	; 没找到文件？
	jne	.findit
	; 没找到MENU.LST 进入LDRSHELL
	call	newline
	mov	ax,0
	mov	es,ax
	mov	al,[es:0x7d78]
	mov	[errormsg+29],al
	mov	si,errormsg
	call	putstr
	call	newline
	jmp	ldrshell
.findit:
	mov	si,menuput
	call	putstr
	call	newline
	mov	word[sinum],0	; 初始化SI=0
.again2:
	mov	si,[sinum]	; 提取SI
	mov	di,0
	mov	ax,[menulstaddr]
	mov	es,ax
	mov	cx,128
.loop:
	mov	al,[es:si]
	cmp	al,0dh
	je	.out
	cmp	al,0
	je	.out2
	mov	[cmdline+di],al
	inc	si
	inc	di
	loop	.loop
.out2:
	mov	byte[lastflag],1
.out:
	add	si,2	; 避开0x0a
	mov	[sinum],si	; 保存SI
	call	command
	mov	al,0
	mov	si,cmdline
	mov	cx,128
	call	clean
	cmp	byte[lastflag],1
	jne	.again2
	jmp	ldrshell

waitinput:
; 等待输入引导系统号
	mov	si,inputsystem
	call	putstr
.wait:
	mov	ah,0
	int	16h
	mov	bl,[systemnum]
	dec	bl
	mov	cx,9
.loopcmp:
	cmp	bl,al
	je	.yes
	dec	bl
	cmp	bl,'0'
	je	.wait
	jmp	.loopcmp
.yes:
	mov	ah,0eh
	int	10h
	sub	al,30h
	mov	[number],al
	mov	ax,[menulstaddr]
	mov	es,ax
	mov	si,0
.again:
	push	si
	mov	si,cmdline
	mov	cx,128
	mov	al,0
	call	clean
	pop	si
	mov	di,0
	mov	cx,7
.find:
	mov	al,[es:si]
	cmp	byte[systemcom+di],al
	jne	.nextline
	inc	si
	inc	di
	loop	.find
	dec	byte[number]
	jz	.find2
.nextline:
	mov	al,[es:si]
	cmp	al,0ah
	je	.gagain
	inc	si
	jmp	.nextline
.gagain:
	inc	si	; 直接指向这一行的起始
	jmp	.again
.find2:
	mov	al,[es:si]
	cmp	al,0ah
	je	.gok
	inc	si
	jmp	.find2
.gok:
	inc	si	; 直接指向这一行的起始
.ok:
	inc	si	; 跳过' '
	mov	di,0
	mov	cx,128
.loop:
	mov	al,[es:si]
	cmp	al,0dh
	je	.out
	cmp	al,0
	je	.out
	mov	[cmdline+di],al
	inc	si
	inc	di
	loop	.loop
.out:
	add	si,2	; 避开0x0a
	push	si
	call	command
	mov	si,cmdline
	mov	cx,128
	mov	al,0
	call	clean
	pop	si
	mov	ax,[menulstaddr]
	mov	es,ax
	jmp	.ok

boot:
	; 判断文件类型
	mov	ax,0
	mov	es,ax
	mov	ax,[es:0xff0]
	mov	es,ax
	cmp	byte[es:4],'H'
	jne	.coff
	cmp	byte[es:5],'a'
	jne	.coff
	cmp	byte[es:6],'r'
	jne	.coff
	cmp	byte[es:7],'i'
	jne	.coff
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
	mov	ax,cs
	shl	ax,4
	add	ax,inprotectmode
	mov	[GDT0+3*8+2],ax	; 初始化inprotectmode代码GDT
	lgdt	[GDTR0]
	mov	eax,cr0
	or	eax,1
	mov	cr0,eax
	
	; 4.跳转到保护模式
	jmp	dword	3*8:0
.coff:	; COFF格式文件
	push	ax
	; BootInfo赋值
	mov	ax,0
	mov	es,ax
	mov	al,[es:0x7d78]
	mov	[es:0x7e00],al
	cmp	al,'C'
	jae	.harddiskdrive
	sub	al,'A'
	jmp	.move
.harddiskdrive:
	sub	al,'C'
	add	al,0x80
.move:
	mov	[es:0x7e01],al
	pop	ax
	mov	ds,ax
	mov	si,0
	mov	ax,COFFAT/0x10
	mov	es,ax
	mov	di,0
	mov	cx,0xffff
	call	memcpy16
	jmp	dword	COFFAT/0x10:0
	
command:
	cmp	byte[cmdline],' '
	je	.ret
	mov	si,cmdline
	mov	di,waitcom
	mov	cx,4
	call	memcmp
	cmp	al,1
	je	.waitinput
	mov	si,cmdline
	mov	di,bootcom
	mov	cx,4
	call	memcmp
	cmp	al,1
	je	boot
	mov	si,cmdline
	mov	di,filecom
	mov	cx,5
	call	memcmp
	cmp	al,1
	je	file
	mov	si,cmdline
	mov	di,systemcom
	mov	cx,7
	call	memcmp
	cmp	al,1
	je	system
	mov	si,cmdline
	mov	di,shellcom
	mov	cx,5
	call	memcmp
	cmp	al,1
	je	.shell
	mov	si,cmdline
	mov	di,dircom
	mov	cx,3
	call	memcmp
	cmp	al,1
	je	dir
	; 不是命令就打印错误并转至LDRSHELL
	call	newline
	mov	si,undefinecom
	call	putstr
	add	sp,8	; 舍弃栈中CS,IP数据
.shell:
	call	newline
	mov	si,ldrshellver
	call	putstr
	call	newline
	jmp	ldrshell
.ret:
	ret
.waitinput:
	add	sp,8
	jmp	waitinput

system:
	mov	al,[systemnum]
	mov	ah,0eh
	int	10h
	mov	al,'.'
	int	10h
	mov	si,cmdline
	add	si,7
.loop:
	mov	al,[si]
	cmp	al,0dh
	je	.out
	cmp	al,0
	je	.out
	mov	ah,0eh
	int	10h
	inc	si
	jmp	.loop
.out:
	call	newline
	inc	byte[systemnum]
	ret

file:
	mov	si,5
	mov	dh,20h
	call	filenamecpy
	mov	di,findfilename
	call	findfile
	ret

dir:
	call	newline
	mov	ax,0xa60
	mov	es,ax
.again:
	mov	si,0
.put:
	mov	al,[es:si]
	cmp	al,0
	je	.putok
	mov	ah,0eh
	int	10h
	inc	si
	jmp	.put
.putok:
	mov	ax,es
	add	ax,2h
	mov	es,ax
	call	newline
	cmp	byte[es:0],0
	je	.ret
	jmp	.again
.ret:
	ret

findfile:
; 查找文件
	mov	ax,0xa60
	mov	es,ax
	mov	si,0
	mov	cx,12
.cmp:
	mov	al,[es:si]
	mov	ah,[di]
	cmp	al,ah
	jne	.nextfile
	inc	si
	inc	di
	loop	.cmp
	; 找到了算出段地址
	mov	ax,es
	add	ax,1
	mov	es,ax
	mov	cx,[es:10]
	sub	ax,ax
.mul:
	add	ax,20h
	loop	.mul
	add	ax,0xbe0
	mov	dx,0
	mov	es,dx
	mov	[es:0xff0],ax	; 段地址暂存到0xff0
	ret
.nextfile:
	mov	ax,es
	add	ax,2h
	mov	es,ax
	sub	si,si
	sub	di,si
	mov	al,[es:si]
	cmp	al,0
	je	.end
	mov	cx,11
	jmp	.cmp
.end:
	mov	ax,0
	ret

filenamecpy:
; 1.将cmdline中"*.*"类文件名转化成"*       *  "类文件名（"a.txt"->"A       TXT"）
; 2.将转化后的文件名写入FindFileName中
; 寄存器：in:SI/DH
	mov	di,0
	mov	cx,11
.strcpy:	; 循环一：将FindFileName全部归' '
	mov	byte[findfilename+di],' '
	inc	di
	loop	.strcpy
	mov	byte[findfilename+11],dh	; 文件属性
	mov	di,0
	mov	cx,8
.strcpy2:	; 循环二：将文件名复制到FindFileName前8位中
	mov	ah,[cmdline+si]
	cmp	ah,'.'
	je	.spot
	cmp	ah,0
	je	.zero
	cmp	ah,' '
	je	.zero
	and	ah,11011111b	; 防止小写错误
	mov	[findfilename+di],ah
	inc	si
	inc	di
	loop	.strcpy2
.strcpy3r:	; 循环三：跳过'.' 将后缀名复制到FindFileName后3位中
	mov	cx,3
	mov	di,8
	inc	si	; 跳过'.'
.strcpy3:
	mov	ah,[cmdline+si]
	cmp	ah,0
	je	.strcpyend
	and	ah,11011111b
	mov	[findfilename+di],ah
	inc	si
	inc	di
	loop	.strcpy3
.strcpyend:
	ret
.spot:	; 从循环二跳来 如果后缀点
	cmp	dh,10h
	je	.spota
	jmp	.strcpy3r
.spota:		; 'cd .?'的情况
	mov	byte[findfilename+0],'.'
	inc	si
	cmp	byte[cmdline+si],'.'
	jne	.strcpyend	; 'cd .'
	mov	byte[findfilename+1],'.'	; 'cd ..'
	jmp	.strcpyend
.zero:	; 从循环二跳来 如果没有后缀点
	mov	cx,8
	sub	cx,di
.zeroloop:
	mov	byte[findfilename+di],' '
	inc	di
	loop	.zeroloop
;	jmp	.strcpy3r
	ret

ldrshell:
stackseg		equ		0
stacktop		equ		7c00h
	mov	ax,stackseg
	mov	ss,ax
	mov	sp,stacktop
.again:
	call	newline
	mov	al,'>'
	mov	ah,0eh
	int	10h
	mov	si,0
.usrinput:
; LDRSHELL中的输入
	cmp	si,128
	je	.enter
	mov	ah,0
	int	16h
	mov	ah,0eh
	int	10h
	cmp	al,0dh
	je	.enter
	cmp	al,08h
	je	.backspace
	mov	[cmdline+si],al
	inc	si
	jmp	.usrinput
.backspace:
	mov	ah,0eh
	mov	al,' '
	int	10h
	mov	al,08h
	int	10h
	dec	si
	mov	byte[cmdline+si],0
	jmp	.usrinput
.enter:
	call	newline
	call	command
	mov	al,0
	mov	si,cmdline
	mov	cx,128
	call	clean
	jmp	.again

memcmp:
; 数据比较
	mov	al,[si]
	cmp	byte[di],al
	jne	.no
	inc	di
	inc	si
	loop	memcmp
	mov	al,1
	ret
.no:
	mov	al,0
	ret

memcpy16:
	mov	al,[si]
	mov	[es:di],al
	inc	si
	inc	di
	loop	memcpy16
	ret

clean:
; 填充
	mov	[si],al
	inc	si
	loop	clean
	ret

putstr:
; 打印字符串
	mov	al,[si]
	cmp	al,0	; 如果[SI]='$'
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

usernamein	db	'Username:',0
passwordin	db	'Password:',0
inputtemp1	times	8	db	' '
inputtemp2	times	16	db	' '
errorput	db	'Username or Password error.',0
menuput		db	'DOSLDR boot menu(menu.lst):',0
inputsystem	db	'Please choose your boot system:',0
menulst		db	'MENU    LST',20h
menulstaddr	dw	0
sinum		dw	0
; menulstaddr:sinum （文件指数）
lastflag	db	0
systemcom	db	'system '
systemnum	db	31h	; ASCII码
waitcom		db	'wait'
number		db	0
filecom		db	'file '
bootcom		db	'boot'
shellcom	db	'shell'
dircom		db	'dir'
findfilename	db	'           ',0
undefinecom	db	'Undefinded command.',0ah,0dh,0
errormsg:
	db	'Boot Error: No MENU.LST in Drive ?.',0dh,0ah,0dh,0ah
ldrshellver:
	db	'LDRSHELL Version 0.1',0dh,0ah
	db	'Copyright (C) zhouzhihao 2022',0
cmdline			times	128	db	0

[BITS 32]
inprotectmode:


	mov	ax,DataGDT
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax
	
	; 1.传输HRB数据
	mov	eax,0
	mov	ax,[0xff0]	; 取出文件段地址
	shl	eax,4	; 段地址*16+偏移地址=物理地址
	mov	esi,eax
	mov	edi,HRBAT
	mov	ecx,512*1024/4
	call	memcpy
	
	; 2.传输磁盘数据
	mov	esi,DISKIMG0
	mov	edi,DISKIMG1
	mov	ecx,0
	mov	cl,[0x7d77]	; numcyline（读的扇区数）
	imul	ecx,512*18*2/4
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
	jmp	dword	2*8:0x0000001b

memcpy:
	mov	eax,[esi]
	mov	[edi],eax
	add	esi,4
	add	edi,4
	dec	ecx
	jnz	memcpy
	ret

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
	dw	0xffff
	dw	0
	db	0x28
	db	0x9a
	db	0x47
	db	0
Code2GDT	equ	$-GDT0
	; inprotectmode代码GDT
	dw	0xffff
	dw	0
	db	0
	db	0x9a
	db	0xcf
	db	0
GDTR0:
	dw	GDTR0-GDT0	; 临时GDT的大小
	dd	GDT0	; 临时GDT的地址
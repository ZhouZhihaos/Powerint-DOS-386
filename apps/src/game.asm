;File name:game.asm
;Create Time:2022/1/25
;Copyright Min0911 & zhouzhihao & YAOHANHONG 2022
;System/API: PowerintDOS 386 0.5a
[bits 32]
db	'A'	; 表示程序是汇编写的
jmp	GameMain
CleanScreen		db	'clear',0
WelcomePut		db	'Welcome to our game!',0
MessageXYBuffer	db	0,0
ClosePut		db	'                       ',0
Q1				db	'1.What',39,'s your name?',0
InputQ1Put		db	'Please input your name here:',0
NameBuffer		times	17	db	0
HelloPut		db	'Oh!Hello, ',0
Q2				db	'2.What',39,'s your gender?',0
InputQ2Put		db	'Please kick here:BOY or GIRL',0
GameMain: ;游戏主函数
	mov	eax,0x19
	mov	edx,CleanScreen
	int	36h
	;先清屏
	
	call	Draw_Basic_UI
	
	mov	esi,WelcomePut
	mov	al,4fh
	mov	ah,0c3h
	call	MessageBox
	
.Input:
	mov	eax,0x16
	mov	ebx,0x01
	int	36h
	cmp	dl,0x0a
	jne	.Input
	call	CloseMessageBox
	
	mov	esi,Q1
	mov	al,4fh
	mov	ah,0c3h
	call	MessageBox
	
	mov	edx,InputQ1Put
	mov	eax,0x05
	int	36h
	mov	eax,0x16
	mov	ebx,0x03
	mov	ecx,16
	mov	edx,NameBuffer
	int	36h
	
	inc	byte[MessageXYBuffer]
	call	CloseMessageBox
	mov	eax,0x05
	mov	edx,HelloPut
	int	36h
	mov	eax,0x05
	mov	edx,NameBuffer
	int	36h
	call	NextLine
	
	mov	esi,Q2
	mov	al,4fh
	mov	ah,0c3h
	call	MessageBox

	mov	edx,InputQ2Put
	mov	eax,0x05
	int	36h
.Mouse:
	mov	eax,0x0f
	int	36h
	cmp	edx,2
	jne	.Mouse
	cmp	ecx,17
	je	.BOY
	cmp	ecx,18
	je	.BOY
	cmp	ecx,19
	je	.BOY
	cmp	ecx,24
	je	.GIRL
	cmp	ecx,25
	je	.GIRL
	cmp	ecx,26
	je	.GIRL
	cmp	ecx,27
	je	.GIRL
	jmp	.Mouse
.BOY:
.GIRL:
	mov	eax,0x1e
	int	36h
	jmp $

MessageBox:
; 窗口提示
; IN:ESI(提示的字符串),AL(窗口外色),AH(窗口内色)
	push	esi
	push	eax
	mov	eax,0x0e
	int	36h
	mov	[MessageXYBuffer],cl
	mov	[MessageXYBuffer+1],dl
	mov	ebx,9
	mov	ecx,28
	mov	edx,16
	mov	esi,52
	pop	eax
	push	eax
	and	eax,0x000000ff
	mov	edi,eax
	mov	eax,0x0c
	int	36h
	pop	eax
	and	eax,0x0000ff00
	shr	eax,8
	mov	edi,eax
	mov	ebx,10
	mov	ecx,29
	mov	edx,15
	mov	esi,51
	mov	eax,0x0c
	int 36h
	mov	eax,0x04
	mov	ecx,12
	mov	edx,29
	int	36h
	pop	esi
	mov	edx,esi
	mov	eax,0x05
	int	36h
	mov	eax,0x04
	mov	cl,[MessageXYBuffer]
	mov	dl,[MessageXYBuffer+1]
	and	ecx,0x000000ff
	and	edx,0x000000ff
	int	36h
	ret

CloseMessageBox:
; 关闭窗口
	mov ebx,9
	mov ecx,28
	mov edx,16
	mov esi,52
	mov edi,0x07
	mov eax,0x0c
	int 0x36
	mov	eax,0x04
	mov	ecx,12
	mov	edx,29
	int	36h
	mov	edx,ClosePut
	mov	eax,0x05
	int	36h
	mov	eax,0x04
	mov	cl,[MessageXYBuffer]
	mov	dl,[MessageXYBuffer+1]
	and	ecx,0x000000ff
	and	edx,0x000000ff
	int	36h
	ret

NextLine:
; 换行
	mov	eax,0x02
	mov	dl,0x0d
	and	edx,0x000000ff
	int	36h
	mov	dl,0x0a
	int	36h
	ret

Draw_Basic_UI:
; 画界面
	mov ebx,0
	mov ecx,0
	mov edx,25
	mov esi,80
	mov edi,0x07
	mov eax,0x0c
	int 0x36
	ret
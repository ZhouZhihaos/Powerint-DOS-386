[BITS 32]
db	'A'	; 表示程序是汇编写的
jmp start
cleanscreen	db	'cls',0
map	times	19*19	db	' '
playerx		db	0
playery		db	0
dobb		db	0
Owinput		db	'O',39,'s player is win!',0
@winput		db	'@',39,'s player is win!',0
start:
	mov	eax,19h
	mov	edx,cleanscreen
	int	36h
.loop:
	call	draw_map
	call	draw_player
	mov	eax,16h
	mov	ebx,2
	int	36h
	cmp	dl,48h
	je	.up
	cmp	dl,50h
	je	.down
	cmp	dl,4bh
	je	.left
	cmp	dl,4dh
	je	.right
	cmp	dl,1ch
	je	.inputok
	jmp	.loop
.inputok:
	mov	al,[playery]
	mov	bl,19
	mul	bl
	mov	bx,ax
	mov	al,[playerx]
	mov	ah,0
	add	bx,ax
	cmp	byte[dobb],0
	je	.O
	cmp	byte[dobb],1
	je	.@
.next:
	xor	byte[dobb],1
	call	win_cmp
	jmp	.loop
.O:
	mov	byte[map+bx],'O'
	jmp	.next
.@:
	mov	byte[map+bx],'@'
	jmp	.next
.up:
	cmp	byte[playery],0
	je	.loop
	dec	byte[playery]
	jmp	.loop
.down:
	cmp	byte[playery],18
	je	.loop
	inc	byte[playery]
	jmp	.loop
.left:
	cmp	byte[playerx],0
	je	.loop
	dec	byte[playerx]
	jmp	.loop
.right:
	cmp	byte[playerx],18
	je	.loop
	inc	byte[playerx]
	jmp	.loop

draw_map:
; 画棋盘
	mov	eax,4
	mov	ecx,0
	mov	edx,0
	int	36h
	mov	si,0
	mov	di,0
	mov	cx,19
.loop1:
	push	cx
	mov	cx,19
.loop2:
	push	si
	add	si,di
	mov	dl,[map+si]
	pop	si
	mov	eax,2
	int	36h
	inc	si
	loop	.loop2
	add	di,19
	mov	si,0
	mov	eax,2
	mov	dl,0ah
	int	36h
	pop	cx
	loop	.loop1
	ret

draw_player:
; 画光标
	mov	eax,4
	mov	ecx,0
	mov	edx,0
	mov	cl,[playery]
	mov	dl,[playerx]
	int	36h
	mov	eax,2
	mov	dl,'>'
	int	36h
	mov	eax,4
	mov	ecx,0
	mov	edx,0
	mov	cl,[playery]
	mov	dl,[playerx]
	int	36h
	ret

win_cmp:
; 判断胜负
	mov	si,0
	mov	di,0
	mov	cx,19
.loop1:
	push	cx
	mov	cx,19
.loop2:
	push	si
	add	si,di
	; 横排
	mov	al,[map+si]
	mov	ah,[map+si+1]
	mov	bl,[map+si+2]
	mov	bh,[map+si+3]
	mov	dl,[map+si+4]
	call	cmp5
	cmp	dh,1
	je	.win
	; 竖排
	mov	al,[map+si]
	mov	ah,[map+si+19]
	mov	bl,[map+si+19*2]
	mov	bh,[map+si+19*3]
	mov	dl,[map+si+19*4]
	call	cmp5
	cmp	dh,1
	je	.win
	; 正斜排
	mov	al,[map+si]
	mov	ah,[map+si+19+1]
	mov	bl,[map+si+19*2+2]
	mov	bh,[map+si+19*3+3]
	mov	dl,[map+si+19*4+4]
	call	cmp5
	cmp	dh,1
	je	.win
	; 反斜排
	mov	al,[map+si]
	mov	ah,[map+si+19-1]
	mov	bl,[map+si+19*2-2]
	mov	bh,[map+si+19*3-3]
	mov	dl,[map+si+19*4-4]
	call	cmp5
	cmp	dh,1
	je	.win
	pop	si
	inc	si
	dec	cx
	jecxz	.nextloop
	jmp	.loop2
.nextloop:
	add	di,19
	mov	si,0
	pop	cx
	dec	cx
	jecxz	.ret
	jmp	.loop1
.ret:
	ret
.win:
	add	esp,8
	mov	eax,4
	mov	ecx,19
	mov	edx,0
	int	36h
	cmp	byte[dobb],1
	je	.Owin
	cmp	byte[dobb],0
	je	.@win
.Owin:
	mov	edx,Owinput
	mov	eax,5
	int	36h
	mov	eax,0x1e
	int	36h
	jmp $
.@win:
	mov	edx,@winput
	mov	eax,5
	int	36h
	mov	eax,0x1e
	int	36h
	jmp $

cmp5:
; 判断5子是否连珠
	cmp	al,'@'
	jne	.l2
	jmp	.next
.l2:
	cmp	al,'O'
	jne	.no
.next:
	cmp	al,ah
	jne	.no
	cmp	ah,bl
	jne	.no
	cmp	bl,bh
	jne	.no
	cmp	bh,dl
	jne	.no
	mov	dh,1
	ret
.no:
	mov	dh,0
	ret
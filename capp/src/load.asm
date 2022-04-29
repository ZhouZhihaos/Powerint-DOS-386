[BITS 32]
db	'C'	; 表示程序是C写的
call start+0x1b
mov	eax,0x06
mov	edx,50	; 等待子进程退出
int	36h
mov	eax,0x1e	; 返回系统
int 36h
jmp $
start:
; 格式化的MBR样例
jmp	short	start
; FAT12文件系统定义
	db	0x90
	db	"POWERINT"
	dw	512
	db	0
	dw	1
	db	2
	dw	0
	dw	0
	db	0xf0
	dw	9
	dw	18
	dw	2
	dd	0
	dd	0
	db	0,0,0x29
	dd	0xffffffff
	db	"POWERINTDOS"
	db	"FAT12   "
start:
    jmp $

times	510-($-$$)	db	0
db	0x55,0xaa

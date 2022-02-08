[FORMAT "WCOFF"]				
[INSTRSET "i486p"]
[BITS 32]				
[FILE "api.asm"]
GLOBAL _putch,_putstr,_getch,_get_xy,_Text_get_mouse,_goto_xy
GLOBAL _SwitchTo320X200X256,_SwitchToText8025,_Draw_Char,_sleep
GLOBAL _PrintChineseChar,_PrintChineseStr,_Draw_Str,_malloc,_free
GLOBAL _print,_scan,_system,_filesize,_fopen,_bmpview
GLOBAL _Draw_Box,_Draw_Px,_Text_Draw_Box,_Grap_get_mouse
GLOBAL _input_char_inSM,_beep,_RAND,_GetCmdline
GLOBAL _drivers_idehdd_read,_drivers_idehdd_write
[SECTION .text]
_putch:
push	edx
push	eax
mov edx,[esp+12]
mov eax,0x02
int 36h
pop	eax
pop	edx
ret

_putstr:
push	edx
push	eax
mov edx,[esp+12]
mov eax,0x05
int 36h
pop	eax
pop	edx
ret

_getch:
push	edx
push	ebx
mov eax,0x16
mov ebx,0x1
int 36h
mov eax,edx
pop	ebx
pop	edx
ret

_get_xy:
push	edx
push	ecx
mov eax,0x0e
int 36h
mov ax,dx	; x
shl	eax,16
mov	ax,cx	; y
pop	ecx
pop	edx
ret

_Text_get_mouse:
push	ecx
push	edx
push	esi
mov	eax,0x0f
int	36h
mov	al,cl	; x
mov	ah,dl	; y
shl	eax,16
mov	ax,si	; btn
pop	esi
pop	edx
pop	ecx
ret

_goto_xy:
push	edx
push	ecx
push	eax
mov	edx,[esp+16]
mov	ecx,[esp+20]
mov	eax,0x04
int	36h
pop	eax
pop	ecx
pop	edx
ret

_SwitchTo320X200X256:
push	eax
push	ebx
mov	eax,0x03
mov	ebx,0x02
int	36h
pop	ebx
pop	eax
ret

_SwitchToText8025:
push	eax
push	ebx
mov	eax,0x03
mov	ebx,0x01
int	36h
pop	ebx
pop	eax
ret

_Draw_Char:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
mov	eax,0x03
mov	ebx,0x03
mov	ecx,[esp+28]
mov	edx,[esp+32]
mov	esi,[esp+36]
mov	edi,[esp+40]
int	36h
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_sleep:
push	eax
push	ebx
mov	eax,0x06
mov	edx,[esp+12]
int	36h
pop	ebx
pop	eax
ret

_PrintChineseChar:
push	eax
push	ebx
push	ecx
push	edx
push	edi
push	esi
mov	eax,0x03
mov	ebx,0x04
mov	ecx,[esp+28]
mov	edx,[esp+32]
mov	edi,[esp+36]
mov	esi,[esp+40]
int	36h
pop	esi
pop	edi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_Draw_Str:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
mov	eax,0x03
mov	ebx,0x07
mov	ecx,[esp+28]
mov	edx,[esp+32]
mov	esi,[esp+36]
mov	edi,[esp+40]
int	36h
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_PrintChineseStr:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
mov	eax,0x03
mov	ebx,0x08
mov	ecx,[esp+28]
mov	edx,[esp+32]
mov	edi,[esp+36]
mov	esi,[esp+40]
int	36h
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_print:
push	eax
push	ebx
mov	eax,0x05
mov	edx,[esp+12]
int	36h
pop	ebx
pop	eax
ret

_scan:
push	eax
push	ebx
push	ecx
push	edx
mov	eax,0x16
mov	ebx,0x03
mov	edx,[esp+20]
mov	ecx,[esp+24]
int	36h
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_malloc:
push	ecx
push	edx
mov	eax,0x08
mov	ecx,[esp+4]
int	36h
mov	eax,edx
pop	edx
pop	ecx
ret

_free:
push	eax
push	ecx
push	edx
mov	eax,0x09
mov	edx,[esp+16]
mov	ecx,[esp+20]
int	36h
pop	edx
pop	ecx
pop	eax
ret

_system:
push edx
push eax
mov eax,0x19
mov edx,[esp+12]
int 36h
pop eax
pop edx
ret

_filesize:
push	ebx
push	edx
mov	eax,0x1a
mov	ebx,0x01
mov	edx,[esp+12]
int	36h
mov	eax,edx
pop	edx
pop	ebx
ret

_fopen:
push	ebx
push	edx
push	esi
mov	eax,0x1a
mov	ebx,0x02
mov	edx,[esp+16]
mov	esi,[esp+20]
int	36h
pop	esi
pop	edx
pop	ebx
ret

_bmpview:
push	eax
push	edx
mov	eax,0x18
mov	edx,[esp+12]
int	36h
pop	edx
pop	eax
ret

_Draw_Box:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
push	ebp
mov	eax,0x03
mov	ebx,0x05
mov	ecx,[esp+32]
mov	edx,[esp+36]
mov	esi,[esp+40]
mov	edi,[esp+44]
mov	ebp,[esp+48]
int	36h
pop	ebp
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_Draw_Px:
push	eax
push	ebx
push	ecx
push	edx
push	esi
mov	eax,0x03
mov	ebx,0x06
mov	ecx,[esp+24]
mov	edx,[esp+28]
mov	esi,[esp+32]
int	36h
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_Text_Draw_Box:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
mov	eax,0x0c
mov	ecx,[esp+28]
mov	ebx,[esp+32]
mov	esi,[esp+36]
mov	edx,[esp+40]
mov	edi,[esp+44]
int	36h
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_Grap_get_mouse:
push	ecx
push	edx
push	esi
mov	eax,0x10
int	36h
mov	ax,cx	; x
shl	eax,16
mov	ax,si	; btn
mov	ah,dl	; y
pop	esi
pop	edx
pop	ecx
ret

_input_char_inSM:
push	ebx
push	edx
mov	eax,0x16
mov	ebx,0x02
int	36h
mov	eax,edx
pop	edx
pop	ebx
ret

_beep:
push	eax
push	ebx
push	ecx
push	edx
mov	eax,0x0d
mov	ebx,[esp+20]
mov	ecx,[esp+24]
mov	edx,[esp+28]
int	36h
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_RAND:
push edx
MOV AX, 32767   ;产生从1到AX之间的随机数
MOV DX, 41H ;用端口41H，或者上面说的其他端口也行
OUT DX, AX  ;有这句话后，我发现就可以产生从1到AX之间的随机数了
IN AL, DX   ;产生的随机数AL
bswap eax
MOV AX, 32767   ;产生从1到AX之间的随机数
MOV DX, 41H ;用端口41H，或者上面说的其他端口也行
OUT DX, AX  ;有这句话后，我发现就可以产生从1到AX之间的随机数了
IN AL, DX   ;产生的随机数AL
pop edx
ret

_GetCmdline:
push eax;4
push edx;8
mov eax,0x1b
mov edx,[esp+4+8]
int 36h
pop edx
pop eax
ret

_drivers_idehdd_read:
push	eax
push	ebx
push	ecx
push	edx
push	esi
mov	eax,0x17
mov	ebx,0x01
mov	ecx,[esp+24]
mov	edx,[esp+28]
mov	esi,[esp+32]
int	36h
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

_drivers_idehdd_write:
push	eax
push	ebx
push	ecx
push	edx
push	esi
mov	eax,0x17
mov	ebx,0x01
mov	ecx,[esp+24]
mov	edx,[esp+28]
mov	esi,[esp+32]
int	36h
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

[BITS 32]				
GLOBAL putch,putstr,getch,get_mouse,get_xy,goto_xy
GLOBAL SwitchTo320X200X256,SwitchToText8025,Draw_Char,sleep
GLOBAL PrintChineseChar,PrintChineseStr,Draw_Str,api_malloc,api_free
GLOBAL print,scan,system,filesize,api_ReadFile
GLOBAL Draw_Box,Draw_Px,Text_Draw_Box
GLOBAL input_char_inSM,beep,RAND,GetCmdline,Get_System_Version,Copy,_kbhit
GLOBAL mkdir,mkfile,Edit_File,SwitchTo320X200X256_BIOS,SwitchToText8025_BIOS
GLOBAL AddTask,SubTask,TaskForever,SendMessage,GetMessage,MessageLength,NowTaskID
GLOBAL exit
GLOBAL timer_alloc,timer_settime,timer_out,timer_free
GLOBAL haveMsg,PhyMemGetByte,GetMessageAll,PhyMemSetByte,format
GLOBAL get_hour_hex,get_min_hex,get_sec_hex,get_day_of_month,get_day_of_week,get_mon_hex,get_year,AddThread
GLOBAL TaskLock,TaskUnlock,SubThread,set_mode,VBEDraw_Px,VBEGet_Px,VBEGetBuffer,VBESetBuffer,roll,VBEDraw_Box
[SECTION .text]
putch:
push	edx
push	eax
mov edx,[ss:esp+12]
mov eax,0x02
int 36h
pop	eax
pop	edx
ret

putstr:
push	edx
push	eax
mov edx,[ss:esp+12]
mov eax,0x05
int 36h
pop	eax
pop	edx
ret

getch:
push	edx
push	ebx
mov eax,0x16
mov ebx,0x1
int 36h
mov eax,edx
pop	ebx
pop	edx
ret

get_mouse:
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

get_xy:
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

goto_xy:
push	edx
push	ecx
push	eax
mov	edx,[ss:esp+16]
mov	ecx,[ss:esp+20]
mov	eax,0x04
int	36h
pop	eax
pop	ecx
pop	edx
ret

SwitchTo320X200X256:
push	eax
push	ebx
mov	eax,0x03
mov	ebx,0x02
int	36h
pop	ebx
pop	eax
ret

SwitchToText8025:
push	eax
push	ebx
mov	eax,0x03
mov	ebx,0x01
int	36h
pop	ebx
pop	eax
ret

Draw_Char:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
mov	eax,0x03
mov	ebx,0x03
mov	ecx,[ss:esp+28]
mov	edx,[ss:esp+32]
mov	esi,[ss:esp+36]
mov	edi,[ss:esp+40]
int	36h
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

sleep:
push	eax
push	ebx
mov	eax,0x06
mov	edx,[ss:esp+12]
int	36h
pop	ebx
pop	eax
ret

PrintChineseChar:
push	eax
push	ebx
push	ecx
push	edx
push	edi
push	esi
mov	eax,0x03
mov	ebx,0x04
mov	ecx,[ss:esp+28]
mov	edx,[ss:esp+32]
mov	edi,[ss:esp+36]
mov	esi,[ss:esp+40]
int	36h
pop	esi
pop	edi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

Draw_Str:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
mov	eax,0x03
mov	ebx,0x07
mov	ecx,[ss:esp+28]
mov	edx,[ss:esp+32]
mov	esi,[ss:esp+36]
mov	edi,[ss:esp+40]
int	36h
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

PrintChineseStr:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
mov	eax,0x03
mov	ebx,0x08
mov	ecx,[ss:esp+28]
mov	edx,[ss:esp+32]
mov	edi,[ss:esp+36]
mov	esi,[ss:esp+40]
int	36h
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

print:
push	eax
push	ebx
mov	eax,0x05
mov	edx,[ss:esp+12]
int	36h
pop	ebx
pop	eax
ret

scan:
push	eax
push	ebx
push	ecx
push	edx
mov	eax,0x16
mov	ebx,0x03
mov	edx,[ss:esp+20]
mov	ecx,[ss:esp+24]
int	36h
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

api_malloc:
push	ecx
push	edx
mov	eax,0x08
mov	ecx,[ss:esp+12]
int	36h
mov	eax,edx
pop	edx
pop	ecx
ret

api_free:
push	eax
push	ecx
push	edx
mov	eax,0x09
mov	edx,[ss:esp+16]
mov	ecx,[ss:esp+20]
int	36h
pop	edx
pop	ecx
pop	eax
ret

system:
push edx
push eax
mov eax,0x19
mov edx,[ss:esp+12]
int 36h
pop eax
pop edx
ret

filesize:
push	ebx
push	edx
mov	eax,0x1a
mov	ebx,0x01
mov	edx,[ss:esp+12]
int	36h
mov	eax,edx
pop	edx
pop	ebx
ret

api_ReadFile:
push	ebx
push	edx
push	esi
mov	eax,0x1a
mov	ebx,0x02
mov	edx,[ss:esp+16]
mov	esi,[ss:esp+20]
int	36h
pop	esi
pop	edx
pop	ebx
ret

Draw_Box:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
push	ebp
mov	eax,0x03
mov	ebx,0x05
mov	ecx,[ss:esp+32]
mov	edx,[ss:esp+36]
mov	esi,[ss:esp+40]
mov	edi,[ss:esp+44]
mov	ebp,[ss:esp+48]
int	36h
pop	ebp
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

Draw_Px:
push	eax
push	ebx
push	ecx
push	edx
push	esi
mov	eax,0x03
mov	ebx,0x06
mov	ecx,[ss:esp+24]
mov	edx,[ss:esp+28]
mov	esi,[ss:esp+32]
int	36h
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

Text_Draw_Box:
push	eax
push	ebx
push	ecx
push	edx
push	esi
push	edi
mov	eax,0x0c
mov	ebx,[ss:esp+28]
mov	ecx,[ss:esp+32]
mov	edx,[ss:esp+36]
mov	esi,[ss:esp+40]
mov	edi,[ss:esp+44]
int	36h
pop	edi
pop	esi
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

input_char_inSM:
push	ebx
push	edx
mov	eax,0x16
mov	ebx,0x02
int	36h
mov	eax,edx
pop	edx
pop	ebx
ret

beep:
push	eax
push	ebx
push	ecx
push	edx
mov	eax,0x0d
mov	ebx,[ss:esp+20]
mov	ecx,[ss:esp+24]
mov	edx,[ss:esp+28]
int	36h
pop	edx
pop	ecx
pop	ebx
pop	eax
ret

RAND:
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

GetCmdline:
push eax;4
push edx;8
mov eax,0x1b
mov edx,[ss:esp+4+8]
int 36h
pop edx
pop eax
ret

Get_System_Version:
    push edx
    mov eax,0x01
    int 36h
    mov eax,edx
    pop edx
    ret
Copy:
    push	eax
    push    edx
    push    esi
    mov eax,0x1c
    mov edx,[ss:esp+4+8+4]
    mov esi,[ss:esp+8+8+4]
    int 36h
    pop esi
    pop edx
    pop eax
    ret
_kbhit:
    mov eax,0x1d
    int 36h
    ret

mkfile:
    push    eax   ;4
    push    ebx   ;8
    push    edx   ;12
    mov eax,0x1a ;文件系统API
    mov ebx,0x03 ;创建文件API
    mov edx,[ss:esp+4+12]
    int 36h
    pop edx
    pop ebx
    pop eax
    ret
mkdir:
    push    eax   ;4
    push    ebx   ;8
    push    edx   ;12
    mov eax,0x1a ;文件系统API
    mov ebx,0x04 ;创建文件API
    mov edx,[ss:esp+4+12]
    int 36h
    pop edx
    pop ebx
    pop eax
    ret
Edit_File:
    push eax ;4
    push ebx ;8
    push edx ;12
    push esi ;16
    push ecx ;20
    mov eax,0x1a ;文件系统API
    mov ebx,0x05 ;编辑文件API
    mov edx,[ss:esp+4+20] ;文件名
    mov esi,[ss:esp+8+20] ;编辑内容
    mov ecx,[ss:esp+12+20] ;编辑内容长度
	mov	edi,[ss:esp+12+24] ;编辑偏移地址
    int 36h
    pop ecx
    pop esi
    pop edx
    pop ebx
    pop eax
    ret

SwitchTo320X200X256_BIOS:
	push	eax
	push	ebx
	mov	eax,0x21
	mov	ebx,0x02
	int	36h
	pop	ebx
	pop	eax
	ret

SwitchToText8025_BIOS:
	push	eax
	push	ebx
	mov	eax,0x21
	mov	ebx,0x01
	int	36h
	pop	ebx
	pop	eax
	ret

AddTask:
	push	ebx
	push	ecx
	push	edx
	mov	eax,0x22
	mov	ebx,0x01
	mov	ecx,[ss:esp+20]
	mov	edx,[ss:esp+16]
	int	36h
	mov	eax,ecx
	pop	edx
	pop	ecx
	pop	ebx
	ret

SubTask:
	push	eax
	push	ebx
	push	ecx
	mov	eax,0x22
	mov	ebx,0x02
	mov	ecx,[ss:esp+16]
	int	36h
	pop	ecx
	pop	ebx
	pop	eax
	ret

TaskForever:
	push	eax
	push	ebx
	mov	eax,0x22
	mov	ebx,0x03
	int	36h
	pop	ebx
	pop	eax
	ret

SendMessage:
	push	eax
	push	ebx
	push	ecx
	push	edx
	push	esi
	mov	eax,0x22
	mov ebx,0x04
	mov ecx,[ss:esp+24]
	mov edx,[ss:esp+28]
	mov esi,[ss:esp+32]
	int 36h
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	eax
	ret

GetMessage:
	push	eax
	push	ebx
	push	ecx
	push	edx
	mov	eax,0x22
	mov ebx,0x05
	mov edx,[ss:esp+20]
	mov ecx,[ss:esp+24]
	int 36h
	pop	edx
	pop	ecx
	pop	ebx
	pop	eax
	ret
	
MessageLength:
	push	ebx
	push	ecx
	mov	eax,0x22
	mov ebx,0x06
	mov ecx,[ss:esp+12]
	int 36h
	pop	edx
	pop	ecx
	ret

NowTaskID:
	push	ebx
	mov	eax,0x22
	mov	ebx,0x07
	int	36h
	pop	ebx
	ret

exit:
	mov	eax,0x1e
	int 36h
	jmp	$

timer_alloc:
	push	eax
	push	ebx
	mov	eax,0x24
	mov	ebx,0x00
	int	36h
	pop	ebx
	pop	eax
	ret
timer_settime:
	push	eax
	push	ebx
	push	ecx
	mov	eax,0x24
	mov	ebx,0x01
	mov	ecx,[ss:esp+4+12]
	int	36h
	pop	ecx
	pop	ebx
	pop	eax
	ret
timer_out:
	push	ebx
	mov	eax,0x24
	mov	ebx,0x02
	int	36h
	pop	ebx
	ret
timer_free:
	push	eax
	push	ebx
	mov	eax,0x24
	mov	ebx,0x03
	int	36h
	pop	ebx
	pop	eax
	ret
haveMsg:
	push ebx
	mov eax,0x22
	mov ebx,0x08
	int 36h
	pop ebx
	ret
PhyMemGetByte:
	push	ds     ; 4
	mov   ax,1*8 ; 系统的数据段（可以访问到所有的内存）
	mov	  ds,ax
	mov   eax,[esp+4+4]
	mov	  al,[eax]
	pop   ds
	ret
GetMessageAll:
	push	eax
	push	ebx
	push	ecx
	push	edx
	mov	eax,0x22
	mov ebx,0x09
	mov edx,[ss:esp+20]
	;mov ecx,[ss:esp+24]
	int 36h
	pop	edx
	pop	ecx
	pop	ebx
	pop	eax
	ret
PhyMemSetByte:
	push	ds     ; 4
	push  ebx    ; 8
	mov   ax,1*8 ; 系统的数据段（可以访问到所有的内存）
	mov	  ds,ax
	mov   eax,[esp+4+8]
	mov   ebx,[esp+4+12]
	mov	  [eax],bl
	pop   ebx
	pop   ds
	ret
format:
	push ebx
	mov eax,0x25
	mov ebx,[esp+4+4]
	int 36h
	pop ebx
	ret

get_hour_hex:
	push ebx
	mov eax,0x26
	mov ebx,0x00
	int 36h
	pop ebx
	ret
get_min_hex:
	push ebx
	mov eax,0x26
	mov ebx,0x01
	int 36h
	pop ebx
	ret
get_sec_hex:
	push ebx
	mov eax,0x26
	mov ebx,0x02
	int 36h
	pop ebx
	ret
get_day_of_month:
	push ebx
	mov eax,0x26
	mov ebx,0x03
	int 36h
	pop ebx
	ret
get_day_of_week:
	push ebx
	mov eax,0x26
	mov ebx,0x04
	int 36h
	pop ebx
	ret
get_mon_hex:
	push ebx
	mov eax,0x26
	mov ebx,0x05
	int 36h
	pop ebx
	ret
get_year:
	push ebx
	mov eax,0x26
	mov ebx,0x06
	int 36h
	pop ebx
	ret
global setjmp
; int setjmp(jmp_buf env);
setjmp:
    mov ecx, [esp + 4]  ; ecx = env
    mov edx, [esp + 0]  ; edx = ret addr
    mov [ecx + 0], edx
    mov [ecx + 4], ebx
    mov [ecx + 8], esp
    mov [ecx + 12], ebp
    mov [ecx + 16], esi
    mov [ecx + 20], edi
    mov [ecx + 24], eax ; eax = trigblock()'s ret val

    xor eax, eax    ; setjmp ret val = 0
    ret

global longjmp
; void longjmp(jmp_buf env, int val)
longjmp:

    mov edx, [esp + 4]  ; edx = env
    mov eax, [esp + 8]  ; eax = val
    mov ecx, [edx + 0]  ; ecx = setjmp()'s ret val 
    mov ebx, [edx + 4]
    mov esp, [edx + 8]
    mov ebp, [edx + 12]
    mov esi, [edx + 16]
    mov edi, [edx + 20]
    
    ; make sure longjmp's ret val not 0
    test eax, eax   ; if eax == 0:
    jnz .1          ;   eax += 1
    inc eax         ; else: goto lable 1
.1: ; let longjmp's ret addr as setjmp's ret addr
    mov [esp + 0], ecx ; ret addr = ecx = setjmp's next code
    ret
global IsGuiMode
IsGuiMode:
	mov eax,0x07
	int 0x72
	ret
AddThread:
	push ebx ; 4
	push ecx ; 8
	push edx ; 12
	push esi ; 16
	mov eax,0x22
	mov ebx,0x0a
	mov ecx,[esp+4+16]
	mov edx,[esp+8+16]
	mov esi,[esp+12+16]
	int 36h
	pop esi
	pop edx
	pop ecx
	pop ebx
	ret
TaskLock:
	push eax
	push ebx
	mov eax,0x22
	mov ebx,0x0b
	int 36h
	pop ebx
	pop ebx
	ret
TaskUnlock:
	push eax
	push ebx
	mov eax,0x22
	mov ebx,0x0c
	int 36h
	pop ebx
	pop ebx
	ret
SubThread:
	push eax ; 4
	push ebx ; 8
	push ecx ; 12
	mov eax,0x22
	mov ebx,0x0d
	mov ecx,[esp+4+12]
	int 36h
	pop ecx
	pop ebx
	pop eax
	ret


set_mode:
	push ebx ; 4
	push ecx ; 8
	push edx ; 12
	mov eax,0x20
	mov ebx,0x05
	mov ecx,[esp+4 + 12]
	mov edx,[esp+8 + 12]
	int 36h
	pop edx
	pop ecx
	pop ebx
	ret
VBEDraw_Px:
	push eax ; 4
	push ebx ; 8
	push ecx ; 12
	push edx ; 16
	mov eax,0x27
	mov ebx,[esp + 4 + 16]
	mov ecx,[esp + 8 + 16]
	mov edx,[esp + 12 + 16]
	int 36h
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret
VBEGet_Px:
	push ebx
	push ecx
	mov eax,0x28
	mov ebx,[esp + 8 + 8] ; y
	mov ecx,[esp + 4 + 8] ; x
	int 36h
	pop ecx
	pop ebx
	ret
VBEGetBuffer:
	push eax
	push ebx
	mov eax,0x29
	mov ebx,[esp + 4 + 8]
	int 36h
	pop ebx
	pop eax
	ret
VBESetBuffer:
	push eax
	push ebx
	mov eax,0x2a
	mov ebx,[esp + 4 + 8]
	int 36h
	pop ebx
	pop eax
	ret
roll:
	push eax
	push ebx
	mov eax,0x2b
	mov ebx,[esp + 4 + 8]
	int 36h
	pop ebx
	pop eax
	ret
VBEDraw_Box:
push eax
push ebx
push ecx
push edx
push esi
push edi
mov	eax,0x2c
mov ebx,[esp+4+24]
mov ecx,[esp+8+24]
mov edx,[esp+12+24]
mov esi,[esp+16+24]
mov edi,[esp+20+24]
int	36h
pop edi
pop esi
pop edx
pop ecx
pop ebx
pop eax
ret

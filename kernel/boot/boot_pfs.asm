; boot for pfs file system
bootseg		equ		7c0h
dataseg		equ		800h
%define e_ident 0
%define e_type 16
%define e_machine 18
%define e_version 20
%define e_entry 24
%define e_phoff 28
%define e_shoff 32
%define e_flags 36
%define e_ehsize 40
%define e_phentsize 42
%define e_phnum 44
%define e_shentsize 46
%define e_shnum 48
%define e_shstrndx 50

%define p_type     0
%define p_offset   4
%define p_vaddr    8
%define p_paddr    12
%define p_filesz   16
%define p_memsz    20
%define p_flags    24
%define p_align    28
jmp start
sign: db "PFS",0xff
sec_bitmap_start: dd 0 ; 我不到阿
resd_sector_start: dd 1
resd_sector_end: dd 100 ; 实际上这里会由pdos那里操作
first_sector_of_bitmap: dd 0 ; 我也不到阿
root_dict_block: dd 0 ; 这是啥？？？？
volid: times 16 db 0
start:
  mov	ax,bootseg
	mov	ds,ax
	mov	ax,dataseg
	mov	es,ax
	mov [0],dl
  mov eax,[resd_sector_start]
  mov ebx,[resd_sector_end]
  sub ebx,eax
  mov  [read_size], word ebx
  mov eax,[resd_sector_start]
  mov [packet.lba],eax
  mov cx,0
.lba_read_loop:
	cmp cx,[read_size]
	jae .intoprotectmode
	call read1sector
	add word[packet.seg],94*512/16
	add dword[packet.lba],94
	add cx,94
jmp .lba_read_loop
.intoprotectmode:
	
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
	mov ah,42h
	mov dl,80h
	mov si,packet
	int 13h
	jc .error
	ret
.error:
	jmp $
[BITS 32]
inprotectmode:
	mov	ax,DataGDT
	mov	ds,ax
	mov	es,ax
	;mov	fs,ax
	;mov gs,ax
	mov	ss,ax


	; 接下来，我们要复制Program Header到正确的内存地址
	
	xor eax,eax
	mov	ax,dataseg	; 取出文件段地址
	shl	eax,4	; 段地址*16+偏移地址=物理地址
	; eax 此时是Loader的起始地址

	xor ecx,ecx
	mov word cx,[eax+e_phnum] ; 获取Program Header的数量
	xor edx,edx
	mov word dx,[eax+e_phentsize]   ; 获取一个Program header的大小
	; 接下来，将ebx设置为第一个程序头的起始地址
	mov ebx,[eax+e_phoff]
	; 因为只是偏移量， 所以要加上eax才能表示其在内存中的位置
	add ebx,eax
.loop:
	cmp dword [ebx+p_filesz],0 ;如果file_sz是0，那么就没必要复制
	je .skip1  ; 直接结束
	mov edi,[ebx+p_vaddr] ; edi是dest  vaddr是这个Program Header应被装载到的地址
	push eax
	add eax,[ebx+p_offset] ; *(ebx+p_offset) 是这个Program Header在这个程序头的偏移
						   ; eax则是文件在内存中的起始地址，所以此时eax是Program Header在内存中的位置
	mov esi,eax			   ; 下面memcpy的src
		push ecx
		mov ecx,[ebx+p_filesz] ; 要复制几个字节？[ebx+p_filesz]指的是这个Program Header的大小
			push ecx
			call memcpy ; 上面已经设置了src和dest，所以这里直接复制
			pop ecx
		mov esi,[ebx+p_memsz] ; 我们现在要处理bss段，于是先获取这个Program Header在内存中的大小
		cmp ecx,esi   ; 如果这个Program Header的大小比它在内存中的大小小，说明是bss段，要处理
		jnb .1        ; 这个Program Header的大小比它在内存中的大小大，不处理
		sub esi,ecx   ; 获取剩下没复制的大小
		mov ecx,esi
		call memset   ; memcpy已经将edi赋值到了p_vaddr+p_filesz的位置，直接调用
.1		pop ecx
	pop eax
.skip1:
	add ebx,edx
	loop .loop
.skip:
	; 4.跳转
	finit
	;cvttss2si eax, [esp+4] 
	mov eax,[eax+e_entry]
	;sub eax,0x100000
	mov esp,0x00ffffff
	push 4*8
	push eax
	jmp far [esp]

memcpy:
	mov	eax,[esi]
	mov	[edi],eax
	add	esi,4
	add	edi,4
	dec	ecx
	jnz	memcpy
	ret
memset:
	; edi: dest
	mov	byte [edi],0
	inc edi
	dec	ecx
	jnz	memset
	ret


packet:
	db	10h
	db	0
	dw	94
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
	db	0
	db	0x9a
	db	0xcf
	db	0
GDTR0:
	dw	GDTR0-GDT0	; 临时GDT的大小
	dd	GDT0	; 临时GDT的地址
read_size: dw 0
times	510-($-$$)	db	0
db	0x55,0xaa
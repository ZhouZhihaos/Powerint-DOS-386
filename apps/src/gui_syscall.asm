[BITS 32]
GLOBAL create_window,close_window,create_button,delete_button
GLOBAL SDraw_Char,Sputs,SDraw_Px,SDraw_Box,PrintChineseChar0,PrintChineseStr0
GLOBAL MsgBox,IsGuiMode,MakeTextBox,DeleteTextBox,GetTextBoxText,MakeListBox,AddItem,RemoveItem,DeleteListBox,GSetBuffer
[SECTION .text]
create_window:
  push ebx ; 4
  push ecx ; 8
  push edx ; 12
  push esi ; 16
  push edi ; 20
  mov eax,0x01
  mov ebx,[ss:esp+4 + 20] ; x
  mov ecx,[ss:esp+8 + 20] ; y
  mov edx,[ss:esp+12 + 20] ; w
  mov esi,[ss:esp+16 + 20] ; h
  mov edi,[ss:esp+20 + 20] ; title
  int 0x72
  pop edi
  pop esi
  pop edx
  pop ecx
  pop ebx
  ret
close_window:
  push ebx
  push edx
  mov eax,0x02
  mov ebx,esp
  int 0x72
  mov eax,edx
  pop edx
  pop ebx
  ret
create_button:
  push ebx
  push edx
  mov eax,0x03
  mov ebx,esp
  int 0x72
  mov eax,edx
  pop edx
  pop ebx
  ret
delete_button:
  push ebx
  push edx
  mov eax,0x04
  mov ebx,esp
  int 0x72
  mov eax,edx
  pop edx
  pop ebx
  ret
 
 SDraw_Char:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x05
	mov	ebx,0x09
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
Sputs:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x05
	mov	ebx,0x0a
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
SDraw_Px:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x05
	mov	ebx,0x0b
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
SDraw_Box:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x05
	mov	ebx,0x0c
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
PrintChineseChar0:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x05
	mov	ebx,0x0d
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
PrintChineseStr0:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x05
	mov	ebx,0x0e
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret

MsgBox:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x06
	mov	ebx,[esp+16]
	mov	edx,[esp+20]
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret

MakeTextBox:
	;push	eax
	push	ebx
	push	edx
	mov	eax,0x08
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
;	pop	eax
	ret
DeleteTextBox:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x09
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
GetTextBoxText:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x0a
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
MakeListBox:
	;push	eax
	push	ebx
	push	edx
	mov	eax,0x0b
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
;	pop	eax
	ret
AddItem:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x0c
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
RemoveItem:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x0d
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
DeleteListBox:
	push	eax
	push	ebx
	push	edx
	mov	eax,0x0e
	mov	edx,esp
	int	0x72
	pop	edx
	pop	ebx
	pop	eax
	ret
GSetBuffer:
	push eax
	push ebx
	push ecx
	push edx
	push esi
	push edi
	push ebp
	mov eax,0x0f
	mov ebx,[esp + 4 + 24+ 4]
	mov ecx,[esp + 8 + 24+ 4]
	mov edx,[esp + 12 + 24+ 4]
	mov esi,[esp + 16 + 24+ 4]
	mov edi,[esp + 20 + 24+ 4]
	mov ebp,[esp + 24 + 24 + 4]
	int 72h
	pop ebp
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret
[BITS 32]
GLOBAL Socket_Alloc,Socket_Init,Socket_Free,Socket_Send,Socket_Recv,GetIP,ping
[SECTION .text]
Socket_Alloc:
    push    ebx
    mov eax,0x01
    mov ebx,[esp+8]
    int 30h
    pop ebx
    ret
Socket_Init:
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    mov eax,0x05
    mov ebx,[esp+28]
    mov ecx,[esp+32]
    mov edx,[esp+36]
    mov esi,[esp+40]
    mov edi,[esp+44]
    int 30h
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret
Socket_Free:
    push    eax
    push    ebx
    mov eax,0x02
    mov ebx,[esp+12]
    int 30h
    pop ebx
    pop eax
    ret
Socket_Send:
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov eax,0x03
    mov ebx,[esp+20]
    mov ecx,[esp+24]
    mov edx,[esp+28]
    int 30h
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret
Socket_Recv:
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov eax,0x04
    mov ebx,[esp+20]
    mov ecx,[esp+24]
    mov edx,[esp+28]
    int 30h
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret
GetIP:
    mov eax,0x06
    int 30h
    ret
ping:
    push    ebx
    mov eax,0x07
    mov ebx,[esp+8]
    int 30h
    pop ebx
    ret

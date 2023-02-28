db 'A'
jmp near @(ENTRY)
[__STR]
db 'HELLO WORLD\n\0'
[ENTRY]
mov $5,eax
mov @(__STR),edx
int 36
mov $1e,eax
int 36
[HLT]
jmp near @(HLT)
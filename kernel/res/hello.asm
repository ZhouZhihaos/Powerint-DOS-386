db 'A'
jmp short @(start)
[str]
  db 'Hello World\n\0'
[start]
  mov $5,eax
  mov @(str),edx
  int 36
  mov $1e,eax
  int 36
[hlt]
  jmp short @(hlt)
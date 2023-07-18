[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[FILE "hello.nas"]
[BITS 32]
GLOBAL _Main
[SECTION .text]
_Main:
  mov eax,0x5
  mov edx,msg
  int 0x36
  mov eax,0x1e
  int 0x36
  jmp $
[SECTION .data]
msg:
  db 'hello, world',0

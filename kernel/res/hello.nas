[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[FILE "hello.nas"]
[BITS 32]
GLOBAL _main
EXTERN _puts
[SECTION .text]
_main:
  push msg
  call _puts
  add esp,4
  xor eax,eax
  ret
[SECTION .data]
msg:
  db 'hello, world',0

db 'A' ; Ҫʹ��Powerint DOS�ܹ�ʶ���ͷ
jmp near @(Entry) ; ��ת��Entry��
[string]
  db 'Hello Welcome To This App! Now You Can type any key!\n\0' ; ע��Ҫ�����ַ���������
[string2]
  db 'Do you want to continue?(y/n)\n\0' ; ע��Ҫ�����ַ���������
[string3]
  db 'Bye Bye!\n\0' ; ע��Ҫ�����ַ���������
[string4]
  db 'OK, Now You are in the APP,You can type any key!\n\0' ; ע��Ҫ�����ַ���������
[Entry]
  mov $5,eax ; ���ô�ӡ�ַ�����ϵͳ����
  mov @(string),edx ; ���ַ���ָ�봫�ݸ�edx
  int 36 ; ϵͳ����
  mov $5,eax
  mov @(string2),edx
  int 36
  call near @(INP)
  cmp $00,eax
  je @(NO)
  jmp short @(YES)
[NO]
  mov $5,eax
  mov @(string3),edx
  int 36
[P_END]
  call near @(Exit_To_DOS)
[HLT]
  jmp short @(HLT)
[Exit_To_DOS]
  mov $1e,eax
  int 36
  ret
[INP]
  mov $16,eax
  mov $01,ebx
  int 36
  cmp $79,edx ; y
  je @(END_Y)
  cmp $6e,edx ; n
  je @(END_N)
  jmp short @(INP)
[END_Y]
  mov $2,eax
  mov $79,edx
  int 36
  mov $2,eax
  mov $0a,edx
  int 36
  mov $1,eax
  ret
[END_N]
  mov $2,eax
  mov $6e,edx
  int 36
  mov $2,eax
  mov $0a,edx
  int 36
  mov $0,eax
  ret
[YES]
  mov $5,eax
  mov @(string4),edx
  int 36
[YES_START]
  mov $16,eax
  mov $01,ebx
  int 36
  cmp $0a,edx
  je @(YES_END)
  mov $2,eax
  int 36
  jmp short @(YES_START)
[YES_END]
  mov $2,eax
  mov $0a,edx
  int 36
  jmp near @(P_END)

#include <arg.h>
#include <stdio.h>
#include <syscall.h>
int main(int argc, char** argv);
void fpu_load_control_word(const uint16_t control)
{
    asm volatile("fldcw %0;"::"m"(control)); 
}
void Main() {
  init_float();
  
  // 初始化stdio stderr
  stdout = malloc(sizeof(FILE));
  stdin = malloc(sizeof(FILE));
  stderr = malloc(sizeof(FILE));
  stdout->buffer = NULL;
  stdout->mode = WRITE;
  stderr->buffer = NULL;
  stderr->mode = WRITE;
  stdin->buffer = NULL;
  stdin->mode = READ;
  char* buf = (char*)api_malloc(1024);
  char** argv;
  GetCmdline(buf);
  argv = api_malloc(sizeof(char*) * (Get_Argc(buf) + 1));
  for (int i = 0; i <= Get_Argc(buf); i++) {
    argv[i] = api_malloc(128);
  }
  for (int i = 0; i <= Get_Argc(buf); i++) {
    Get_Arg(argv[i], buf, i);
  }
  main(Get_Argc(buf) + 1, argv);
  exit();
}
void __main() {
  //莫名其妙的错误
}
void __chkstk_ms() {
  //莫名其妙的错误
}

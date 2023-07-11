#include <arg.h>
#include <stdio.h>
#include <syscall.h>
int main(int argc, char** argv);
void init_mem();
void Main() {
  // 初始化stdio stderr
  init_mem();
  stdout = malloc(sizeof(FILE));
  stdin = malloc(sizeof(FILE));
  stderr = malloc(sizeof(FILE));
  stdout->buffer = NULL;
  stdout->mode = WRITE;
  stderr->buffer = NULL;
  stderr->mode = WRITE;
  stdin->buffer = NULL;
  stdin->mode = READ;
  char* buf = (char*)malloc(1024);
  char** argv;
  GetCmdline(buf);
  // printf("buf = %s\n", buf);
  argv = malloc(sizeof(char*) * (Get_Argc(buf) + 1));
  for (int i = 0; i <= Get_Argc(buf); i++) {
    argv[i] = malloc(128);
  }
  //  printf("buf = %s\n", buf);
  for (int i = 0; i <= Get_Argc(buf); i++) {
    Get_Arg(argv[i], buf, i);
    // printf("#%d %s\n",i,argv[i]);
  }
  init_float();
  //   printf("Get_Argc = %d\n",Get_Argc(buf));
  //   printf("buf = %s\n", buf);
  main(Get_Argc(buf) + 1, argv);
  exit();
}
void __main() {
  //莫名其妙的错误
}
void __chkstk_ms() {
  //莫名其妙的错误
}

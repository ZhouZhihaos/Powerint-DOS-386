// 桌面应用程序入口
#include <arg.h>
#include <stdio.h>
#include <syscall.h>
int PowerMain(int argc, char** argv);
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
  TaskForever();
  PowerMain(Get_Argc(buf) + 1, argv);
  exit();
}
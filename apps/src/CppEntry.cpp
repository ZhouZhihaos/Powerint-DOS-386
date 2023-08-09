#include <syscall.h>
#include <arg.h>
#include <stdio.h>
#undef bool
#undef true
#undef false
int main(int argc,char **argv);
extern "C" void init_env();
extern "C" void init_mem();
extern "C" void init_float();
extern "C" void Main()
{
  // 初始化stdio stderr
  init_mem();
  stdout = (FILE *)malloc(sizeof(FILE));
  stdin = (FILE *)malloc(sizeof(FILE));
  stderr = (FILE *)malloc(sizeof(FILE));
  stdout->buffer = NULL;
  stdout->mode = WRITE;
  stderr->buffer = NULL;
  stderr->mode = WRITE;
  stdin->buffer = (unsigned char *)malloc(1024);
  stdin->fileSize = -1;
  stdin->bufferSize = 1024;
  stdin->p = 0;
  stdin->mode = READ;
  char *buf = (char *)malloc(1024);
  char **argv;
  GetCmdline(buf);
  init_env();
  // printf("buf = %s\n", buf);
  argv = (char **)malloc(sizeof(char *) * (Get_Argc(buf) + 1));
  for (int i = 0; i <= Get_Argc(buf); i++) {
    argv[i] = (char *)malloc(128);
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
extern "C" void __main()
{
    //莫名其妙的错误
}
void __chkstk_ms()
{
    //莫名其妙的错误
}

extern "C" void __cxa_pure_virtual()
{
    // Do nothing or print an error message.
}


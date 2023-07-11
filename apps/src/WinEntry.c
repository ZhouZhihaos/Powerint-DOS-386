// 桌面应用程序入口
#include <syscall.h>
#include <arg.h>
#include <stdio.h>
int PowerMain(int argc,char **argv);
void Main()
{
    if(!IsGuiMode()) {
        printf("This program can not run in DOS mode.\n");
        exit();
    }
    char *buf = malloc(1024);
    char **argv;
    GetCmdline(buf);
    argv = malloc(sizeof(char*)*(Get_Argc(buf)+1));
    for(int i = 0;i<=Get_Argc(buf);i++)
    {
        argv[i] = malloc(128);
    }
    for(int i=0;i<=Get_Argc(buf);i++)
    {
        Get_Arg(argv[i],buf,i);
    }
    TaskForever();
    PowerMain(Get_Argc(buf)+1,argv);
	exit();
}
#include <syscall.h>
#include <Arg.h>
int main(int argc,char **argv);
void Main()
{
    char *buf = malloc(1024);
    char **argv;
    GetCmdline(buf);
    argv = malloc(sizeof(char*)*1024);
    for(int i = 0;i<=Get_Argc(buf);i++)
    {
        argv[i] = malloc(1024);
    }
    for(int i=0;i<=Get_Argc(buf);i++)
    {
        Get_Arg(argv[i],buf,i);
    }
    main(Get_Argc(buf)+1,argv);
}
void __main()
{
    //莫名其妙的错误
}
void __chkstk_ms()
{
    //莫名其妙的错误
}
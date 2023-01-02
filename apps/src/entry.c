#include <syscall.h>
#include <arg.h>
int main(int argc,char **argv);
void Main()
{

    char *buf = api_malloc(1024);
    char **argv;
    GetCmdline(buf);
    argv = api_malloc(sizeof(char*)*(Get_Argc(buf)+1));
    for(int i = 0;i<=Get_Argc(buf);i++)
    {
        argv[i] = api_malloc(128);
    }
    for(int i=0;i<=Get_Argc(buf);i++)
    {
        Get_Arg(argv[i],buf,i);
    }
    main(Get_Argc(buf)+1,argv);
	exit();
}
void __main()
{
    //莫名其妙的错误
}
void __chkstk_ms()
{
    //莫名其妙的错误
}

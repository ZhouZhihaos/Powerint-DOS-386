#include <syscall.h>
#include <arg.h>
#undef bool
#undef true
#undef false
int main(int argc,char **argv);
extern "C" void init_mem();
extern "C" void Main()
{
    init_mem();
    char *buf = (char *)malloc(1024);
    char **argv;
    GetCmdline(buf);
    argv = (char **)malloc(sizeof(char*)*(Get_Argc(buf)+1));
    for(int i = 0;i<=Get_Argc(buf);i++)
    {
        argv[i] = (char *)malloc(128);
    }
    for(int i=0;i<=Get_Argc(buf);i++)
    {
        Get_Arg(argv[i],buf,i);
    }
    main(Get_Argc(buf)+1,argv);
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


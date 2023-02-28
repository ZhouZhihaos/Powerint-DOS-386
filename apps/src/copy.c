#include <syscall.h>
#include <ctypes.h>
#include <arg.h>
#include <string.h>
int main(int argc,char **argv)
{
    char *buf = api_malloc(1000);
    GetCmdline(buf);
    char *arg = api_malloc(1000);
    char *arg2 = api_malloc(1000);
    if(Get_Argc(buf) < 2)
    {
        print("Usage: copy <source> <destination>\n");
        return 0;
    }
    Get_Arg(arg,buf,1);
    Get_Arg(arg2,buf,2);
    print("Copy: ");
    print(arg);
    print(" -> ");
    print(arg2);
    print("\n");
    Copy(arg,arg2);
    api_free(buf,1000);
    api_free(arg,1000);
    api_free(arg2,1000);
	return 0;
}
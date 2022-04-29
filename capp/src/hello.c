//hello.c
#include <syscall.h>
#include <string.h>
#include <arg.h>
int main(int argc,char **argv)
{
    if(argc==0)
    {
        print("Hello World\n");
    }
    else
    {
        if(strcmp(argv[1],"-H")==0)
        {
            print("Help:\n");
            print("    -v ------ View Version\n");
            print("    -h ------ Help\n");
        }
        else if(strcmp(argv[1],"-V")==0)
        {
            print("1.0.0\n");
        }
        else
        {
            print("Unknown Arg\n");
        }
    }
	return 0;
}
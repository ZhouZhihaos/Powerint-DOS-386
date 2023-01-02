#include <syscall.h>
#include <rand.h>
#include <string.h>
int atoi(char *str)
{
    return strtol(str, NULL, 10);
}
int main(int argc,char **argv)
{
    print("Rand Num Game V1.0\n");
    print("By min0911_\n");
    print("\n");
    mysrand(RAND());
    int num = myrand() % 100;
    while (1)
    {
        char *buf = api_malloc(128);
        print("Input:");
        scan(buf,128);
        int input = atoi(buf);
        if (input == num)
        {
            print("You Win!\n");
            break;
        }
        else if (input > num)
        {
            print("Too Big!\n");
        }
        else
        {
            print("Too Small!\n");
        }
        for(int i = 0; i < 128; i++)
        {
            buf[i] = 0;
        }
        api_free(buf, 128);
    }
    return 0;
}
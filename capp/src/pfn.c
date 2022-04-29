#include <syscall.h>
#include <string.h>
#include <math.h>
bool is_pfn(int num)
{
    //判断是否是完美数
    int sum = 0;
    for(int i = 1;i<=num;i++)
    {
        if(num%i==0)
        {
            sum+=i;
        }
    }
    if(sum-num==num)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void clean(char *s, int l)
{
    for (int i = 0; i < l; i++)
    {
        s[i] = 0;
    }
}
void Print_Num(int num)
{
    //打印数字
    char *buf = malloc(128);
    my_itoa(num, buf);
    print(buf);
    clean(buf, 128);
    free(buf, 128);
}
int main(int argc,char **argv)
{
    for(int i = 1; ; i++)
    {
        if(is_pfn(i))
        {
            Print_Num(i);
            print("\n");
        }
    }
	return 0;
}

#include <string.h>
#include <syscall.h>
void F2S(double d, char *str,int l) {
    int n = (int)d; //去掉小数点
    int b = 0;
    int i,j;
    double m = d;//
    char *buf;
    buf = malloc(128);
    my_itoa(n,str);
    i=strlen(str);
    str[i] = '.'; //小数点
    str[i+1] = 0;
    while(d>1.0)
    {
        
        d /= 10.0;
        b++;
    }
    for(i=0;i<l;i++)
    {
        m *= 10;//扩大
    }
    n = (int)m; //放弃其他小数点
    my_itoa(n,buf);//转换成字符串，更好操作
    for(i=b,j=strlen(str);i<strlen(buf);i++,j++)
    {
        str[j] = buf[i];
    }
    str[j] = 0;

    free(buf,128);
    //5.55 => 0.555 => <1 = true and b = 1
    // 1.56 0.156,15.6,156
}
void reverse(char *s)
{
    char temp;
    char *p = s;    //p指向s的头部
    char *q = s;    //q指向s的尾部
    while(*q)
        ++q;
    q--;

    //交换移动指针，直到p和q交叉
    while(q > p)
    {
        temp = *p;
        *p++ = *q;
        *q-- = temp;
    }
}
/*
 * 功能：整数转换为字符串
 * char s[] 的作用是存储整数的每一位
 */
void my_itoa(int a,char *str)
{
    int n = a;
    if (a < 0)
    {
        a = -a;//如果是负数先转化为正数
    }
    //static char str[100];//这里的str必须是静态变量或全局变量
    int i = 0;
    while (a>0)//从低位开始变为字符，最后进行翻转
    { 
        str[i++] = a % 10 + '0';//将数字转为字符串
        a = a / 10;
    }
    if (n < 0)//如果是负数加上‘-’号
    {
        str[i++] = '-';
    }
    str[i] = '\0';

    reverse(str);
}

int Atoi(char *Str)
{
    int i = 0,n = 0;
    int isNegative = 0;
    if(Str[0] == '-')
    {
        isNegative = 1;
        i++;
    }
    while(Str[i] != '\0')
    {
        n = n*10 + (Str[i] - '0');
        i++;
    }
    if(isNegative)
    {
        n = -n;
    }
    return n;
}
#include <string.h>
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
void my_itoa(int n,char *s)
{
    int i = 0,isNegative = 0;
    if((isNegative = n) < 0) //如果是负数，先转为正数
    {
        n = -n;
    }
    do        //从各位开始变为字符，直到最高位，最后应该反转
    {
        s[i++] = n%10 + '0';
        n = n/10;
    }while(n > 0);

    if(isNegative < 0)    //如果是负数，补上负号
    {
        s[i++] = '-';
    }
    s[i] = '\0';    //最后加上字符串结束符   
    reverse(s);
}
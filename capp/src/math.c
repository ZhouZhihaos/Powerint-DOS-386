#include <math.h>
float intpower(float a, int n)
{ //非负整数次方
    if (n == 0)
    {
        return 1;
    }
    else
    {
        int i;
        float s = 1;
        for (i = 0; i < n; i++)
        {
            s = s * a;
        }
        return s;
    }
}
float ln(float x)
{ //自然对数
    float s = 0;
    int E = 50; //精度
    if (x < 1)
    {
        int i, sygn = 1;
        for (i = 1; i < E + 1; i++)
        {

            s = s + sygn * intpower(x - 1, i) / i;
            sygn = -sygn;
        }
    }
    else
    {
        x = 1 / x;
        int i, sygn = 1;
        for (i = 1; i < E + 1; i++)
        {

            s = s + sygn * intpower(x - 1, i) / i;
            sygn = -sygn;
        }
        s = -s;
    }

    return s;
}
int fact(int n) //阶乘
{
    int s = 1;
    if (n == 0)
    {
        return 1;
    }
    else
    {
        int i;
        for (i = 1; i <= n; i++)
        {
            s = s * i;
        }
        return s;
    }
}
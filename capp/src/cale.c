#include <string.h>
#include <syscall.h>
void clean(char *s, int l)
{
    for (int i = 0; i < l; i++)
    {
        s[i] = 0;
    }
}
int Week(int yy, int mm, int dd)
{
    int year = yy;  //因为定义的是全局变量 所以就需要定义新变量来传递值
    int month = mm; //当然 你也可以定义在main函数内然后用&引用传递
    int day = dd;
    if (mm < 3)
    {
        year -= 1;
        month += 12;
    }
    int y = year % 100;
    int c = year / 100;
    int d = day;
    int m = month;
    int w = (y + y / 4 + c / 4 - 2 * c + 13 * (m + 1) / 5 + d - 1) % 7;
    return w;
}
void cal(int year, int mouth)
{
    int day = 0;
    if (mouth == 1 || mouth == 3 || mouth == 5 || mouth == 7 || mouth == 8 || mouth == 10 || mouth == 12)
    {
        day = 31;
    }
    else if (mouth == 4 || mouth == 6 || mouth == 9 || mouth == 11)
    {
        day = 30;
    }
    else if (mouth == 2)
    {
        if (year % 4 == 0)
        {
            day = 29;
        }
        else
        {
            day = 28;
        }
    }
    print("      ");
    char *buf = malloc(128);
    my_itoa(year, buf);
    print(buf);
    print("year,");
    clean(buf, 128);
    my_itoa(mouth, buf);
    print(buf);
    print("mouth");
    clean(buf, 128);
    free(buf, 128);
    print("\n");
    print("Sun Mon Tue Wed Thu Fri Sat\n");
    buf = malloc(128);
    int week = 0;

    for (int i = 0; i < Week(year, mouth, 1); i++)
    {
        print("    ");
        week++;
    }
    week++;
    print("1   ");
    if (week % 7 == 0)
    {
        print("\n");
        week = 1;
    }else{
        week++;
    }
    for (int i = 2,k=week; i <= day; i++, k++)
    {
        my_itoa(i, buf);
        print(buf);
        if (k % 7 == 0)
        {
            print("\n");
        }
        else
        {
            for (int j = 0; j < 3 - strlen(buf) + 1; j++)
            {
                print(" ");
            }
        }
        clean(buf, 128);
    }
    clean(buf, 128);
    free(buf, 128);
}
int main(int argc,char **argv)
{
    char *buf = malloc(128);
    print("Year:");
    scan(buf, 128);
    int year = Atoi(buf);
    clean(buf, 128);
    print("Month:");
    scan(buf, 128);
    int month = Atoi(buf);
    clean(buf, 128);
    free(buf, 128); //释放内存
    cal(year, month);
	return 0;
}
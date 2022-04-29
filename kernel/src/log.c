#include "system.h"
//系统日志打印
void kprint(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        write_serial(str[i]);
    }
}
void printk(char *str, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, str);
    vsprintf(buf, str, ap);
    va_end(ap);
    kprint(buf);
}

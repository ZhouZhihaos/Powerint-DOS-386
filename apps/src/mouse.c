#include <mouse.h>
int GetMouse_x(int mouse)
{
    unsigned short high = ((short *)(&mouse))[1];
    //获取mouse的高十六位
    high = mouse >> 16;
    //获取mouse的低十六位
    int low = mouse & 0xFFFF;

    char x = (char)((high & 0xFF));
    char y = ((char *)(&high))[1];
    char btn = low & 0xf;
    return x;
}
int GetMouse_y(int mouse)
{
    unsigned short high = ((short *)(&mouse))[1];
    //获取mouse的高十六位
    high = mouse >> 16;
    //获取mouse的低十六位
    int low = mouse & 0xFFFF;

    char x = (char)((high & 0xFF));
    char y = ((char *)(&high))[1];
    char btn = low & 0xf;
    return y;
}
int GetMouse_btn(int mouse)
{
    unsigned short high = ((short *)(&mouse))[1];
    //获取mouse的高十六位
    high = mouse >> 16;
    //获取mouse的低十六位
    int low = mouse & 0xFFFF;

    char x = (char)((high & 0xFF));
    char y = ((char *)(&high))[1];
    char btn = low & 0xf;
    return btn;
}
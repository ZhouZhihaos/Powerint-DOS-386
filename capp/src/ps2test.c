#include <syscall.h>
#include <string.h>
#include <mouse.h>
void PrintNum(int num)
{
    char *BUF = malloc(128);
    my_itoa(num, BUF);
    print(BUF);
    free(BUF, 128);
}
int main(int argc,char **argv)
{
    while (1)
    {
        int mouse = Text_get_mouse();
        //mouse:
        //高十六位中的高八位存放y
        //低八位存放x
        //低十六位存放btn
        int x = GetMouse_x(mouse);
        int y = GetMouse_y(mouse);
        int btn = GetMouse_btn(mouse);

        print("x:");
        PrintNum(x);
        print(" y:");
        PrintNum(y);
        print(" btn:");
        PrintNum(btn);
        print(" ");
        if(btn == CLICK_LEFT)
        {
            print("CLICK: LEFT\n");
        }
        else if(btn == CLICK_RIGHT)
        {
            print("CLICK: RIGHT\n");
        }
        else if(btn == CLICK_MIDDLE)
        {
            print("CLICK: MIDDLE\n");
        }
        else
        {
            print("CLICK: NONE\n");
        }
    }
	return 0;
}
#include <syscall.h>
#include <string.h>
#include <mouse.h>
int main(int argc,char **argv)
{
	char *buf = api_malloc(100);
    while (1)
    {
        int mouse = get_mouse();
        //mouse:
        //高十六位中的高八位存放y
        //低八位存放x
        //低十六位存放btn
        int x = GetMouse_x(mouse);
        int y = GetMouse_y(mouse);
        int btn = GetMouse_btn(mouse);

        sprintf(buf,"X:%d Y:%d ",x,y);
		print(buf);
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
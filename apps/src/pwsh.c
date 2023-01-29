#include <syscall.h>
#include <mouse.h>
#include <string.h>
void Create_Button(int x, int y, int w, int h, char *text)
{
    Text_Draw_Box(x, y, x + w, y + h, 0xf0);
    goto_xy(y, x + w / 2);
    print(text);
}
void New_CreateButton(int x, int y, char *text)
{
    Create_Button(y, x, 3, 10, text);
}
bool Button_Click_Left(int x, int y, int w, int h)
{
    int mouse = get_mouse();
    int x1 = GetMouse_x(mouse);
    int y1 = GetMouse_y(mouse);
    int btn = GetMouse_btn(mouse);
    if (x1 >= x && x1 <= x + w && y1 >= y && y1 <= y + h && btn == CLICK_LEFT)
    {
        return true;
    }
    return false;
}
bool Button_Click_Right(int x, int y, int w, int h)
{
    int mouse = get_mouse();
    int x1 = GetMouse_x(mouse);
    int y1 = GetMouse_y(mouse);
    int btn = GetMouse_btn(mouse);
    if (x1 >= x && x1 <= x + w && y1 >= y && y1 <= y + h && btn == CLICK_RIGHT)
    {
        return true;
    }
    return false;
}
bool Button_Click_Middle(int x, int y, int w, int h)
{
    int mouse = get_mouse();
    int x1 = GetMouse_x(mouse);
    int y1 = GetMouse_y(mouse);
    int btn = GetMouse_btn(mouse);
    if (x1 >= x && x1 <= x + w && y1 >= y && y1 <= y + h && btn == CLICK_MIDDLE)
    {
        return true;
    }
    return false;
}
void MessageBox(int x, int y, char *text)
{
    system("cls");
    Text_Draw_Box(0, 0, 25, 80, 0xa0);
    Text_Draw_Box(y, x, x + 10, y + 20, 0xb0);
    goto_xy(y, x + 10 / 2);
    print(text);
    Create_Button(y + 8, x + 14, 2, 5, "OK");
    while (1)
    {
        int mouse = get_mouse();
        int x1 = GetMouse_x(mouse);
        int y1 = GetMouse_y(mouse);
        int btn = GetMouse_btn(mouse);
        if (Button_Click_Left(x + 14, y + 8, 5, 2))
        {
            break;
        }
    }
    system("cls");
    Draw_UI();
}
void Draw_UI()
{
    Text_Draw_Box(0, 0, 25, 80, 0xa0);
    New_CreateButton(5, 5, "Exit");
    New_CreateButton(5, 9, "About");
    New_CreateButton(5, 13, "System");
    New_CreateButton(5, 17, "CLICK");
}
void Click(int x,int y)
{
    system("cls");
    Text_Draw_Box(0, 0, 25, 80, 0xa0);
    Text_Draw_Box(y, x, x + 10, y + 50, 0xb0);
    Text_Draw_Box(y + 1, x + 10 / 2, x + 1, y + 48, 0xf0);
    int n = 0;
    Create_Button(y + 7, x + 25, 3, 5, "CLICK");
    Create_Button(y + 1, x + 43, 2, 4, " X");
    while (1)
    {
        int mouse = get_mouse();
        int x1 = GetMouse_x(mouse);
        int y1 = GetMouse_y(mouse);
        int btn = GetMouse_btn(mouse);
        if (Button_Click_Left(x + 25, y + 7, 5, 3))
        {
            n++;
            goto_xy(y + 1, x + 10 / 2);
			char *s = api_malloc(100);
            sprintf(s,"TIMES:%d",n);
			print(s);
        }
        if (Button_Click_Left(x + 43, y + 1, 4, 2))
        {
            break;
        }
    }
    system("cls");
    Draw_UI();
}
void System_Box(int x, int y)
{
    system("cls");
    Text_Draw_Box(0, 0, 25, 80, 0xa0);
    Text_Draw_Box(y, x, x + 10, y + 50, 0xb0);
    Text_Draw_Box(y + 1, x + 10 / 2, x + 1, y + 48, 0xf0);
    goto_xy(y + 1, x + 10 / 2);
    print("Command:");
    char *com = api_malloc(48);
    scan(com, 48);
    goto_xy(y + 1, x + 10 / 2 + 1);
    system(com);
    for (int i = 0; i < 48; i++)
    {
        com[i] = 0;
    }
    api_free(com, 48);
    Create_Button(y + 7, x + 25, 3, 5, "OK");
    while (1)
    {
        int mouse = get_mouse();
        int x1 = GetMouse_x(mouse);
        int y1 = GetMouse_y(mouse);
        int btn = GetMouse_btn(mouse);
        if (Button_Click_Left(x + 25, y + 7, 5, 3))
        {
            break;
        }
    }
    system("cls");
    Draw_UI();
}
int main(int argc,char **argv)
{
    system("cls");
    Draw_UI();
    while (1)
    {
        /* code */
        int mouse = get_mouse();
        int x = GetMouse_x(mouse);
        int y = GetMouse_y(mouse);
        int btn = GetMouse_btn(mouse);
        if (Button_Click_Left(5, 5, 10, 3))
        {
            break;
        }
        else if (Button_Click_Left(5, 9, 10, 3))
        {
            MessageBox(5, 5, "Power shell v1.0");
        }
        else if (Button_Click_Left(5, 13, 10, 3))
        {
            System_Box(5, 5);
        }
        else if (Button_Click_Left(5, 17, 10, 3))
        {
            Click(5,5);
        }
    }
    return 0;
}
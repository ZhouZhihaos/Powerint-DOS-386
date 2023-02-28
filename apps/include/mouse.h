#ifndef __MOUSE_H__
#define __MOUSE_H__
//鼠标状态
enum __MOUSE__STATUS
{
    CLICK_LEFT = 1,
    CLICK_RIGHT = 2,
    CLICK_MIDDLE = 3,
};
#ifdef __cplusplus
extern "C" {
#endif
int GetMouse_x(int mouse);
int GetMouse_y(int mouse);
int GetMouse_btn(int mouse);
#ifdef __cplusplus
}
#endif
#endif
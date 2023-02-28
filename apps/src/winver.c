#include <gui.h>
#include <stdio.h>
#include <syscall.h>

window_t mainWindow;
button_t OK;
int PowerMain() {
  mainWindow = create_window(50, 50, 400, 200, "winver");
  OK = create_button(200 - 5, 200 - 90, 35, 40, mainWindow, "OK");
  Sputs(mainWindow, "Powerint DOS 386 v0.6c", 5, 28, 0x0, 400);
  Sputs(mainWindow, "Copyright (C) 2022 zhouzhihao & min0911_", 5, 57, 0x0, 400);
  Sputs(mainWindow, "All rights reserved.", 5, 86, 0x0, 400);
  // PrintChineseStr0(mainWindow, "你好世界!", 0, 28+32, 0x0, 200);
  while (1) {
    int sz_win = MessageLength(WINDOW_CLOSE_MSG_ID);
    int btn_sz = MessageLength(BUTTON_MSG_ID);
    if (sz_win == 4) {
      window_t id;
      GetMessage(&id, WINDOW_CLOSE_MSG_ID);
      if (id == mainWindow) {
        delete_button(OK);
        close_window(mainWindow);
        return 0;
      }
    }
    if (btn_sz == 4) {
      button_t id;
      GetMessage(&id, BUTTON_MSG_ID);
      if (id == OK) {
        delete_button(OK);
        close_window(mainWindow);
        return 0;
      }
    }
  }
}
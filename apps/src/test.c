#include <gui.h>
#include <syscall.h>
#include <stdio.h>
int PowerMain(int argc, char** argv) {
  window_t id = create_window(0, 0, 150, 150, "test");
  button_t id2 = create_button(0, 0, 48, 32, id, "TEST");
  window_t id3 = -1;
  button_t id4 = -1;
  static unsigned int mes;
  // int i = create_window(50, 50, 150, 150, "MESSAGE");
  // create_button(0, 0, 48, 32, i, "TEST");
  static unsigned int mes2;
  while (1) {
    if (MessageLength(BUTTON_MSG_ID) != -1) {
      GetMessage(&mes, BUTTON_MSG_ID);
      if (mes == (unsigned int)id2) {
        if (id3 == -1) {
          id3 = create_window(50, 50, 150, 150, "MESSAGE");
          id4 = create_button(0, 0, 48, 32, id3, "TEST");
        }
        mes = 0;
      }
    }
    if (MessageLength(WINDOW_CLOSE_MSG_ID) != -1) {
      GetMessage(&mes2, WINDOW_CLOSE_MSG_ID);
      if (mes2 == (unsigned int)id) {
        // 记得关闭id3
        if(id3 != -1) {
          delete_button(id4);
          close_window(id3);
        }
        delete_button(id2);
        close_window(id);
        return 0;
      }
      if(id3 != -1 && mes2 == (unsigned int)id3) {
        delete_button(id4);
        close_window(id3);
        id3 = -1;
        id4 = -1;
      }
      mes2 = 0;
    }
  }
}
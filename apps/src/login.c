#include <gui.h>
#include <string.h>
#include <syscall.h>

window_t mainwindow;
int PowerMain() {
  mainwindow = create_window(50, 50, 400, 200, "login");
  Sputs(mainwindow, "ID:", 10, 40, 0x0, 400);
  HWND text_id =
      MakeTextBox(10 + 10 + strlen("Password:") * 8, 40, 200, 20, mainwindow);
  Sputs(mainwindow, "Password:", 10, 60, 0x0, 400);
  HWND text_pwd =
      MakeTextBox(10 + 10 + strlen("Password:") * 8, 60, 200, 20, mainwindow);

  button_t login = create_button(400 / 2 - 3, 100, strlen("login") * 8 + 10, 20,
                                 mainwindow, "login");
  while (1) {
    int sz_win = MessageLength(WINDOW_CLOSE_MSG_ID);
    int btn_sz = MessageLength(BUTTON_MSG_ID);
    if (sz_win == 4) {
      window_t id;
      GetMessage(&id, WINDOW_CLOSE_MSG_ID);
      if (id == mainwindow) {
        delete_button(login);
        DeleteTextBox(text_id);
        DeleteTextBox(text_pwd);
        close_window(mainwindow);
        return 0;
      }
    }
    if (btn_sz == 4) {
      button_t id;
      GetMessage(&id, BUTTON_MSG_ID);
      if (id == login) {
        char id_str[200/8];
        char pwd_str[200/8];
        GetTextBoxText(id_str, text_id);
        GetTextBoxText(pwd_str, text_pwd);
        if(strcmp(id_str, "admin") == 0 && strcmp(pwd_str, "admin") == 0) {
          MsgBox("login success", "login");
        } else {
          MsgBox("ID or Password is wrong!", "login");
        }
      }
    }
  }
}
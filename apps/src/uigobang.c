#include <gui.h>
#include <stdio.h>
#include <syscall.h>

button_t BTN[19][19];
window_t WND;
int x, y;
int f = 1;
char map[19][19] = {0};
void init() {
  WND = create_window(150, 150, 19 * 18 + 28, 19 * 18 + 28, "Gobang");
  for (int j = 0; j < 19; j++) {
    for (int i = 0; i < 19; i++) {
      BTN[i][j] = create_button(i * 18, j * 18, 16, 16, WND, "");
    }
  }
}
void get_where_click(unsigned int mes, int* btnx, int* btny) {
  for (int i = 0; i != 19; i++) {
    for (int j = 0; j != 19; j++) {
      if ((unsigned int)BTN[i][j] == mes) {
        *btnx = i;
        *btny = j;
        return;
      }
    }
  }
}
int Win() {
  int i, j;
  for (i = 0; i < 19; i++) {
    for (j = 0; j < 19; j++) {
      int w_count = 0, b_count = 0;
      int k, l;
      for (k = 0; k < 5; k++) {
        if (map[i][j] == 'O') {
          if (map[i + k][j] == 'O')
            b_count++;
          else
            break;
        } else if (map[i][j] == 'X') {
          if (map[i + k][j] == 'X')
            w_count++;
          else
            break;
        }
      }
      if (b_count == 5)
        return 1;
      else if (w_count == 5)
        return 2;
      else {
        b_count = 0;
        w_count = 0;
      }
      for (l = 0; l < 5; l++) {
        if (map[i][j] == 'O') {
          if (map[i][j + l] == 'O')
            b_count++;
          else
            break;
        } else if (map[i][j] == 'X') {
          if (map[i][j + l] == 'X')
            w_count++;
          else
            break;
        }
      }
      if (b_count == 5)
        return 1;
      else if (w_count == 5)
        return 2;
      else {
        b_count = 0;
        w_count = 0;
      }
      for (k = 0; k < 5; k++) {
        if (map[i][j] == 'O') {
          if (map[i + k][j + k] == 'O')
            b_count++;
          else
            break;
        } else if (map[i][j] == 'X') {
          if (map[i + k][j + k] == 'X')
            w_count++;
          else
            break;
        }
      }
      if (b_count == 5)
        return 1;
      else if (w_count == 5)
        return 2;
      else {
        b_count = 0;
        w_count = 0;
      }
      for (k = 0; k < 5; k++) {
        if (map[i][j] == 'O') {
          if (map[i - k][j + k] == 'O')
            b_count++;
          else
            break;
        } else if (map[i][j] == 'X') {
          if (map[i - k][j + k] == 'X')
            w_count++;
          else
            break;
        }
      }
      if (b_count == 5)
        return 1;
      else if (w_count == 5)
        return 2;
      else {
        b_count = 0;
        w_count = 0;
      }
    }
  }
}
void mainloop() {
  static unsigned int mes = 0;
  static unsigned int len_btn = -1, len_wnd = -1;
  while (1) {
    len_btn = MessageLength(BUTTON_MSG_ID);
    if (len_btn == 0x4) {
      GetMessage(&mes, BUTTON_MSG_ID);
      get_where_click(mes, &x, &y);
      mes = 0;
      delete_button(BTN[x][y]);
      if (f) {
        BTN[x][y] = create_button(x * 18, y * 18, 16, 16, WND, "O");
        map[x][y] = 'O';
        f = 0;
      } else {
        BTN[x][y] = create_button(x * 18, y * 18, 16, 16, WND, "X");
        map[x][y] = 'X';
        f = 1;
      }
      if (Win() == 1) {
        MsgBox("O's player is win!", "Gobang");
        break;
      } else if (Win() == 2) {
        MsgBox("X's player is win!", "Gobang");
        break;
      }
    }
    len_btn = -1;
  }
  while (1) {
    len_wnd = MessageLength(WINDOW_CLOSE_MSG_ID);
    if (len_wnd == 0x4) {
      GetMessage(&mes, WINDOW_CLOSE_MSG_ID);
      for (int j = 0; j < 19; j++) {
        for (int i = 0; i < 19; i++) {
          delete_button(BTN[i][j]);
        }
      }
      close_window(WND);
      return;
    }
    len_wnd = -1;
  }
}
int PowerMain() {
  init();
  mainloop();
}

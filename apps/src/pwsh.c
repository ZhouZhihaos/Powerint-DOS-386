#include <mouse.h>
#include <string.h>
#include <syscall.h>
#define max_file_list_num 17
static struct finfo_block *file_list;
static int roll = 0, choose = 0, file_list_num;
static char *path;
static int tid_main, tid_mouse;
void Create_Button(int x, int y, int w, int h, char *text, char color) {
  goto_xy(x + (w - strlen(text)) / 2, y);
  print(text);
  T_DrawBox(x, y, w, h, color);
}
void Draw_UI() {
  system("cls");
  goto_xy(32, 0);
  printf("Powerint Shell");
  goto_xy(2, 1);
  printf("Path: %s", path);
  goto_xy(2, 3);
  printf("NAME");
  goto_xy(22, 3);
  printf("SIZE");
  goto_xy(36, 3);
  printf("TYPE");
  goto_xy(52, 3);
  printf("DATE       TIME");
  for (int i = 0;
       file_list[roll + i].name[0] != 0 && roll + i <= max_file_list_num; i++) {
    goto_xy(2, 4 + i);
    printf("%s", file_list[roll + i].name);
    goto_xy(22, 4 + i);
    printf("%d", file_list[roll + i].size);
    goto_xy(36, 4 + i);
    if (file_list[roll + i].type == FLE) {
      printf("FILE");
    } else if (file_list[roll + i].type == DIR) {
      printf("DIR");
    } else if (file_list[roll + i].type == RDO) {
      printf("READ-ONLY");
    } else if (file_list[roll + i].type == SYS) {
      printf("SYSTEM-FILE");
    } else if (file_list[roll + i].type == HID) {
      printf("HIDE");
    }
    goto_xy(52, 4 + i);
    printf("%04d-%02d-%02d %02d:%02d", file_list[roll + i].year,
           file_list[roll + i].month, file_list[roll + i].day,
           file_list[roll + i].hour, file_list[roll + i].minute);
  }
  T_DrawBox(2, 4 + choose, 76, 1, 0xf0);
  Create_Button(2, 23, 13, 1, "CHANGE PATH", 0xf0);
  Create_Button(18, 23, 6, 1, "EXIT", 0xf0);
}
void mouse_thread() {
  tid_mouse = NowTaskID();
  while (1) {
    int mouse = get_mouse();
    int x = GetMouse_x(mouse);
    int y = GetMouse_y(mouse);
    int btn = GetMouse_btn(mouse);
    if (btn == 1) {
      if (x >= 2 && x <= 78 && y <= 4 + file_list_num - 1 &&
          y <= 4 + max_file_list_num && y >= 4) {
        choose = y - 4;
      } else if (x >= 2 && x <= 2 + 13 && y == 23) {
        TaskLock();
        goto_xy(8, 1);
        while (_kbhit())
          getch(); // 清空输入缓冲区
        scan(path, 512);
        free((void *)file_list);
        file_list = listfile(path);
        for (file_list_num = 0; file_list[file_list_num].name[0] != 0;
             file_list_num++)
          ;
        roll = 0;
        choose = 0;
        TaskUnlock();
      } else if (x >= 18 && x <= 18 + 6 && y == 23) {
        system("cls");
        SendMessage(tid_main, "\0", 1);
      }
      Draw_UI();
    } else if (btn == 4) {
      if (choose > 0) {
        choose--;
      } else if (roll > 0) {
        roll--;
      }
      Draw_UI();
    } else if (btn == 5) {
      if (choose >= file_list_num - 1 && file_list_num > max_file_list_num &&
          roll < file_list_num - max_file_list_num) {
        roll++;
      } else if (choose < file_list_num - 1) {
        choose++;
      }
      Draw_UI();
    }
  }
}
int main(int argc, char **argv) {
  tid_main = NowTaskID();
  path = (char *)malloc(512);
  strcpy(path, "/");
  file_list = listfile(path);
  for (file_list_num = 0; file_list[file_list_num].name[0] != 0;
       file_list_num++)
    ;
  Draw_UI();
  unsigned int stack = (unsigned int)malloc(16 * 1024);
  AddThread("mouse", &mouse_thread, stack + 16 * 1024);
  while (1) {
    if (_kbhit()) {
      int c = getch();
      if (c == -1) {
        if (choose > 0) {
          choose--;
        } else if (roll > 0) {
          roll--;
        }
      } else if (c == -2) {
        if (choose >= file_list_num - 1 && file_list_num > max_file_list_num &&
            roll < file_list_num - max_file_list_num) {
          roll++;
        } else if (choose < file_list_num - 1) {
          choose++;
        }
      } else if (c == '\n') {
        if (file_list[choose].type == FLE || file_list[choose].type == HID) {
          TaskLock();
          system("cls");
          T_DrawBox(15, 9, 50, 6, 0xf0);
          T_DrawBox(16, 10, 48, 4, 0x0f);
          goto_xy(21, 10);
          printf("What do you want to do with this file?");
          Create_Button(21, 12, 14, 1, "Open With...", 0xf0);
          Create_Button(38, 12, 8, 1, "Delete", 0x0f);
          Create_Button(49, 12, 8, 1, "Cancel", 0x0f);
          int move = 0;
          for (;;) {
            if (_kbhit()) {
              int c1 = getch();
              if (c1 == -3 && move > 0) {
                move--;
                if (move == 0) {
                  Create_Button(21, 12, 14, 1, "Open With...", 0xf0);
                  Create_Button(38, 12, 8, 1, "Delete", 0x0f);
                } else {
                  Create_Button(38, 12, 8, 1, "Delete", 0xf0);
                  Create_Button(49, 12, 8, 1, "Cancel", 0x0f);
                }
              } else if (c1 == -4 && move <= 2) {
                move++;
                if (move == 1) {
                  Create_Button(21, 12, 14, 1, "Open With...", 0x0f);
                  Create_Button(38, 12, 8, 1, "Delete", 0xf0);
                } else {
                  Create_Button(38, 12, 8, 1, "Delete", 0x0f);
                  Create_Button(49, 12, 8, 1, "Cancel", 0xf0);
                }
              } else if (c1 == '\n') {
                char *buffer = (char *)malloc(512);
                if (move == 0) {
                  goto_xy(2, 15);
                  printf("Program Path: ");
                  scan(buffer, 512);
                  sprintf(buffer, "%s %s%s", buffer, path,
                          file_list[choose].name);
                  system(buffer);
                } else if (move == 1) {
                  sprintf(buffer, "del %s%s", path, file_list[choose].name);
                  system(buffer);
                  choose--;
                }
                free((void *)file_list);
                file_list = listfile(path);
                for (file_list_num = 0; file_list[file_list_num].name[0] != 0;
                     file_list_num++)
                  ;
                free(buffer);
                TaskUnlock();
                break;
              }
            }
          }
        }
      } else if (c == 1) {
        TaskLock();
        goto_xy(8, 1);
        scan(path, 512);
        free((void *)file_list);
        file_list = listfile(path);
        for (file_list_num = 0; file_list[file_list_num].name[0] != 0;
             file_list_num++)
          ;
        roll = 0;
        choose = 0;
        TaskUnlock();
      }
      Draw_UI();
    }
    if (haveMsg()) {
      exit();
    }
  }
  return 0;
}
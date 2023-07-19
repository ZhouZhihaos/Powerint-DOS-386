#include <io.h>
extern char keytable[];
extern char keytable1[];
void input(char *ptr, int len) {
  /**
   * 别问我为什么这么写，我也不知道为什么
   * 将就用吧，能用就行
   */
  clean(ptr, len);

  int i;
  int BNOW = input_stack_get_now();
  int NOW = input_stack_get_now() - 1;
  // int rf = 0; //右方向键按下过吗？
  int Bmx, Bmy;
  Bmx = get_x();
  Bmy = get_y();
  for (i = 0; i != len; i++) {
    int in = getch();
    if (in == '\0') {
      i--;
      continue;
    }
    if (in == '\n') {
      ptr[strlen(ptr)] = 0;

      if (Bmy >= get_ysize() - 1) {
        int bx = get_x();
        int by = get_y();
        for (int j = 0; j < strlen(ptr) - i + 1; j++) {
          gotoxy(bx + j, by);
        }
        print("\n");
      } else {
        gotoxy(0, Bmy + (strlen(ptr) / get_xsize()) + 1);
      }
      input_stack_set_now(BNOW);
      if (i > 0) {
        input_stack_put(ptr);
        input_stack_set_now(BNOW - 1);
      }

      return;
    } else if (in == '\b') {
      if (i == 0) {
        i--;
        continue;
      }
      if (ptr[i] == 0) {

        i--;
        ptr[i] = 0;
        i--;
        printchar('\b');
      } else {
        int bx = get_x();
        int by = get_y();
        i--;
        delete_char(ptr, i);
        i--;
        for (int j = 0; j < strlen(ptr) - i - 1; j++) {
          printchar(' ');
        }
        for (int j = 0; j < strlen(ptr) + 1; j++) {
          printchar('\b');
        }
        print(ptr);
        gotoxy(bx - 1, by);
      }
      continue;
    } else if (in == '\t') {
      //找到空格的扫描码
      for (int k = 0; k < 0x54; ++k) {
        if (keytable1[k] == ' ') {
          // in = k;
          fifo8_put(task_get_key_fifo(current_task()), k);
          fifo8_put(task_get_key_fifo(current_task()), k);
          fifo8_put(task_get_key_fifo(current_task()), k);
          fifo8_put(task_get_key_fifo(current_task()), k);
          i--;
          break;
        }
      }
      continue;
    } else if (in == -1) {
      int bx = get_x();
      int by = get_y();
      if (get_free() != 1023 && NOW != 1023) //没有输入
      {
        if (NOW < 1022) {
          NOW++;
          input_stack_set_now(NOW);
        }
        // printf("%d\n",get_free()==1023);
        for (int j = 0; j < strlen(ptr) - i; j++) {
          printchar(' ');
        }
        for (int j = 0; j < strlen(ptr); j++) {
          printchar('\b');
        }
        char *Str = input_stack_pop();
        clean(ptr, len);
        strcpy(ptr, Str);
        i = strlen(ptr);
        print(ptr);
        i--;

      } else {
        // printf("get_free()==%d\n", get_free());
        i--;
      }
      continue;
    } else if (in == -2) {
      int bx = get_x();
      int by = get_y();
      if (NOW != BNOW) {
        // rf = 1;
        NOW--;
        input_stack_set_now(NOW);

        for (int j = 0; j < strlen(ptr) - i; j++) {
          printchar(' ');
        }
        for (int j = 0; j < strlen(ptr); j++) {
          printchar('\b');
        }
        char *Str = input_stack_pop();
        clean(ptr, len);
        strcpy(ptr, Str);
        i = strlen(ptr);
        print(ptr);
        i--;
        continue;
      }
      // i = strlen(ptr);
      i--;
      continue;
    } else if (in == -3) {
      if (i == 0) {
        i--;
        continue;
      }
      gotoxy(get_x() - 1, get_y());
      i -= 2;
      continue;
    } else if (in == -4) {
      if (ptr[i] == 0) {
        i--;
        continue;
      }
      gotoxy(get_x() + 1, get_y());
      continue;
    }
    if (ptr[i] == 0) {
      printchar(in);
      ptr[i] = in;
    } else {
      int bx = get_x(), by = get_y();
      int ry = get_raw_y();
      for (int j = 0; j < strlen(ptr) - i; j++) {
        printchar(' ');
      }
      if (bx == get_xsize())
        bx = 0;

      int BMPX = get_x();
      int BMPY = get_y();
      for (int j = 0; j < strlen(ptr); j++) {
        printchar('\b');
      }
      insert_char(ptr, i, in);

      print(ptr);
      int NX = get_x();
      int NY = get_y();
      int RNy = get_raw_y();
      if (NY >= get_ysize() - 1) {
        if (BMPY == NY && NX <= BMPX) {
          gotoxy(bx + 1, by - (RNy - ry)); //还原到它应该在的地方
        } else {
          gotoxy(bx + 1, by);
        }
      } else {
        gotoxy(bx + 1, by);
      }
    }
  }
  ptr[strlen(ptr)] = 0; //设置终止符号0

  if (Bmy >= get_ysize() - 1) {
    int bx = get_x();
    int by = get_y();
    for (int j = 0; j < strlen(ptr) - i + 1; j++) {
      gotoxy(bx + j, by);
    }
    print("\n");
  } else {
    gotoxy(0, Bmy + (strlen(ptr) / get_xsize()) + 1);
  }
  input_stack_set_now(BNOW);
  if (i > 0) {
    input_stack_put(ptr);
    input_stack_set_now(BNOW - 1); // Now-1
  }
}
void input_no_endl(char *ptr, int len) {
  int i;
  int BNOW = input_stack_get_now();
  int NOW = input_stack_get_now();
  int Bmx, Bmy;
  Bmx = get_x();
  Bmy = get_y();
  for (i = 0; i != len; i++) {
    char in = getch();
    if (in == '\n') {
      ptr[strlen(ptr)] = 0;

      if (Bmy >= get_ysize() - 1) {
        int bx = get_x();
        int by = get_y();
        for (int j = 0; j < strlen(ptr) - i + 1; j++) {
          gotoxy(bx + j, by);
        }
      } else {
        gotoxy(0, Bmy + (strlen(ptr) / get_xsize()) + 1);
      }
      input_stack_set_now(BNOW);
      if (i > 0) {
        input_stack_put(ptr);
        input_stack_set_now(BNOW - 1);
      }

      return;
    } else if (in == '\b') {
      if (i == 0) {
        i--;
        continue;
      }
      if (ptr[i] == 0) {

        i--;
        ptr[i] = 0;
        i--;
        printchar('\b');
      } else {
        int bx = get_x();
        int by = get_y();
        i--;
        delete_char(ptr, i);
        i--;
        for (int j = 0; j < strlen(ptr) - i - 1; j++) {
          printchar(' ');
        }
        for (int j = 0; j < strlen(ptr) + 1; j++) {
          printchar('\b');
        }
        print(ptr);
        gotoxy(bx - 1, by);
        // print(ptr);
      }
      continue;
    } else if (in == -1) {
      int bx = get_x();
      int by = get_y();
      if (Get_times() > 0) {

        for (int j = 0; j < strlen(ptr) - i; j++) {
          printchar(' ');
        }
        for (int j = 0; j < strlen(ptr); j++) {
          printchar('\b');
        }
        char *Str = input_stack_pop();
        strcpy(ptr, Str);
        i = strlen(ptr);
        print(ptr);
        i--;
        if (NOW < 1022) {
          NOW++;
          input_stack_set_now(NOW);
        }
      }
      continue;
    } else if (in == -2) {
      int bx = get_x();
      int by = get_y();
      if (Get_times() > 0 && NOW != BNOW) {
        NOW--;
        input_stack_set_now(NOW);

        for (int j = 0; j < strlen(ptr) - i; j++) {
          printchar(' ');
        }
        for (int j = 0; j < strlen(ptr); j++) {
          printchar('\b');
        }
        char *Str = input_stack_pop();
        strcpy(ptr, Str);
        i = strlen(ptr);
        print(ptr);
        i--;
        continue;
      }
      i = strlen(ptr);
      i--;
      continue;
    } else if (in == -3) {
      if (i == 0) {
        i--;
        continue;
      }
      gotoxy(get_x() - 1, get_y());
      i -= 2;
      continue;
    } else if (in == -4) {
      if (ptr[i] == 0) {
        i--;
        continue;
      }
      gotoxy(get_x() + 1, get_y());
      continue;
    }
    if (ptr[i] == 0) {
      printchar(in);
      ptr[i] = in;
    } else {
      int bx = get_x(), by = get_y();
      int ry = get_raw_y();
      for (int j = 0; j < strlen(ptr) - i; j++) {
        printchar(' ');
      }
      if (bx == get_xsize())
        bx = 0;

      int BMPX = get_x();
      int BMPY = get_y();
      for (int j = 0; j < strlen(ptr); j++) {
        printchar('\b');
      }
      insert_char(ptr, i, in);
      print(ptr);
      int NX = get_x();
      int NY = get_y();
      int RNy = get_raw_y();
      if (NY >= get_ysize() - 1) {
        if (BMPY == NY && NX <= BMPX) {
          gotoxy(bx + 1, by - (RNy - ry));
        } else {
          gotoxy(bx + 1, by);
        }
      } else {
        gotoxy(bx + 1, by);
      }
    }
  }
  ptr[strlen(ptr)] = 0;

  if (Bmy >= get_ysize() - 1) {
    int bx = get_x();
    int by = get_y();
    for (int j = 0; j < strlen(ptr) - i + 1; j++) {
      gotoxy(bx + j, by);
    }
  } else {
    gotoxy(0, Bmy + (strlen(ptr) / get_xsize()) + 1);
  }
  input_stack_set_now(BNOW);
  if (i > 0) {
    input_stack_put(ptr);
    input_stack_set_now(BNOW - 1);
  }
}

#include <gui.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#define Sputs(wid,str,x,y,col,xz) Sputs(wid,str,(x)*8,(y)*16,col,xz)
int x = 0;
int f = 0;
#define REDRAW_INPUT SDraw_Box(wid, 5, 28, 195, 68, 0x00ffffff, 200)
int atoi(char* str) {
  return strtol(str, NULL, 10);
}
void Cat_Char(char* _Ptr, char ch) {
  int i = 0;
  while (_Ptr[i] != '\0') {
    i++;
  }
  _Ptr[i] = ch;
  _Ptr[i + 1] = '\0';
}
int eval(char* str) {
  int i = 0;
  int sum = 0;          //结果
  char buf[100] = {0};  //储存数字
  int flag =
      -1;  //标志位:-1代表没有被使用过，0代表乘法，1代表加法，2代表减法，3代表除法
           // 4表示乘方，5表示与，6表示或，7表示左移，8表示右移
  for (i = 0; i < strlen(str); i++) {
    if (str[i] >= '0' && str[i] <= '9') {
      //就是数字，存入buf变量
      Cat_Char(buf, str[i]);
      // printf("buf=%s\n", buf);
      continue;
    }
    if (flag == -1) {
      sum = atoi(buf);
      if (str[i] == '+') {
        buf[0] = 0;
        flag = 1;
      } else if (str[i] == '-') {
        buf[0] = 0;
        flag = 2;
      } else if (str[i] == '*') {
        buf[0] = 0;
        flag = 0;
      } else if (str[i] == '/') {
        buf[0] = 0;
        flag = 3;
      }
      continue;
    } else if (flag == 1)
      sum = sum + atoi(buf);
    else if (flag == 2)
      sum = sum - atoi(buf);
    else if (flag == 3)
      sum = sum / atoi(buf);
    else if (flag == 0)
      sum = sum * atoi(buf);
    if (str[i] == '+') {
      buf[0] = '\0';
      flag = 1;
    } else if (str[i] == '-') {
      buf[0] = '\0';
      flag = 2;
    } else if (str[i] == '*') {
      buf[0] = '\0';
      flag = 0;
    } else if (str[i] == '/') {
      buf[0] = '\0';
      flag = 3;
    }
  }
  if (flag == 0) {
    sum *= atoi(buf);
  } else if (flag == 1) {
    sum += atoi(buf);
  } else if (flag == 2) {
    sum -= atoi(buf);
  } else if (flag == 3) {
    sum /= atoi(buf);
  } else {
    sum = atoi(buf);
  }
  // printf("%d\n", sum);
  return sum;
}
window_t wid;
button_t btn[3][3];
button_t add;
button_t sub;
button_t mul;
button_t div;
button_t res;
button_t ce;
button_t zero;
unsigned int mes = 0;
unsigned int length_of_window = -1;
unsigned int length_of_button = -1;
char buffer[190 / 8];
void AddChar(char c) {
  if (x <= 190) {
    char buf[2] = {c, 0};
    Sputs(wid, buf, x / 8 + 1, 2, 0x0, 200);
    buffer[x / 8] = c;
    x += 8;
  }
}
void AddStr(char* str) {
  if (x <= 190) {
    for (size_t i = 0; i < strlen(str); i++) {
      AddChar(str[i]);
    }
  }
}
int PowerMain(int argc, char **argv) {
  wid = create_window(0, 0, 200, 400, "calc");
  REDRAW_INPUT;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      char buf[10];
      sprintf(buf, "%d", i * 3 + j + 1);
      btn[i][j] = create_button(j * 52, i * 52 + 68 + 10, 50, 50, wid, buf);
    }
  }
  zero = create_button(1 * 52, 3 * 52 + 68 + 10, 50, 50, wid, "0");
  add = create_button(5, 400 - 28 - 20, 32, 16, wid, "+");
  sub = create_button(5 + 32 + 2, 400 - 28 - 20, 32, 16, wid, "-");
  mul = create_button(5 + 32 + 2 + 32 + 2, 400 - 28 - 20, 32, 16, wid, "*");
  div = create_button(5 + 32 + 2 + 32 + 2 + 32 + 2, 400 - 28 - 20, 32, 16, wid,
                      "/");
  res = create_button(5 + 32 + 2 + 32 + 2 + 32 + 2 + 32 + 2, 400 - 28 - 20, 32,
                      16, wid, "=");
  ce = create_button(5 + 32 + 2 + 32 + 2 + 32 + 2 + 32 + 2 + 32 + 2,
                     400 - 28 - 20, 20, 16, wid, "CE");
  while (1) {
    length_of_window = MessageLength(WINDOW_CLOSE_MSG_ID);
    if (length_of_window != 0xFFFFFFFF) {
      if (length_of_window != 0x4) {
        length_of_window = -1;
        length_of_button = -1;
        mes = 0;
        continue;
      }
      GetMessage(&mes, WINDOW_CLOSE_MSG_ID);
      if (mes == wid) {
        close_window(wid);
        return 0;
      }
      mes = 0;
    }
    length_of_button = MessageLength(BUTTON_MSG_ID);
    if (length_of_button != 0xFFFFFFFF) {
      if (length_of_button != 0x4) {
        length_of_window = -1;
        length_of_button = -1;
        mes = 0;
        continue;
      }
      if (f) {
        for (int i = 0; i < 190 / 8; i++) {
          buffer[i] = 0;
        }
        x = 0;
        REDRAW_INPUT;
        f = 0;
      }
      GetMessage(&mes, BUTTON_MSG_ID);
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
          if (mes == btn[i][j]) {
            // REDRAW_INPUT;
            char buf[10];
            sprintf(buf, "%d", i * 3 + j + 1);
            AddStr(buf);
            break;
          }
        }
      }
      if(mes == zero) {
        AddStr("0");
      }
      if (mes == add) {
        AddChar('+');
      }
      if (mes == sub) {
        AddChar('-');
      }
      if (mes == mul) {
        AddChar('*');
      }
      if (mes == div) {
        AddChar('/');
      }
      if (mes == ce) {
        for (int i = 0; i < 190 / 8; i++) {
          buffer[i] = 0;
        }
        x = 0;
        REDRAW_INPUT;
      }
      if (mes == res) {
        //MsgBox(buffer, "result");
        f = 1;
        REDRAW_INPUT;
        char buf[10];
        sprintf(buf, "%d", eval(buffer));
        x=0;
        AddStr(buf);
      }
      // MsgBox("Fake Button", "calc");
      mes = 0;
    }
    length_of_window = -1;
    length_of_button = -1;
  }
}
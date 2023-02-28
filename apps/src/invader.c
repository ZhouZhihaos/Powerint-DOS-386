// 从《30天自制操作系统》移植的游戏
// 改写：zhouzhihao
#include <gui.h>
#include <string.h> /* strlen */
#include <syscall.h>

#define Sputs(wid, str, x, y, col, xz) Sputs(wid, str, (x)*8, (y)*16, col, xz)
void putcharset(window_t win, int x, int y, color_t col, unsigned char* s);
void wait(int time, char* keyflag);

static unsigned char charset[16 * 8] = {

    /* invader(0) */
    0x00, 0x00, 0x00, 0x43, 0x5f, 0x5f, 0x5f, 0x7f, 0x1f, 0x1f, 0x1f, 0x1f,
    0x00, 0x20, 0x3f, 0x00,

    /* invader(1) */
    0x00, 0x0f, 0x7f, 0xff, 0xcf, 0xcf, 0xcf, 0xff, 0xff, 0xe0, 0xff, 0xff,
    0xc0, 0xc0, 0xc0, 0x00,

    /* invader(2) */
    0x00, 0xf0, 0xfe, 0xff, 0xf3, 0xf3, 0xf3, 0xff, 0xff, 0x07, 0xff, 0xff,
    0x03, 0x03, 0x03, 0x00,

    /* invader(3) */
    0x00, 0x00, 0x00, 0xc2, 0xfa, 0xfa, 0xfa, 0xfe, 0xf8, 0xf8, 0xf8, 0xf8,
    0x00, 0x04, 0xfc, 0x00,

    /* fighter(0) */
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x43, 0x47, 0x4f,
    0x5f, 0x7f, 0x7f, 0x00,

    /* fighter(1) */
    0x18, 0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0xff, 0xe7, 0xe7,
    0xe7, 0xe7, 0xff, 0x00,

    /* fighter(2) */
    0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xc2, 0xe2, 0xf2,
    0xfa, 0xfe, 0xfe, 0x00,

    /* laser */
    0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x00};
/* invader:"abcd", fighter:"efg", laser:"h" */

int PowerMain(int argc, char** argv) {
  /*
          fx：自机的x坐标（fighter_x）
          lx,ly：等离子炮弹的坐标（laser_x,laser_y）
          ix,iy：外星人群的坐标（invaders_x,in-vaders_y）
          idir：外星人群的移动方向（invaders_direc-tion）
          laserwait：等离子炮弹的剩余充电时间
          movewait：当这个变量变为0时外星人群前进一步
          movewait0：movewait的初始值（消灭30只敌人后减少）
          invline：外星人群的行数（invaders_line）
          score：当前得分
          high：最高得分
          point：得分的增加量（奖金的单价？）
          invstr：将外星人群的状态显示为字符串的变量
  */

  window_t win;
  int i, j, fx, laserwait, lx = 0, ly;
  int ix, iy, movewait0, movewait, idir;
  int invline, score, high, point;
  char invstr[32 * 6], keyflag[4], *p;
  char BMP[32];
  static char invstr0[32] = " abcd abcd abcd abcd abcd ";

  timer_alloc();

  win = create_window(150, 150, 336, 261, "Invader from 30days");
  SDraw_Box(win, 6, 27, 329, 256, 0, 336);

  high = 0;
  Sputs(win, "HIGH:00000000", 1, 2, 0x00ffffff, 336);

restart:
  score = 0;
  point = 1;
  Sputs(win, "SCORE:00000000", 19, 2, 0x00ffffff, 336);
  movewait0 = 200;
  fx = 18;
  putcharset(win, fx, 13, 0x005b8cf0, "efg");
  wait(1000, keyflag);

next_group:
  wait(100, keyflag);
  ix = 7;
  iy = 1;
  invline = 6;
  for (i = 0; i < 6; i++) {
    for (j = 0; j < 27; j++) {
      invstr[i * 32 + j] = invstr0[j];
    }
    putcharset(win, ix, iy + i, 0x0000ff00, invstr + i * 32);
  }
  keyflag[0] = 0;
  keyflag[1] = 0;
  keyflag[2] = 0;

  ly = 0; /*不显示*/
  laserwait = 0;
  movewait = movewait0;
  idir = +1;
  wait(100, keyflag);

  for (;;) {
    if (laserwait != 0) {
      laserwait--;
      keyflag[2 /* space */] = 0;
    }

    wait(40, keyflag);

    /*自机的处理*/
    if (keyflag[0 /* left */] != 0 && fx > 0) {
      fx--;
      putcharset(win, fx, 13, 0x005b8cf0, "efg ");
      keyflag[0 /* left */] = 0;
    }
    if (keyflag[1 /* right */] != 0 && fx < 37) {
      putcharset(win, fx, 13, 0x005b8cf0, " efg");
      fx++;
      keyflag[1 /* right */] = 0;
    }
    if (keyflag[2 /* space */] != 0 && laserwait == 0) {
      laserwait = 15;
      lx = fx + 1;
      ly = 13;
      ly = 13;
    }

    /*外星人移动*/
    if (movewait != 0) {
      movewait--;
    } else {
      movewait = movewait0;
      if (ix + idir > 14 || ix + idir < 0) {
        if (iy + invline == 13) {
          break; /* GAME OVER */
        }
        idir = -idir;
        putcharset(win, ix + 1, iy, 0, "                         ");
        iy++;
      } else {
        ix += idir;
      }
      for (i = 0; i < invline; i++) {
        putcharset(win, ix, iy + i, 0x0000ff00, invstr + i * 32);
      }
    }

    /*炮弹处理*/
    if (ly > 0) {
      if (ly < 13) {
        if (ix < lx && lx < ix + 25 && iy <= ly && ly < iy + invline) {
          putcharset(win, ix, ly, 0x0000ff00, invstr + (ly - iy) * 32);
        } else {
          putcharset(win, lx, ly, 0, " ");
        }
      }
      ly--;
      if (ly > 0) {
        putcharset(win, lx, ly, 0x00ffff00, "h");
      } else {
        point -= 10;
        if (point <= 0) {
          point = 1;
        }
      }
      if (ix < lx && lx < ix + 25 && iy <= ly && ly < iy + invline) {
        p = invstr + (ly - iy) * 32 + (lx - ix);
        if (*p != ' ') {
          /* hit ! */
          score += point;
          point++;
          sprintf(BMP, "SCORE:%08d", score);
          SDraw_Box(win, 19 * 8, 2 * 16, 19 * 8 + strlen(BMP) * 8, 3 * 16, 0,
                    336);
          Sputs(win, BMP, 19, 2, 0x00ffffff, 336);
          if (high < score) {
            high = score;
            SDraw_Box(win, 1 * 8, 2 * 16, 1 * 8 + strlen(BMP) * 8, 3 * 16, 0,
                      336);
            Sputs(win, BMP, 1, 2, 0x00ffffff, 336);
          }
          for (p--; *p != ' '; p--) {
          }
          for (i = 1; i < 5; i++) {
            p[i] = ' ';
          }
          putcharset(win, ix, ly, 0x0000ff00, invstr + (ly - iy) * 32);
          for (; invline > 0; invline--) {
            for (p = invstr + (invline - 1) * 32; *p != 0; p++) {
              if (*p != ' ') {
                goto alive;
              }
            }
          }
          /*全部消灭*/
          movewait0 -= movewait0 / 3;
          goto next_group;
        alive:
          ly = 0;
        }
      }
    }
  }

  /* GAME OVER */
  Sputs(win, "GAME OVER", 15, 6, 0x00ff0000, 336);
  getch();
  SDraw_Box(win, 6, 27, 329, 256, 0, 336);
  sprintf(BMP, "HIGH:%08d", high);
  SDraw_Box(win, 1 * 8, 2 * 16, 1 * 8 + strlen(BMP) * 8, 3 * 16, 0, 336);
  Sputs(win, BMP, 1, 2, 0x00ffffff, 336);
  goto restart;
}

void putcharset(window_t win, int x, int y, color_t col, unsigned char* s) {
  x = x * 8 + 8;
  y = y * 16 + 32;
  int len = strlen(s);
  SDraw_Box(win, x, y, x + len * 8, y + 16, 0, 336);
  for (int i = 0; i != len; s++, x += 8, i++) {
    char c = *s;
    if (c != ' ') {
      if ('a' <= c && c <= 'h') {
        unsigned char* p = charset + 16 * (c - 'a');
        for (int j = 0; j < 16; j++, y += 1) {
          if ((p[j] & 0x80) != 0) {
            SDraw_Px(win, x, y, col, 336);
          }
          if ((p[j] & 0x40) != 0) {
            SDraw_Px(win, x + 1, y, col, 336);
          }
          if ((p[j] & 0x20) != 0) {
            SDraw_Px(win, x + 2, y, col, 336);
          }
          if ((p[j] & 0x10) != 0) {
            SDraw_Px(win, x + 3, y, col, 336);
          }
          if ((p[j] & 0x08) != 0) {
            SDraw_Px(win, x + 4, y, col, 336);
          }
          if ((p[j] & 0x04) != 0) {
            SDraw_Px(win, x + 5, y, col, 336);
          }
          if ((p[j] & 0x02) != 0) {
            SDraw_Px(win, x + 6, y, col, 336);
          }
          if ((p[j] & 0x01) != 0) {
            SDraw_Px(win, x + 7, y, col, 336);
          }
        }
        y -= 16;
      }
    }
  }
}

void wait(int time, char* keyflag) {
  timer_settime(time);
  while (!timer_out()) {
    if (_kbhit()) {
      char c = getch();
      if (c == -3) {  // left
        keyflag[0] = 1;
      } else if (c == -4) {  // right
        keyflag[1] = 1;
      } else if (c == 0x20) {  // space
        keyflag[2] = 1;
      }
    }
  }
}

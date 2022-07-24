// Graphic.c : 图形界面的简单实现
// Windows 2000 风格
#include <dos.h>
#include <gui.h>
struct VBEINFO *vbinfo = NULL;
char mouse_cur_graphic[16][16] = {
    "*...............", "**..............", "*O*.............",
    "*OO*............", "*OOO*...........", "*OOOO*..........",
    "*OOOOO*.........", "*OOOOOO*........", "*OOOOOOO*.......",
    "*OOOO*****......", "*OO*O*..........", "*O*.*O*.........",
    "**..*O*.........", "*....*O*........", ".....*O*........",
    "......*........."};
vram_t *buf_back, *buf_win, *buf_cur, *buf_win2;
vram_t buf_mouse[256];
vram_t *buf_win_src;
struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cur, *sht_win2;
struct SHTCTL *shtctl;
struct SHEET *sht_win_;
struct SHEET *sht_b_cur;
List *list;
List *list1;
int c = 0;
struct SHTCTL *close_tid;
Button *button_cl; // 目前按下的按钮
extern char drive;
static int small_window[32] = {NULL};
void cmdline(void) {
  command_run("ver");
  while (1) {
    printchar(drive);
    print(":\\");
    print(path);
    print(">");
    int i;
    clean(line, 1024);
    input(line, 1024);
    command_run(line);
  }
}
void init_screen(vram_t *vram, int x, int y) {
  // 壁纸
  SDraw_Box(vram, 0, 0, x, y, argb(0, 58, 110, 165), x);
  // 画任务栏
  boxfill(vram, x, COL_C6C6C6, 0, y - 28, x - 1, y - 28);
  boxfill(vram, x, COL_FFFFFF, 0, y - 27, x - 1, y - 27);
  boxfill(vram, x, COL_C6C6C6, 0, y - 26, x - 1, y - 1);
}
void init_mouse_cursor(vram_t *mouse, int bc) {
  int x, y;

  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      if (mouse_cur_graphic[y][x] == '*') {
        mouse[y * 16 + x] = COL_000000;
      }
      if (mouse_cur_graphic[y][x] == 'O') {
        mouse[y * 16 + x] = COL_FFFFFF;
      }
      if (mouse_cur_graphic[y][x] == '.') {
        mouse[y * 16 + x] = bc;
      }
    }
  }
  return;
}
void window2() {
  while (1) {
    char s[512];
    sprintf(s, "%08d", c);
    putfonts_asc_sht(sht_win2, 8, 48, COL_000000, COL_C6C6C6, s, strlen(s));
    c++;
    sleep(1000);
  }
}
static void func() {
  MsgBox("Reset.", "Info");
  c = 0;
}
void small_window_handler() {
  int cnt = button_cl->x / 32;
  sheet_updown((struct SHEET *)(small_window[cnt]), shtctl->top);
  DeleteButton(button_cl);
  small_window[cnt] = NULL;
}
int gmx, gmy; // 鼠标的坐标
void graphic(void) {
  if (set_mode(1024, 768, 32) != 0) {
    printf("Can't enable 1024x768x32 VBE mode.\n\n");
    return;
  }
  gotoxy(0, 0);
  io_cli();
  // 不定 -> 定值
  vbinfo = (struct VBEINFO *)page_malloc(sizeof(struct VBEINFO));
  struct VBEINFO *vbinfo0 = (struct VBEINFO *)VBEINFO_ADDRESS;
  vbinfo->vram = vbinfo0->vram;
  vbinfo->xsize = vbinfo0->xsize;
  vbinfo->ysize = vbinfo0->ysize;
  io_sti();
  int new_mx = 0, new_my = 0;
  int new_wx = 0x7fffffff, new_wy = 0, mmx2 = 0;
  int x, y;
  int button_clicking = false;

  struct FIFO8 fifo;
  int fifobuf[128];
  int mmx = -1, mmy = -1;
  list = NewList();
  list1 = NewList();
  fifo8_init(&fifo, 128, fifobuf);
  struct SHEET *sht = (struct SHEET *)NULL;
  shtctl = shtctl_init((vram_t *)vbinfo->vram, vbinfo->xsize, vbinfo->ysize);
  static struct SHTCTL *shtctl2;
  sht_back = sheet_alloc(shtctl);
  sht_mouse = sheet_alloc(shtctl);
  sht_win = sheet_alloc(shtctl);
  sht_win2 = sheet_alloc(shtctl);
  sht_win->Close = NULL;
  sht_win2->Close = NULL;
  buf_back =
      (vram_t *)page_malloc(vbinfo->xsize * vbinfo->ysize * sizeof(color_t));
  buf_win =
      (vram_t *)page_malloc(CMDLINEXSIZE * CMDLINEYSIZE * sizeof(color_t));
  buf_win_src =
      (vram_t *)page_malloc(CMDLINEXSIZE * CMDLINEYSIZE * sizeof(color_t));
  buf_cur = (vram_t *)page_malloc(135 * 75 * sizeof(color_t));
  buf_win2 = (vram_t *)page_malloc(135 * 75 * sizeof(color_t));
  vram_t *b_cur = (vram_t *)page_malloc(16 * 8 * sizeof(color_t));

  shtctl2 = shtctl_init((vram_t *)buf_win_src, CMDLINEXSIZE, CMDLINEYSIZE);
  sht_win_ = sheet_alloc(shtctl2);
  sheet_setbuf(sht_win_, buf_win, CMDLINEXSIZE, CMDLINEYSIZE, -1);
  sheet_slide(sht_win_, 0, 0);
  sheet_updown(sht_win_, 0);
  sht_b_cur = sheet_alloc(shtctl2);
  sheet_setbuf(sht_b_cur, b_cur, 8, 16, -1);
  for (int i = 0; i < 16 * 8; i++) {
    b_cur[i] = COL_FFFFFF;
  }
  sheet_slide(sht_b_cur, 5, 24);
  sheet_slide(sht_b_cur, 5, 24);
  sheet_updown(sht_b_cur, -1);
  sheet_setbuf(sht_back, buf_back, vbinfo->xsize, vbinfo->ysize,
               -1); /* 没有透明色 */
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, COL_TRANSPARENT); /* 透明色号99 */
  sheet_setbuf(sht_win, buf_win_src, CMDLINEXSIZE, CMDLINEYSIZE, -1);
  sheet_setbuf(sht_win2, buf_win2, 135, 75, -1);
  init_screen(buf_back, vbinfo->xsize, vbinfo->ysize);
  init_mouse_cursor((vram_t *)buf_mouse, COL_TRANSPARENT); /* 背景色号99 */
  make_window(buf_win, CMDLINEXSIZE, CMDLINEYSIZE, "cmdline");
  make_window(buf_win2, 135, 75, "counter");
  boxfill(buf_win, CMDLINEXSIZE, COL_000000, 5, 24, CMDLINEXSIZE - 5,
          CMDLINEYSIZE - 5);
  sheet_slide(sht_back, 0, 0);
  gmx = (vbinfo->xsize - 16) / 2; /* 按显示在画面中央来计算坐标 */
  gmy = (vbinfo->ysize - 28 - 16) / 2;
  sheet_slide(sht_mouse, gmx, gmy);
  sheet_slide(sht_win, 100, 100);
  sheet_slide(sht_cur, 0, 0);
  sheet_slide(sht_win2, 200, 300);
  sheet_updown(sht_back, 0);
  sheet_updown(sht_win, 1);
  sheet_updown(sht_mouse, 2);
  sheet_updown(sht_cur, 1);
  sheet_updown(sht_win2, 1);
  putfonts_asc(buf_back, vbinfo->xsize, 0, 0, COL_FFFFFF,
               (unsigned char *)"Power Desktop");
  putfonts_asc(buf_back, vbinfo->xsize, 0, 1 * 16, COL_FFFFFF,
               (unsigned char *)"Powered by:");
  putfonts_asc(buf_back, vbinfo->xsize, 0, 2 * 16, COL_FFFFFF,
               (unsigned char *)"Powerint DOS 386");
  putfonts_asc(buf_back, vbinfo->xsize, 0, 3 * 16, COL_FFFFFF,
               (unsigned char *)"KernelVersion:0.6b");
  putfonts_asc(buf_back, vbinfo->xsize, 0, 4 * 16, COL_FFFFFF,
               (unsigned char *)"Copyright(C) 2022 min0911_ & zhouzhihao");
  sheet_refresh(sht_win_, 0, 0, CMDLINEXSIZE, CMDLINEYSIZE);
  sheet_refresh(sht_win, 0, 0, CMDLINEXSIZE, CMDLINEYSIZE);
  sheet_refresh(sht_back, 0, 0, vbinfo->xsize, vbinfo->ysize); /* 刷新文字 */
                                                               // for(;;);
  init_listBox();
  char *window2_esp = (char *)page_malloc(32 * 1024);
  char *cmdline_esp = (char *)page_malloc(32 * 1024);
  struct TASK *Task_Window2 = AddTask("Window2", 3, 2 * 8, (int)window2, 1 * 8,
                                      1 * 8, (int)window2_esp);
  struct TASK *Task_cmdline = AddTask("Cmdline", 2, 2 * 8, (int)cmdline, 1 * 8,
                                      1 * 8, (int)cmdline_esp);
  // 给cmdline设置fifo缓冲区
  char *cmdline_kfifo = (struct FIFO8 *)page_malloc(sizeof(struct FIFO8));
  char *cmdline_mfifo = (struct FIFO8 *)page_malloc(sizeof(struct FIFO8));
  char *cmdline_kbuf = (char *)page_malloc(32);
  char *cmdline_mbuf = (char *)page_malloc(128);
  fifo8_init(cmdline_kfifo, 32, cmdline_kbuf);
  fifo8_init(cmdline_mfifo, 128, cmdline_mbuf);
  TaskSetFIFO(Task_cmdline, cmdline_kfifo, cmdline_mfifo);
  int alloc_addr = (int)page_malloc(512 * 1024);
  char *memman = (char *)page_malloc(4 * 1024);
  Task_cmdline->alloc_addr = alloc_addr;
  Task_cmdline->memman = memman;
  sht_win->task = Task_cmdline;
  Button *Win2_Button = MakeButton(0, 0, 80, 16, sht_win2, "Reset", func);
  SleepTaskFIFO(Task_Window2); // window2记数 不需要FIFO RIGHT?
  sheet_slide(sht_cur, 188, 172 + 28);
  mouse_ready(&mdec);
  struct TIMER *timer;
  timer = timer_alloc();
  timer_init(timer, &fifo, 1);
  timer_settime(timer, 500);
  for (;;) {
    Maskirq(0);
    char s[100];
    sprintf(s, "Date:20%02x/%02x/%02x Time:%02x:%02x:%02x", read_cmos(9),
            read_cmos(8), read_cmos(7), read_cmos(4), read_cmos(2),
            read_cmos(0));
    boxfill(buf_back, vbinfo->xsize, COL_C6C6C6, 2, vbinfo->ysize - 18,
            2 + strlen(s) * 8, vbinfo->ysize - 2);
    putfonts_asc(buf_back, vbinfo->xsize, 2, vbinfo->ysize - 18, COL_000000, s);
    sheet_refresh(sht_back, 2, vbinfo->ysize - 18, 2 + strlen(s) * 8,
                  vbinfo->ysize - 2);
    if (fifo8_status(TaskGetMousefifo(NowTask())) +
            fifo8_status(TaskGetKeyfifo(NowTask())) + fifo8_status(&fifo) ==
        0) {
      if (new_mx >= 0) {
        // io_sti();
        sheet_slide(sht_mouse, new_mx, new_my);
        new_mx = -1;
      } else if (new_wx != 0x7fffffff) {
        // io_sti();
        sheet_slide(sht, new_wx, new_wy);
        new_wx = 0x7fffffff;
      } else {
      }
      WakeUp(NowTask());
      io_sti();
    } else {
      if (fifo8_status(TaskGetMousefifo(NowTask())) != 0) {
        int i = fifo8_get(TaskGetMousefifo(NowTask()));
        if (mouse_decode(&mdec, i) != 0) {
          // io_cli();
          SleepTaskFIFO(NowTask());
          gmx += mdec.x;
          gmy += mdec.y;
          if (gmx > vbinfo->xsize - 10) {
            gmx = vbinfo->xsize - 10;
          } else if (gmx < 0) {
            gmx = 0;
          }
          if (gmy > vbinfo->ysize - 16) {
            gmy = vbinfo->ysize - 16;
          } else if (gmy < 0) {
            gmy = 0;
          }
          new_mx = gmx;
          new_my = gmy;
          if ((mdec.btn & 0x01) != 0) {
            for (int i = 1; FindForCount(i, list) != NULL; i++) {
              Button *btn = (Button *)FindForCount(i, list)->val;
              if (Button_Click(btn, gmx, gmy)) {
                Button_Draw_Clicking(btn);
                btn->is_clicking = true;
                button_clicking = true;
              } else {
                if (btn->is_clicking) {
                  Button_Draw(btn);
                  btn->is_clicking = false;
                }
              }
            }
            for (int i = 1; FindForCount(i, list1) != NULL; i++) {
              TextBox *txt = (TextBox *)FindForCount(i, list1)->val;
              if (TextBox_Click(txt, gmx, gmy)) {
                txt->is_clicking = true;
              } else {
                txt->is_clicking = false;
              }
            }

            /*如果处于通常模式*/
            /*按照从上到下的顺序寻找鼠标所指向的图层*/
            if (mmx < 0) {
              struct SHEET *old = sht;
              bool is_the_same = false;
              for (int j = shtctl->top - 1; j > 0; j--) {
                sht = shtctl->sheets[j];
                x = gmx - sht->vx0;
                y = gmy - sht->vy0;
                if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
                  if (sht == old) {
                    is_the_same = true;
                  }
                  uint32_t times = (sht->bxsize - 8) / (240 - 106) + 1;
                  for (int i = 3; i < 20; i++) {
                    color_t color = argb(0, 10, 36, 106);
                    for (int j = 3, count = 0; j < sht->bxsize - 4;
                         j++, count++) {
                      if (sht->buf[j + i * sht->bxsize] != COL_FFFFFF &&
                          !(sht->bxsize - 37 <= j && j < sht->bxsize - 5 &&
                            5 <= i && i < 19)) {
                        sht->buf[j + i * sht->bxsize] = color;
                      }
                      if (count == times && (color & 0xff) != 240) {
                        color += 0x00010101;
                        count = 0;
                      }
                    }
                    sheet_refresh(sht, 3, 3, sht->bxsize - 4, 20);
                  }
                  if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
                    for (int L = 1; GetTask(L) != NULL; L++) {
                      if (GetTask(L) == NowTask()) {
                        // SubTask(L);
                        // break;
                        continue;
                      }
                      SleepTaskFIFO(GetTask(L));
                    }
                    WakeUp(sht->task);
                    sheet_updown(sht, shtctl->top - 1);
                    if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
                      mmx = gmx; /*进入窗口移动模式*/
                      mmy = gmy;
                      mmx2 = sht->vx0;
                      new_wy = sht->vy0;
                    }
                    if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 &&
                        5 <= y && y < 19) {
                      // 点击“×”按钮
                      if (sht->Close != NULL) {
                        close_tid = sht;
                        sht->Close();
                      }
                    } else if (sht->bxsize - 37 <= x && x < sht->bxsize - 21 &&
                               5 <= y && y < 19) {
                      // 点击“_”按钮
                      sheet_updown(sht, -1); // 隐藏窗口
                      int cnt;
                      for (cnt = 0; cnt != 32; cnt++) {
                        if (small_window[cnt] == NULL) {
                          Button *btn = MakeButton(
                              cnt * 32 - 3, vbinfo->ysize - 28 - 32 - 28, 32,
                              32, sht_back, "\0", small_window_handler);
                          small_window[cnt] = sht;
                          Button_Draw(btn);
                          break;
                        }
                      }
                    }
                    break;
                  }
                }
              }
              if (!is_the_same) {
                uint32_t times = (old->bxsize - 8) / (0xc6 - 0x84) + 1;
                for (int i = 3; i < 20; i++) {
                  color_t color = COL_848484;
                  for (int j = 3, count = 0; j < old->bxsize - 4;
                       j++, count++) {
                    if (old->buf[j + i * old->bxsize] != COL_FFFFFF &&
                        !(old->bxsize - 37 <= j && j < old->bxsize - 5 &&
                          5 <= i && i < 19)) {
                      old->buf[j + i * old->bxsize] = color;
                    }
                    if (count == times && color != COL_C6C6C6) {
                      color += 0x00010101;
                      count = 0;
                    }
                  }
                }
                sheet_refresh(old, 3, 3, old->bxsize - 4, 20);
              }
            } else {
              /*如果处于窗口移动模式*/
              x = gmx - mmx; /*计算鼠标的移动距离*/
              y = gmy - mmy;
              // sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
              new_wx = (mmx2 + x + 2) & ~3;
              new_wy = new_wy + y;
              // mmx = gmx; /*更新为移动后的坐标*/
              mmy = gmy;
            }
          } else if ((mdec.btn & 0x02) != 0) {
            // Sputs(buf_back, "Right", 0, 0, 0xf, 320);
            // sheet_refresh(sht_back, 0, 0, 320, 100);
          } else if ((mdec.btn & 0x04) != 0) {
            // Sputs(buf_back, "MIDDLE", 0, 0, 0xf, 320);
            // sheet_refresh(sht_back, 0, 0, 320, 100);
          } else {
            if (button_clicking) {
              for (int i = 1; FindForCount(i, list) != NULL; i++) {
                Button *btn = (Button *)FindForCount(i, list)->val;
                if (Button_Click(btn, gmx, gmy)) {
                  if (btn->is_clicking) {
                    Button_Draw(btn);
                    if (btn->OnClick != NULL) {
                      button_cl = btn;
                      btn->OnClick(btn);
                    }
                    btn->is_clicking = false;
                    break;
                  }
                }
              }
              button_clicking = false;
            }
            mmx = -1;
          }
          // Sputs(buf_back, "Left", 0, 0, 0xf, 320);
          // sheet_refresh(sht_back, 0, 0, 320, 100);
        }
        sheet_slide(sht_mouse, gmx, gmy);
        // io_sti();
      }
    }
    if (fifo8_status(TaskGetKeyfifo(NowTask())) != 0) {
      int i = fifo8_get(TaskGetKeyfifo(NowTask()));
      if (i > 0x80) {
        ClearMaskIrq(0);
        continue;
      }
      i = sc2a(i);
      for (int j = 1; FindForCount(j, list1) != NULL; j++) {
        TextBox *txt = (TextBox *)FindForCount(j, list1)->val;
        if (txt->is_clicking) {
          TextBox_AddChar(txt, i);
          break;
        }
      }
    }
    if (fifo8_status(&fifo) != 0) {
      int i = fifo8_get(&fifo);
      if (i == 1) {
        sheet_updown(sht_b_cur, -1);
        sheet_refresh(sht_win, sht_b_cur->vx0, sht_b_cur->vy0,
                      sht_b_cur->vx0 + 8, sht_b_cur->vy0 + 16);
        timer_init(timer, &fifo, 2);
        timer_settime(timer, 500);
      } else if (i == 2) {
        sheet_updown(sht_b_cur, 1);
        sheet_refresh(sht_win, sht_b_cur->vx0, sht_b_cur->vy0,
                      sht_b_cur->vx0 + 8, sht_b_cur->vy0 + 16);
        timer_init(timer, &fifo, 1);
        timer_settime(timer, 500);
      }
    }
    ClearMaskIrq(0);
  }
}
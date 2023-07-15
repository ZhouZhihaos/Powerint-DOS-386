// Graphic.c : 图形界面的简单实现
// Windows 2000 风格
#include <dos.h>
struct VBEINFO *vbinfo = NULL;
static char mouse_cur_graphic[16][16] = {
    "*...............", "**..............", "*O*.............",
    "*OO*............", "*OOO*...........", "*OOOO*..........",
    "*OOOOO*.........", "*OOOOOO*........", "*OOOOOOO*.......",
    "*OOOO*****......", "*OO*O*..........", "*O*.*O*.........",
    "**..*O*.........", "*....*O*........", ".....*O*........",
    "......*........."};
int gmx, gmy; // 鼠标的坐标
struct SHTCTL *shtctl;
struct SHEET *sht_back, *sht, *close_tid;
List *list;        // Button
List *list1;       // TextBox
List *list2;       // NoFrameButton
Button *button_cl; // 目前按下的按钮
static List *list_ipc;
static struct tty *tty_g, *tty_default_, *tty_now_;
static int small_window[32] = {NULL};
static void cmdline(void) {
  shell_handler();
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
static void small_window_handler() {
  int cnt = button_cl->x / 32;
  sheet_updown((struct SHEET *)(small_window[cnt]), shtctl->top);
  DeleteButton(button_cl);
  small_window[cnt] = NULL;
}
void graphic(void) {
  if (set_mode(1024, 768, 32) != 0) {
    printf("Can't enable 1024x768x32 VBE mode.\n\n");
    return;
  }
  io_cli();
  // 不定 -> 定值
  vbinfo = (struct VBEINFO *)page_malloc(sizeof(struct VBEINFO));
  struct VBEINFO *vbinfo0 = (struct VBEINFO *)VBEINFO_ADDRESS;
  vbinfo->vram = vbinfo0->vram;
  vbinfo->xsize = vbinfo0->xsize;
  vbinfo->ysize = vbinfo0->ysize;

  // 桌面
  int new_mx = 0, new_my = 0;
  int new_wx = 0x7fffffff, new_wy = 0, mmx2 = 0;
  int x, y;
  bool button_clicking = false;
  bool sheet_updown_flags = true;
  struct FIFO8 fifo;
  struct FIFO8 fifo2;
  int fifobuf[32];
  int fifobuf2[32];
  int mmx = -1, mmy = -1;
  fifo8_init(&fifo, 32, fifobuf);
  fifo8_init(&fifo2, 32, fifobuf2);
  sht = (struct SHEET *)NULL;
  shtctl = shtctl_init((vram_t *)vbinfo->vram, vbinfo->xsize, vbinfo->ysize);
  static struct SHTCTL *shtctl2;
  sht_back = sheet_alloc(shtctl);
  struct SHEET *sht_mouse = sheet_alloc(shtctl);
  struct SHEET *sht_win = sheet_alloc(shtctl);
  sht_win->Close = NULL;
  vram_t *buf_back =
      (vram_t *)page_malloc(vbinfo->xsize * vbinfo->ysize * sizeof(color_t));
  vram_t *buf_win =
      (vram_t *)page_malloc(CMDLINEXSIZE * CMDLINEYSIZE * sizeof(color_t));
  vram_t *buf_win_src =
      (vram_t *)page_malloc(CMDLINEXSIZE * CMDLINEYSIZE * sizeof(color_t));
  vram_t *b_cur = (vram_t *)page_malloc(16 * 8 * sizeof(color_t));
  shtctl2 = shtctl_init((vram_t *)buf_win_src, CMDLINEXSIZE, CMDLINEYSIZE);
  struct SHEET *sht_win_ = sheet_alloc(shtctl2);
  sheet_setbuf(sht_win_, buf_win, CMDLINEXSIZE, CMDLINEYSIZE, -1);
  sheet_slide(sht_win_, 0, 0);
  sheet_updown(sht_win_, 0);
  struct SHEET *sht_b_cur = sheet_alloc(shtctl2);
  sheet_setbuf(sht_b_cur, b_cur, 8, 16, -1);
  for (int i = 0; i < 16 * 8; i++) {
    b_cur[i] = COL_FFFFFF;
  }
  sheet_slide(sht_b_cur, 5, 24);
  sheet_slide(sht_b_cur, 5, 24);
  sheet_updown(sht_b_cur, -1);
  sheet_setbuf(sht_back, buf_back, vbinfo->xsize, vbinfo->ysize,
               -1); /* 没有透明色 */
  static vram_t buf_mouse[256];
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, COL_TRANSPARENT); /* 透明色号99 */
  sheet_setbuf(sht_win, buf_win_src, CMDLINEXSIZE, CMDLINEYSIZE, -1);
  init_screen(buf_back, vbinfo->xsize, vbinfo->ysize);
  init_mouse_cursor((vram_t *)buf_mouse, COL_TRANSPARENT); /* 背景色号99 */
  make_window(buf_win, CMDLINEXSIZE, CMDLINEYSIZE, "控制台");
  boxfill(buf_win, CMDLINEXSIZE, COL_000000, 5, 24, CMDLINEXSIZE - 5,
          CMDLINEYSIZE - 5);
  sheet_slide(sht_back, 0, 0);
  gmx = (vbinfo->xsize - 16) / 2; /* 按显示在画面中央来计算坐标 */
  gmy = (vbinfo->ysize - 28 - 16) / 2;
  sheet_slide(sht_mouse, gmx, gmy);
  sheet_slide(sht_win, 100, 100);
  sheet_updown(sht_back, 0);
  sheet_updown(sht_win, 1);
  sheet_updown(sht_mouse, 2);
  putfonts_asc(buf_back, vbinfo->xsize, 2, 2, COL_FFFFFF,
               (unsigned char *)"Powerint DOS 桌面环境");
  putfonts_asc(buf_back, vbinfo->xsize, 2, 1 * 16 + 6, COL_FFFFFF,
               (unsigned char *)"基于内核：Powerint DOS 386");
  putfonts_asc(buf_back, vbinfo->xsize, 2, 2 * 16 + 10, COL_FFFFFF,
               (unsigned char *)"内核版本：0.7a");
  putfonts_asc(buf_back, vbinfo->xsize, 2, 3 * 16 + 14, COL_FFFFFF,
               (unsigned char *)"版权所有 2022 min0911_ & zhouzhihao");
  sheet_refresh(sht_win_, 0, 0, CMDLINEXSIZE, CMDLINEYSIZE);
  sheet_refresh(sht_win, 0, 0, CMDLINEXSIZE, CMDLINEYSIZE);
  sheet_refresh(sht_back, 0, 0, vbinfo->xsize, vbinfo->ysize); /* 刷新文字 */
                                                               // for(;;);
  // 控件
  list = NewList();
  list1 = NewList();
  list2 = NewList();
  list_ipc = NewList();
  init_listBox();
  // cmdline
  char *cmdline_esp = (char *)page_malloc(32 * 1024);
  struct TASK *Task_cmdline = AddTask("Cmdline", 2, 2 * 8, (int)cmdline, 1 * 8,
                                      1 * 8, (int)cmdline_esp);
  char *cmdline_kfifo = (struct FIFO8 *)page_malloc(sizeof(struct FIFO8));
  char *cmdline_mfifo = (struct FIFO8 *)page_malloc(sizeof(struct FIFO8));
  char *cmdline_kbuf = (char *)page_malloc(128);
  char *cmdline_mbuf = (char *)page_malloc(128);
  fifo8_init(cmdline_kfifo, 128, cmdline_kbuf);
  fifo8_init(cmdline_mfifo, 128, cmdline_mbuf);
  TaskSetFIFO(Task_cmdline, cmdline_kfifo, cmdline_mfifo);
  int alloc_addr = (int)page_malloc(512 * 1024);
  Task_cmdline->alloc_addr = alloc_addr;
  Task_cmdline->alloc_size = 512 * 1024;
  init_mem(Task_cmdline);
  sht_win->task = Task_cmdline;
  mouse_ready(&mdec);
  struct TIMER *timer;
  struct TIMER *timer2;
  timer = timer_alloc();
  timer2 = timer_alloc();
  timer_init(timer, &fifo, 1);
  timer_init(timer2, &fifo2, 1);
  timer_settime(timer, 500);
  timer_settime(timer2, 1200);
  // gui_login();
  tty_g = tty_alloc((void *)sht_win_, 80, 25, putchar_GraphicMode,
                    MoveCursor_GraphicMode, clear_GraphicMode,
                    screen_ne_GraphicMode, Draw_Box_GraphicMode);
  tty_set_reserved(tty_g, (unsigned int)sht_b_cur, (unsigned int)sht_win, 0, 0);
  tty_default_ = tty_set_default(tty_g);
  tty_now_ = tty_set(current_task(), tty_g);
  tty_set(Task_cmdline, tty_g);
  io_sti();
  for (;;) {
    Maskirq(0);
    if (shtctl->sheets[shtctl->top - 1]->task->TTY->reserved[0] *
            shtctl->sheets[shtctl->top - 1]->task->TTY->reserved[1] !=
        NULL) {
      sht_win = (struct SHEET *)shtctl->sheets[shtctl->top - 1]
                    ->task->TTY->reserved[1];
      sht_b_cur = (struct SHEET *)shtctl->sheets[shtctl->top - 1]
                      ->task->TTY->reserved[0];
    } else {
      sht_win = (struct SHEET *)current_task()->TTY->reserved[1];
      sht_b_cur = (struct SHEET *)current_task()->TTY->reserved[0];
    }
    char s[100];
    sprintf(s, "日期:20%02x年%02x月%02x日 时间:%02x时%02x分%02x秒",
            read_cmos(9), read_cmos(8), read_cmos(7), read_cmos(4),
            read_cmos(2), read_cmos(0));
    boxfill(buf_back, vbinfo->xsize, COL_C6C6C6, 2, vbinfo->ysize - 18,
            2 + strlen(s) * 8, vbinfo->ysize - 2);
    putfonts_asc(buf_back, vbinfo->xsize, 2, vbinfo->ysize - 18, COL_000000, s);
    sheet_refresh(sht_back, 2, vbinfo->ysize - 18, 2 + strlen(s) * 8,
                  vbinfo->ysize - 2);
    /*if (get_login_flag() == 1) {
      sheet_updown(sht_fm, 1);
      sheet_updown(sht_win, 1);
      clean(line, 1024);
    } else if (get_login_flag() == 0) {
      sheet_updown(sht_fm, -1);
      sheet_updown(sht_win, -1);
    }*/
    if (fifo8_status(TaskGetMousefifo(current_task())) +
            fifo8_status(TaskGetKeyfifo(current_task())) + fifo8_status(&fifo) ==
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
      WakeUp(current_task());
      io_sti();
    } else {
      if (fifo8_status(TaskGetMousefifo(current_task())) != 0) {
        // printk("Mouse.\n");
        int i = fifo8_get(TaskGetMousefifo(current_task()));
        if (mouse_decode(&mdec, i) != 0) {
          // io_cli();
          SleepTaskFIFO(current_task());
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
          for (int i = 1; FindForCount(i, list2) != NULL; i++) {
            Button *btn = (Button *)FindForCount(i, list2)->val;
            if (Button_Click(btn, gmx, gmy)) {
              Button_Draw_NoFrame_Clicking(btn);
            } else {
              Button_Draw_NoFrame(btn);
            }
          }
          if (mdec.roll == MOUSE_ROLL_DOWN) {
            ListBox_Roll_Down();
          } else if (mdec.roll == MOUSE_ROLL_UP) {
            ListBox_Roll_Up();
          }
          if ((mdec.btn & 0x01) != 0) {
            sheet_updown_flags = true;
            for (int i = 1; FindForCount(i, list) != NULL; i++) {
              Button *btn = (Button *)FindForCount(i, list)->val;
              if (Button_Click(btn, gmx, gmy)) {
                Button_Draw_Clicking(btn);
                btn->is_clicking = true;
                button_clicking = true;
                sheet_updown_flags = false;
              } else {
                if (btn->is_clicking) {
                  Button_Draw(btn);
                  btn->is_clicking = false;
                }
              }
            }
            for (int i = 1; FindForCount(i, list2) != NULL; i++) {
              Button *btn = (Button *)FindForCount(i, list2)->val;
              if (Button_Click(btn, gmx, gmy)) {
                if (btn->OnClick != NULL && btn->clicking_first) {
                  sheet_updown_flags = false;
                  button_cl = btn;
                  btn->OnClick(btn);
                }
                btn->clicking_first ^= 1;
              } else if (btn->clicking_first) {
                btn->clicking_first = 0;
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
            if (mmx < 0 && sheet_updown_flags) {
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
                  uint32_t times = (sht->bxsize - 8) / (255 - 106) + 1;
                  for (int i = 3; i < 20; i++) {
                    color_t color = argb(0, 10, 36, 106);
                    for (int j = 3, count = 0; j < sht->bxsize - 4;
                         j++, count++) {
                      if (sht->buf[j + i * sht->bxsize] != COL_FFFFFF &&
                          !(sht->bxsize - 37 <= j && j < sht->bxsize - 5 &&
                            5 <= i && i < 19)) {
                        sht->buf[j + i * sht->bxsize] = color;
                      }
                      if (count == times && (color & 0xff) != 255) {
                        color += 0x00010101;
                        count = 0;
                      }
                    }
                    sheet_refresh(sht, 3, 3, sht->bxsize - 4, 20);
                  }
                  if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
                    // for (int L = 1; GetTask(L) != NULL; L++) {
                    //   if (GetTask(L) == current_task()) {
                    //  SubTask(L);
                    //  break;
                    //    continue;
                    //  }
                    //  SleepTaskFIFO(GetTask(L));
                    //}
                    // printk("%s:mouse:%d,%d\n",sht->task->name,x,y);
                    unsigned int xy = x << 16 | y;
                    SendIPCMessageTID(get_tid(sht->task), -3, &xy, 4,
                                      asynchronous);
                    AddVal(get_tid(sht->task), list_ipc);
                    if (old->task != current_task()) {
                      SleepTaskFIFO(old->task);
                      if (sht != old) {
                        // printf("Change -> %s\n",old->task->name);
                        change_level(old->task, 3);
                      }
                    }
                    WakeUp(sht->task);
                    if (sht->task != current_task()) {
                      change_level(sht->task, 1);
                    }
                    for (int j = 1; GetTask(j) != 0; j++) {
                      struct TASK *t = GetTask(j);
                      // printk("%s:%08x %s:%08x\n", t->name, t->TTY,
                      //        current_task()->name, current_task()->TTY);
                      if (t->TTY == current_task()->TTY && t->app && !t->forever) {
                        SleepTaskFIFO(sht->task);
                        break;
                      }
                    }
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
                              32, sht_back, "", small_window_handler);
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
            } else if (sheet_updown_flags) {
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
    if (fifo8_status(TaskGetKeyfifo(current_task())) != 0) {
      int i = fifo8_get(TaskGetKeyfifo(current_task()));
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
    if (fifo8_status(&fifo2) != 0) {
      int i = fifo8_get(&fifo2);
      if (i == 1) {
        io_cli();
        for (int j = 1; FindForCount(j, list_ipc) != NULL; j++) {
          int tid = FindForCount(j, list_ipc)->val;
          unsigned int tmp;
          GetIPCMessageTID(tid, &tmp, -3);
          DeleteVal(j, list_ipc);
        }
        io_sti();
        timer_settime(timer2, 1200);
      }
    }
    ClearMaskIrq(0);
  }
}

#include <box.hpp>
#include <pbutton.hpp>
#include <pconsole.hpp>
#include <psheet.hpp>
#include <textbox.hpp>
#include <window.hpp>

extern "C" void wav_player(char* filename);
extern "C" int sc2a(int sc);
static struct VBEINFO* vbinfo;
struct SHTCTL* ctl;
PSheetBase *main_sheet, *mouse_sheet;
PSheetBase* new_window2;
WindowBox* click_left;
WindowBox* click_right;
WindowBox* stay_box;
PSheetBase* left_last;
PSheetBase* stay_last;
PSheetBase* right_last;
int pgmx = 0, pgmy = 0;
static char* mouse_cur_graphic[19] = {
    "*...............", "**..............", "*O*.............",
    "*OO*............", "*OOO*...........", "*OOOO*..........",
    "*OOOOO*.........", "*OOOOOO*........", "*OOOOOOO*.......",
    "*OOOOOOOO*......", "*OOOOO*****.....", "*OO*OO*.........",
    "*O*.*OO*........", "**..*OO*........", "*....*OO*.......",
    ".....*OO*.......", "......*OO*......", "......*OO*......",
    ".......**......."};

void pgui_init_mouse_cursor(vram_t* mouse, int bc) {
  int x, y;
  for (y = 0; y < 19; y++) {
    for (x = 0; x < 16; x++) {
      if (mouse_cur_graphic[y][x] == '*') {
        mouse[y * 16 + x] = COL_000000;
      } else if (mouse_cur_graphic[y][x] == 'O') {
        mouse[y * 16 + x] = COL_FFFFFF;
      } else if (mouse_cur_graphic[y][x] == '.') {
        mouse[y * 16 + x] = bc;
      }
    }
  }
}
static void handle() {
  for (;;) {
    if (fifo8_status(TaskGetMousefifo(current_task())) +
            fifo8_status(TaskGetKeyfifo(current_task())) ==
        0) {
      WakeUp(current_task());
    } else {
      //  io_cli();
      if (fifo8_status(TaskGetMousefifo(current_task())) != 0) {
        // printk("Mouse.\n");
        SleepTaskFIFO(current_task());
        int i = fifo8_get(TaskGetMousefifo(current_task()));
        if (mouse_decode(&mdec, i) != 0) {
          // io_cli();
          pgmx += mdec.x;
          pgmy += mdec.y;
          if (pgmx > vbinfo->xsize) {
            pgmx = vbinfo->xsize;
          } else if (pgmx < -mouse_sheet->get_xsize()) {
            pgmx = -mouse_sheet->get_xsize();
          }
          if (pgmy > vbinfo->ysize) {
            pgmy = vbinfo->ysize;
          } else if (pgmy < -mouse_sheet->get_ysize()) {
            pgmy = -mouse_sheet->get_xsize();
          }
          if ((mdec.btn & 0x01) != 0) {
            left_last = click_left->click_handle(ctl, pgmx, pgmy);
            right_last = nullptr;
            stay_last = nullptr;
          } else if ((mdec.btn & 0x02) != 0) {
            right_last = click_right->click_handle(ctl, pgmx, pgmy);
            left_last = nullptr;
            stay_last = nullptr;
          } else if ((mdec.btn & 0x04) != 0) {
            // do nothing
          } else {
            stay_last = stay_box->click_handle(ctl, pgmx, pgmy);
            left_last = nullptr;
            right_last = nullptr;
          }
          mouse_sheet->slide(pgmx, pgmy);
        }
      }
      if (fifo8_status(TaskGetKeyfifo(current_task())) != 0) {
        int i = fifo8_get(TaskGetKeyfifo(current_task()));
        struct SHEET* sht = ctl->sheets[ctl->top - 1];
        ((PSheetBase*)sht->args)->key_press_handle(i);
      }
      //   io_sti();
    }
  }
}
void onclick1(PSheetBase* ps, int x, int y, uint32_t val) {
  SDraw_Box(ps->get_vram(), 0, 0, 100, 100, COL_848484, ps->get_xsize());
  ps->refresh(0, 0, 100, 100);
}
void onclick2(PSheetBase* ps, int x, int y, uint32_t val) {
  SDraw_Box(ps->get_vram(), 0, 0, 100, 100, COL_000000, ps->get_xsize());
  ps->refresh(0, 0, 100, 100);
}
Window* wnd10;
void btn_click(PButton* pbtn, uint32_t val) {
  wav_player("other/coin.wav");
}
void btn_click1(PButton* pbtn, uint32_t val) {
  command_run("halt");
}
void btn_click2(PButton* pbtn, uint32_t val) {
  command_run("reboot");
}
void btn_click3(PButton* pbtn, uint32_t val) {
  PTextBox* pt = (PTextBox*)val;
  wav_player(pt->Text());
}
PButton* w_btn;
Window* wnd1;
int flag = 0;
void btn_click4(PButton* pbtn, uint32_t val) {
  if (flag) {
    flag = 0;
    return;
  }
  flag = 1;
  if (!fork()) {
    while (flag) {
      srand(timerctl.count);
      w_btn->slide(rand() % (wnd1->get_xsize()), rand() % (wnd1->get_ysize()));
      sleep(100);
    }
    SubTask(current_task());
  }
}
void key(char ch, uint32_t val) {
  char buf[2];
  sprintf(buf, "%c", ch);
  wnd10 = new Window(ctl, buf, 0, 0, 100, 100);
}
void pgui_shell() {
  io_cli();
  PConsole* pcons = new PConsole(ctl, 100, 100);
  register_tty(pcons);
  io_sti();
  shell_handler();
  for (;;)
    ;
}
void start(PButton* pbtn, uint32_t val) {
  struct FIFO8* fifo = (struct FIFO8*)malloc(sizeof(struct FIFO8));
  struct FIFO8* fifo2 = (struct FIFO8*)malloc(sizeof(struct FIFO8));
  uint8_t* fifobuf = new uint8_t[32];
  uint8_t* fifobuf2 = new uint8_t[32];
  io_cli();
  fifo8_init(fifo, 32, fifobuf);
  fifo8_init(fifo2, 32, fifobuf2);
  struct TASK* t = AddTask("cmdline", 1, 2 * 8, (int)pgui_shell, 1 * 8, 1 * 8,
                           (int)page_malloc(32 * 1024) + 32 * 1024);
  int alloc_addr = (int)page_kmalloc(512 * 1024);
  t->alloc_addr = alloc_addr;
  t->alloc_size = 512 * 1024;
  init_mem(t);
  TaskSetFIFO(t, fifo, fifo2);
  SleepTaskFIFO(t);
  io_sti();
}
void pgui_main() {
  set_mode(1024, 768, 32);
  click_left = new WindowBox();
  click_right = new WindowBox();
  stay_box = new WindowBox();
  vbinfo = (struct VBEINFO*)page_malloc(sizeof(struct VBEINFO));
  struct VBEINFO* vbinfo0 = (struct VBEINFO*)VBEINFO_ADDRESS;
  vbinfo->vram = vbinfo0->vram;
  vbinfo->xsize = vbinfo0->xsize;
  vbinfo->ysize = vbinfo0->ysize;
  ctl = shtctl_init((vram_t*)vbinfo->vram, vbinfo->xsize, vbinfo->ysize);
  main_sheet =
      new PSheetBase(ctl, vbinfo->xsize, vbinfo->ysize, 0, 0, COL_TRANSPARENT);
  printk("A object news.\n");
  memset(main_sheet->get_vram(), 0xff,
         main_sheet->get_xsize() * main_sheet->get_ysize() * 4);
  printk("set ok\n");
  main_sheet->updown(0);
  main_sheet->refresh(0, 0, main_sheet->get_xsize(), main_sheet->get_ysize());
  mouse_sheet = new PSheetBase(ctl, 16, 19, 0, 0, COL_TRANSPARENT);
  pgui_init_mouse_cursor(mouse_sheet->get_vram(), COL_TRANSPARENT);
  mouse_sheet->updown(2);
  mouse_sheet->refresh(0, 0, mouse_sheet->get_xsize(),
                       mouse_sheet->get_ysize());
  pgmx = 0;
  pgmy = 0;
  Window* wnd = new Window(ctl, "RabbitWindow", 300, 100, 400, 250);
  wnd1 = new Window(ctl, "RabbitWindow1", 300, 100, 400, 250);
  w_btn = new PButton(wnd1, "start", 2, 28, 100, 20, 0, btn_click4);
  PTextBox* pt = new PTextBox(wnd, 2, 50, 200, 20);
  PButton* btn = new PButton(wnd, "sb16", 2, 28, 100, 20, 0, btn_click);
  PButton* btn1 = new PButton(wnd, "start", 103, 28, 100, 20, 0, start);
  PButton* btn3 =
      new PButton(wnd, "play", 203, 50, 100, 20, (uint32_t)pt, btn_click3);

  mouse_ready(&mdec);
  handle();
}

extern "C" void c_pgui_main() {
  pgui_main();
}
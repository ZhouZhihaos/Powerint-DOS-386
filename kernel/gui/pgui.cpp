
#include <box.hpp>
#include <pbutton.hpp>
#include <psheet.hpp>
#include <window.hpp>

extern "C" void wav_player(char* filename);
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
    if (fifo8_status(TaskGetMousefifo(current_task())) == 0) {
      WakeUp(current_task());
    } else {
      //  io_cli();
      if (fifo8_status(TaskGetMousefifo(current_task())) != 0) {
        // printk("Mouse.\n");
        int i = fifo8_get(TaskGetMousefifo(current_task()));
        if (mouse_decode(&mdec, i) != 0) {
          // io_cli();
          SleepTaskFIFO(current_task());
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
  Window* wnd = new Window(ctl, "testWindow", 300, 100, 400, 250);
  PButton* btn = new PButton(wnd, "sb16", 2, 28, 100, 20, 0, btn_click);
  PButton* btn1 = new PButton(wnd, "halt", 103, 28, 100, 20, 0, btn_click1);
  PButton* btn2 = new PButton(wnd, "reboot", 204, 28, 100, 20, 0, btn_click2);
  mouse_ready(&mdec);
  handle();
}

extern "C" void c_pgui_main() {
  pgui_main();
}
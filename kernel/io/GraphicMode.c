#include <dos.h>
extern color_t color_table[16];
void putchar_GraphicMode(struct tty* res, int c) {
  struct SHEET* sht = (struct SHEET*)res->vram;
  struct SHEET* sht2 = (struct SHEET*)res->reserved[1];
  // printk("%d %d\n",res->x,res->xsize);
  if (c == '\n' || res->x * 8 + 5 >= res->xsize *8) {
    if (res->y == res->ysize - 1) {
      res->screen_ne(res);
    } else {
      res->y++;
    }
    res->x = 0;
    if (c == '\n') {
      return;
    }
  }
  if (c == '\r') {
    res->x = 0;
    return;
  } else if (c == '\b') {
  //  printk("res->x=%d\n",res->x);
    if (res->x >= 1) {
      res->x--;
    } else if (res->y > 0) {
      res->y--;
      res->x = res->xsize - 1;
    }
    boxfill(sht->buf, sht->bxsize, color_table[res->color >> 4], res->x * 8 + 5,
            res->y * 16, res->x * 8 + 8 + 5, res->y * 16 + 24 + 16);
    sheet_refresh(sht, res->x * 8 + 5, res->y * 16 + 24, res->x * 8 + 5 + 8,
                  res->y * 16 + 24 + 16);
    sheet_refresh(sht2, res->x * 8 + 5, res->y * 16 + 24, res->x * 8 + 5 + 8,
                  res->y * 16 + 24 + 16);
  } else {
    if (c != '\b') {
      boxfill(sht->buf, sht->bxsize, color_table[res->color >> 4],
              res->x * 8 + 5, res->y * 16 + 24, res->x * 8 + 5 + 8,
              res->y * 16 + 24 + 16);
      SDraw_Char(sht->buf, res->x * 8 + 5, res->y * 16 + 24, (char)c,
                 color_table[res->color & 0xf], sht->bxsize);
      sheet_refresh(sht, res->x * 8 + 5, res->y * 16 + 24, res->x * 8 + 5 + 8,
                    res->y * 16 + 24 + 16);
      sheet_refresh(sht2, res->x * 8 + 5, res->y * 16 + 24, res->x * 8 + 5 + 8,
                    res->y * 16 + 24 + 16);
      res->x++;
    } else {
      // 画个8*16的黑色矩形
      boxfill(sht->buf, sht->bxsize, 0, res->x * 8 + 5 - 8, res->y * 16 + 24,
              res->x * 8 + 5 + 8, res->y * 16 + 24 + 16);
      sheet_refresh(sht, res->x * 8 + 5 - 8, res->y * 16 + 24,
                    res->x * 8 + 5 + 8, res->y * 16 + 24 + 16);
      sheet_refresh(sht2, res->x * 8 + 5 - 8, res->y * 16 + 24,
                    res->x * 8 + 5 + 8, res->y * 16 + 24 + 16);
    }
  }
  res->MoveCursor(res, res->x, res->y);
}
void clear_GraphicMode(struct tty* res) {
  io_cli();
  struct SHEET* sht = (struct SHEET*)res->vram;
  struct SHEET* sht2 = (struct SHEET*)res->reserved[1];
  boxfill(sht->buf, sht->bxsize, color_table[res->color >> 4], 5, 24,
          res->xsize * 8 + 5, res->ysize * 16 + 24);
  sheet_refresh(sht, 5, 24, res->xsize * 8 + 5, res->ysize * 16 + 24);
  sheet_refresh(sht2, 5, 24, res->xsize * 8 + 5, res->ysize * 16 + 24);
  res->MoveCursor(res, 0, 0);
  io_sti();
}
void MoveCursor_GraphicMode(struct tty* res, int x, int y) {
  struct SHEET* sht = (struct SHEET*)res->vram;
  struct SHEET* sht2 = (struct SHEET*)res->reserved[1];
  struct SHEET* sht_cur = (struct SHEET*)res->reserved[0];
  res->x = x;
  res->y = y;
  int ox = sht_cur->vx0;
  int oy = sht_cur->vy0;
  sheet_slide(sht_cur, x * 8 + 5, y * 16 + 24);
  sheet_refresh(sht2, ox, oy, ox + 8, oy + 16);
  sheet_refresh(sht2, (x * 8 + 5), (y * 16 + 24), (x * 8 + 5) + 8,
                (y * 16 + 24) + 16);
  return;
}
vram_t* vramcpy(vram_t* s, const vram_t* ct, size_t n) {
  if (NULL == s || NULL == ct || n <= 0)
    return NULL;
  while (n--)
    *(vram_t*)s++ = *(vram_t*)ct++;
  return s;
}
void screen_ne_GraphicMode(struct tty* res) {
  io_cli();
  struct SHEET* sht = (struct SHEET*)res->vram;
  struct SHEET* sht2 = (struct SHEET*)res->reserved[1];
  for (int j = 0; j != 16; j++) {
    for (int i = 0; i != 25 * 16; i++) {
      vramcpy(sht->buf + (sht->bxsize * (24 + i)) + 5,
              sht->buf + (sht->bxsize * (24 + i + 1)) + 5, sht->bxsize - 10);
    }
  }
  sheet_refresh(sht, 0, 0, sht->bxsize, sht->bysize);
  sheet_refresh(sht2, 0, 0, sht->bxsize, sht->bysize);
  io_sti();
}
void Draw_Box_GraphicMode(struct tty* res,
                          int x,
                          int y,
                          int x1,
                          int y1,
                          unsigned char color) {
  io_cli();
  struct SHEET* sht = (struct SHEET*)res->vram;
  struct SHEET* sht2 = (struct SHEET*)res->reserved[1];
  for (int i = y * 16 + 24; i <= y1 * 16 + 24; i++) {
    for (int j = x * 8 + 5; j <= x1 * 8 + 5; j++) {
      if (sht->buf[i * sht->bxsize + j] == color_table[res->color & 0xf]) {
        sht->buf[i * sht->bxsize + j] = color_table[color & 0xf];
      } else {
        sht->buf[i * sht->bxsize + j] = color_table[color >> 4];
      }
    }
  }
  sheet_refresh(sht, x * 8 + 5, y * 16 + 24, x1 * 8 + 5, y1 * 16 + 24);
  sheet_refresh(sht2, x * 8 + 5, y * 16 + 24, x1 * 8 + 5, y1 * 16 + 24);
  io_sti();
}
extern struct SHTCTL* shtctl;
void AddShell_GraphicMode() {
  io_cli();
  struct TASK* task = NowTask();
  struct SHEET* sht_win = sheet_alloc(shtctl);
  sht_win->Close = NULL;
  vram_t* vram1 =
      (vram_t*)page_kmalloc(CMDLINEXSIZE * CMDLINEYSIZE * sizeof(color_t));
  vram_t* vram2 =
      (vram_t*)page_kmalloc(CMDLINEXSIZE * CMDLINEYSIZE * sizeof(color_t));
  struct SHTCTL* shtctl2 = shtctl_init(vram2, CMDLINEXSIZE, CMDLINEYSIZE);
  struct SHEET* sht_win_ = sheet_alloc(shtctl2);
  sheet_setbuf(sht_win_, vram1, CMDLINEXSIZE, CMDLINEYSIZE, -1);
  sheet_setbuf(sht_win, vram2, CMDLINEXSIZE, CMDLINEYSIZE, -1);
  make_window(vram1, CMDLINEXSIZE, CMDLINEYSIZE, "控制台");
  sheet_slide(sht_win_, 0, 0);
  sheet_updown(sht_win_, 0);
  struct SHEET* sht_b_cur = sheet_alloc(shtctl2);
  vram_t* vram3 = (vram_t*)page_malloc(8 * 16 * sizeof(color_t));
  sheet_setbuf(sht_b_cur, vram3, 8, 16, -1);
  for (int i = 0; i < 16 * 8; i++) {
    vram3[i] = COL_FFFFFF;
  }
  sheet_slide(sht_b_cur, 5, 24);
  sheet_slide(sht_b_cur, 5, 24);
  sheet_updown(sht_b_cur, -1);
  boxfill(vram1, CMDLINEXSIZE, COL_000000, 5, 24, CMDLINEXSIZE - 5,
          CMDLINEYSIZE - 5);
  sheet_updown(sht_win, shtctl->top - 1);
  struct SHEET* now_sht = (struct SHEET*)task->TTY->reserved[1];
  sheet_slide(sht_win, now_sht->vx0 + 5, now_sht->vy0 + 24);
  sheet_refresh(sht_win_, 0, 0, CMDLINEXSIZE, CMDLINEYSIZE);
  sheet_refresh(sht_win, 0, 0, CMDLINEXSIZE, CMDLINEYSIZE);
  struct tty* ntty =
      tty_alloc((void*)sht_win_, task->TTY->xsize, task->TTY->ysize,
                putchar_GraphicMode, MoveCursor_GraphicMode, clear_GraphicMode,
                screen_ne_GraphicMode, Draw_Box_GraphicMode);
  tty_set_reserved(ntty, (unsigned int)sht_b_cur, (unsigned int)sht_win, 0, 0);
  struct TASK* ntask =
      AddTask("Cmdline", 3, 2 * 8, (int)shell_handler, 1 * 8, 1 * 8,
              (unsigned int)page_kmalloc(128 * 1024) + 128 * 1024);
  char* kfifo = (struct FIFO8*)page_kmalloc(sizeof(struct FIFO8));
  char* mfifo = (struct FIFO8*)page_kmalloc(sizeof(struct FIFO8));
  char* kbuf = (char*)page_kmalloc(4096);
  char* mbuf = (char*)page_kmalloc(4096);
  fifo8_init(kfifo, 4096, kbuf);
  fifo8_init(mfifo, 4096, mbuf);
  TaskSetFIFO(ntask, kfifo, mfifo);
  char* memman = (char*)page_kmalloc(4 * 1024);
  int alloc_addr = (int)page_kmalloc(512 * 1024);
  ntask->memman = memman;
  ntask->alloc_addr = alloc_addr;
  ntask->alloc_size = 512 * 1024;
  int fg = tty_set(ntask, ntty);
  //  printk("set vram = %08x\n",ntty->vram);
  ntty->clear(ntty);
  sht_win->task = ntask;
  io_sti();
}
bool now_tty_GraphicMode(struct tty* res) {
  struct SHEET* sht = (struct SHEET*)res->reserved[1];
  if (sht->height == shtctl->top - 1) {
    return true;
  } else {
    return false;
  }
}

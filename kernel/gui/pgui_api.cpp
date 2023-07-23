#include <pbutton.hpp>
#include <psheet.hpp>
#include <textbox.hpp>
#include <window.hpp>

extern struct SHTCTL* ctl;
enum { EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX };

void close_handler(PSheetBase* ps, int x, int y, uint32_t val) {}
void button_click_handler(PButton* pbtn, uint32_t val) {}
struct button_args {
  unsigned int red;
  unsigned int x;
  unsigned int y;
  unsigned int width;
  unsigned int height;
  int sht;
  // int func;
  unsigned int text;
} __attribute__((packed));

struct close_window_args {
  int red;
  int sht;
} __attribute__((packed));
void pgui_api(int edi,
              int esi,
              int ebp,
              int esp,
              int ebx,
              int edx,
              int ecx,
              int eax,
              uint32_t* reg) {
  struct TASK* task = current_task();
  int ds_base = task->ds_base;

  /*强行改写通过PUSHAD保存的值*/
  /* reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP */
  /* reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */
  if (eax == 0x01) {
    Window* wnd = new Window(ctl, (char *)(edi+ds_base), ebx,ecx,edx,esi);
    reg[EAX] = (uint32_t)wnd;
  } else if (eax == 0x02) {
    // EBX，窗口ID
    Window* wnd = (Window*)ebx;
    delete wnd;
  } else if (eax == 0x03) {
    struct button_args* args = (struct button_args*)(ds_base + ebx + 8);
    reg[EDX] = (uint32_t) new PButton(
        (PSheetBase*)args->sht, (char*)(args->text + ds_base), args->x, args->y,
        args->width, args->height, -1, button_click_handler);
  } else if (eax == 0x04) {
    struct close_window_args* _sht =
        (struct close_window_args*)(ds_base + ebx + 8);
    delete (PButton*)_sht->sht;
  } else if (eax == 0x05) {
    if (ebx == 0x09) {
      int base = edx + ds_base + 12;
      PSheetBase* sht = (PSheetBase*)(*(int*)(base + 4));
      int x = *(int*)(base + 8), y = *(int*)(base + 12),
          color = *(int*)(base + 20), xsize = *(int*)(base + 24);
      int c = *(int*)(base + 16);
      SDraw_Char(sht->get_vram(), x, y, c, color, xsize);
      sht->refresh(x, y, x + 8, y + 16);
    } else if (ebx == 0x0a) {
      int base = edx + ds_base + 12;
      PSheetBase* sht = (PSheetBase*)(*(int*)(base + 4));
      char* str = (char*)(ds_base + *(int*)(base + 8));
      int x = *(int*)(base + 12), y = *(int*)(base + 16),
          color = *(int*)(base + 20), xsize = *(int*)(base + 24);
      Sputs(sht->get_vram(), str, x, y, color, xsize);
      sht->refresh(x, y, x + strlen(str) * 8, y + 16);
    } else if (ebx == 0x0b) {
      int base = edx + ds_base + 12;
      PSheetBase* sht = (PSheetBase*)(*(int*)(base + 4));
      int x = *(int*)(base + 8), y = *(int*)(base + 12),
          color = *(int*)(base + 16), xsize = *(int*)(base + 20);
      SDraw_Px(sht->get_vram(), x, y, color, xsize);
      sht->refresh(x, y, x + 1, y + 1);
    } else if (ebx == 0x0c) {
      int base = edx + ds_base + 12;
      PSheetBase* sht = (PSheetBase*)(*(int*)(base + 4));
      int x = *(int*)(base + 8), y = *(int*)(base + 12),
          x1 = *(int*)(base + 16), y1 = *(int*)(base + 20);
      int color = *(int*)(base + 24), xsize = *(int*)(base + 28);
      SDraw_Box(sht->get_vram(), x, y, x1, y1, color, xsize);
      sht->refresh(x, y, x1 + 1, y1 + 1);
    } else if (ebx == 0x0d) {
      int base = edx + ds_base + 12;
      PSheetBase* sht = (PSheetBase*)(*(int*)(base + 4));
      int x = *(int*)(base + 12), y = *(int*)(base + 16),
          color = *(int*)(base + 20), xsize = *(int*)(base + 8);
      unsigned short c = *(unsigned short*)(base + 24);
      PutChineseChar0(sht->get_vram(), xsize, x, y, color, c);
      sht->refresh(x * 16, y * 16, x * 16 + 16, y * 16 + 16);
    } else if (ebx == 0x0e) {
      int base = edx + ds_base + 12;
      PSheetBase* sht = (PSheetBase*)(*(int*)(base + 4));
      int x = *(int*)(base + 12), y = *(int*)(base + 16),
          color = *(int*)(base + 20), xsize = *(int*)(base + 8);
      unsigned char* str = (unsigned char*)(ds_base + *(int*)(base + 24));
      PutChineseStr0(sht->get_vram(), xsize, x, y, color, str);
      sht->refresh(x * 16, y * 16, x * 16 + strlen((char*)str) * 8,
                   y * 16 + 16);
    }
  } else if (eax == 0x06) {
    // MsgBox((char*)(ds_base + ebx), (char*)(ds_base + edx));
  } else if (eax == 0x07) {
    if (running_mode != POWERDESKTOP) {
      reg[EAX] = 0;
    } else {
      reg[EAX] = 1;
    }

  } else if (eax == 0x08) {
    int base = edx + ds_base + 12 - 4;
    int* arg = (int*)(base + 4);
    reg[EAX] = (uint32_t) new PTextBox((PSheetBase*)arg[4], arg[0], arg[1],
                                       arg[2], arg[3]);
  } else if (eax == 0x09) {
    int base = edx + ds_base + 12;
    int* arg = (int*)(base + 4);
    delete (PTextBox*)arg[0];
  } else if (eax == 0x0a) {
    int base = edx + ds_base + 12;
    int* arg = (int*)(base + 4);
    strcpy((char*)(arg[0] + ds_base), ((struct TextBox*)(arg[1]))->text);
  } else if (eax == 0x0b) {
  } else if (eax == 0x0c) {
  } else if (eax == 0x0d) {
  } else if (eax == 0x0e) {
  } else if (eax == 0x0f) {
    PSheetBase* r = (PSheetBase*)ebp;
    int x = ebx;
    int y = ecx;
    int w = edx;
    int h = esi;
    unsigned int* buffer = (unsigned int*)(edi + ds_base);
    for (int i = x; i < x + w; i++) {
      for (int j = y; j < y + h; j++) {
        r->get_vram()[j * r->get_xsize() + i] = buffer[(j - y) * w + (i - x)];
      }
    }
    r->refresh(x, y, x + w, y + h);
  }
}
extern "C" void c_pgui_api(int edi,
                           int esi,
                           int ebp,
                           int esp,
                           int ebx,
                           int edx,
                           int ecx,
                           int eax) {
  uint32_t* reg = (uint32_t*)(&eax + 1); /* eax后面的地址*/
  pgui_api(edi, esi, ebp, esp, ebx, edx, ecx, eax, reg);
}
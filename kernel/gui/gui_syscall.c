#include <dos.h>
#include <gui.h>
TextBox* MakeTextBox(int x, int y, int w, int h, struct SHEET* window_sht);
void DeleteTextBox(TextBox* buf);
extern struct SHTCTL* shtctl;
extern List* list;
extern List* list1;
extern struct SHTCTL* shtctl;
struct Button_args {
  unsigned int red;
  unsigned int x;
  unsigned int y;
  unsigned int width;
  unsigned int height;
  int sht;
  // int func;
  unsigned int text;
} __attribute__((packed));
struct Close_Window_args {
  int red;
  int sht;
} __attribute__((packed));
extern Button* button_cl;
extern struct SHEET* close_tid;
void Button_handle() {
  // printk("app button--TID--%d\n", button_cl->tid);
  // io_cli();
  SendIPCMessageTID(Get_Tid(button_cl->task), -1, &button_cl,
                    sizeof(unsigned int), asynchronous);
  // io_sti();
}
void Window_Close_Handler() {
  SendIPCMessageTID(Get_Tid(close_tid->task), -2, &close_tid,
                    sizeof(unsigned int), asynchronous);
}
void Gui_API(int edi,
             int esi,
             int ebp,
             int esp,
             int ebx,
             int edx,
             int ecx,
             int eax) {
  // Power Desktop API
  // Maskirq(0);
  struct TASK* task = NowTask();
  int cs_base = task->cs_base;
  int ds_base = task->ds_base;
  int alloc_addr = task->alloc_addr;  // malloc地址
  if (eax == 0x01) {
    // Create Window
    // MsgBox("Create Window", "Create Window");
    intreturn((int)MakeWindow(ebx, ecx, edx, esi, (char*)ds_base + edi, shtctl,
                              page_malloc(edx * esi * sizeof(color_t)),
                              Window_Close_Handler),
              ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x02) {
    // EBX，窗口ID
    struct Close_Window_args* _sht =
        (struct Close_Window_args*)(ds_base + ebx + 8);
    struct SHEET* sht = (struct SHEET*)_sht->sht;
    page_free(sht->buf, sht->bxsize * sht->bysize);
    sheet_updown(sht, -1);
    sheet_free(sht);
  } else if (eax == 0x03) {
    struct Button_args* args = (struct Button_args*)(ds_base + ebx + 8);
    unsigned int a = (unsigned int)MakeButton(
        args->x, args->y, args->width, args->height, (struct SHEET*)args->sht,
        (char*)args->text + ds_base, Button_handle);
    intreturn(eax, ebx, ecx, a, esi, edi, ebp);
  } else if (eax == 0x04) {
    struct Close_Window_args* _sht =
        (struct Close_Window_args*)(ds_base + ebx + 8);
    DeleteButton((struct Button*)_sht->sht);
  } else if (eax == 0x05) {
    if (ebx == 0x09) {
      int base = edx + ds_base + 12;
      struct SHEET* sht = (struct SHEET*)(*(int*)(base + 4));
      int x = *(int*)(base + 8), y = *(int*)(base + 12),
          color = *(int*)(base + 20), xsize = *(int*)(base + 24);
      int c = *(int*)(base + 16);
      SDraw_Char(sht->buf, x, y, c, color, xsize);
      sheet_refresh(sht, x * 8, y * 16, x * 8 + 8, y * 16 + 16);
    } else if (ebx == 0x0a) {
      int base = edx + ds_base + 12;
      struct SHEET* sht = (struct SHEET*)(*(int*)(base + 4));
      char* str = ds_base + *(int*)(base + 8);
      int x = *(int*)(base + 12), y = *(int*)(base + 16),
          color = *(int*)(base + 20), xsize = *(int*)(base + 24);
      Sputs(sht->buf, str, x, y, color, xsize);
      sheet_refresh(sht, x, y, x + strlen(str) * 8, y + 16);
    } else if (ebx == 0x0b) {
      int base = edx + ds_base + 12;
      struct SHEET* sht = (struct SHEET*)(*(int*)(base + 4));
      int x = *(int*)(base + 8), y = *(int*)(base + 12),
          color = *(int*)(base + 16), xsize = *(int*)(base + 20);
      SDraw_Px(sht->buf, x, y, color, xsize);
      sheet_refresh(sht, x, y, x + 1, y + 1);
    } else if (ebx == 0x0c) {
      int base = edx + ds_base + 12;
      struct SHEET* sht = (struct SHEET*)(*(int*)(base + 4));
      int x = *(int*)(base + 8), y = *(int*)(base + 12),
          x1 = *(int*)(base + 16), y1 = *(int*)(base + 20);
      int color = *(int*)(base + 24), xsize = *(int*)(base + 28);
      SDraw_Box(sht->buf, x, y, x1, y1, color, xsize);
      sheet_refresh(sht, x, y, x1 + 1, y1 + 1);
    } else if (ebx == 0x0d) {
      int base = edx + ds_base + 12;
      struct SHEET* sht = (struct SHEET*)(*(int*)(base + 4));
      int x = *(int*)(base + 12), y = *(int*)(base + 16),
          color = *(int*)(base + 20), xsize = *(int*)(base + 8);
      unsigned short c = *(unsigned short*)(base + 24);
      PutChineseChar0(sht->buf, xsize, x, y, color, c);
      sheet_refresh(sht, x * 16, y * 16, x * 16 + 16, y * 16 + 16);
    } else if (ebx == 0x0e) {
      int base = edx + ds_base + 12;
      struct SHEET* sht = (struct SHEET*)(*(int*)(base + 4));
      int x = *(int*)(base + 12), y = *(int*)(base + 16),
          color = *(int*)(base + 20), xsize = *(int*)(base + 8);
      unsigned char* str = ds_base + *(int*)(base + 24);
      PutChineseStr0(sht->buf, xsize, x, y, color, str);
      sheet_refresh(sht, x * 16, y * 16, x * 16 + strlen(str) * 8, y * 16 + 16);
    }
  } else if (eax == 0x06) {
    MsgBox((char*)(ds_base + ebx), (char*)(ds_base + edx));
  } else if (eax == 0x07) {
    if (running_mode != POWERDESKTOP) {
      intreturn(0, ebx, ecx, edx, esi, edi, ebp);
    } else {
      intreturn(1, ebx, ecx, edx, esi, edi, ebp);
    }

  } else if (eax == 0x08) {
    int base = edx + ds_base + 12 - 4;
    int* arg = (int*)(base + 4);

    intreturn(MakeTextBox(arg[0], arg[1], arg[2], arg[3], arg[4]), ebx, ecx,
              edx, esi, edi, ebp);
  } else if (eax == 0x09) {
    int base = edx + ds_base + 12;
    int* arg = (int*)(base + 4);
    DeleteTextBox((struct TextBox*)arg[0]);
    intreturn(eax, ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x0a) {
    int base = edx + ds_base + 12;
    int* arg = (int*)(base + 4);
    strcpy((char*)(arg[0] + ds_base), ((struct TextBox*)(arg[1]))->text);
    intreturn(eax, ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x0b) {
    int base = edx + ds_base + 12 - 4;
    int* arg = (int*)(base + 4);
    // printf("wid:%d,x%d,y:%d,w:%d,h:%d", arg[0], arg[1], arg[2], arg[3], arg[4]);
    intreturn(MakeListBox(arg[0], arg[1], arg[2], arg[3], arg[4]), ebx, ecx,
              edx, esi, edi, ebp);
  } else if (eax == 0x0c) {
    int base = edx + ds_base + 12;
    int* arg = (int*)(base + 4);
    AddItem((struct ListBox*)(arg[0]), (char*)(arg[1] + ds_base), NULL);
  } else if(eax == 0x0d) {
    int base = edx + ds_base + 12;
    int* arg = (int*)(base + 4);
    RemoveItem((struct ListBox*)(arg[0]), (char*)(arg[1] + ds_base));
  } else if(eax==0x0e) {
    int base = edx + ds_base + 12;
    int* arg = (int*)(base + 4);
    DeleteListBox((struct ListBox*)(arg[0]));
  }

  // ClearMaskIrq(0);
}
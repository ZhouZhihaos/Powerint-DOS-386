#include <dos.h>
#include <gui.h>
extern Button* button_cl;
void Button1_Click();
void Button2_Click();
List* listBox_Node;
static void NormalHandler() {
  extern Button* button_cl;
  for (int i = 1; FindForCount(i, listBox_Node) != NULL; i++) {
    listBox_t* listBox = (listBox_t*)FindForCount(i, listBox_Node)->val;
    for (int i = listBox->now_min; i != listBox->now_max; i++) {
      if (listBox->btns[i] == button_cl) {
        MsgBox(listBox->item[i], "");
      }
    }
  }
}
static void refresh_all(listBox_t* listBox) {
  sheet_refresh(listBox->sheet, listBox->x, listBox->y,
                listBox->x + listBox->width + 19, listBox->y + listBox->height);
}
static void draw_listBox(listBox_t* listBox) {
  // printk("%08x\n", listBox->sheet->buf);
  SDraw_Box(listBox->sheet->buf, listBox->x, listBox->y,
            listBox->x + listBox->width + 19, listBox->y + listBox->height,
            COL_FFFFFF, listBox->sheet->bxsize);
  for (int i = 0; i != listBox->height / 2; i++) {
    for (int j = 0; j != 19 / 2; j++) {
      SDraw_Px(listBox->sheet->buf, listBox->x + listBox->width + 3 + j * 2,
               listBox->y + i * 2, COL_C6C6C6, listBox->sheet->bxsize);
      SDraw_Px(listBox->sheet->buf, listBox->x + listBox->width + 3 + j * 2 + 1,
               listBox->y + i * 2 + 1, COL_C6C6C6, listBox->sheet->bxsize);
    }
  }
  refresh_all(listBox);
}
static void Draw_All(listBox_t* listBox) {
  SDraw_Box(listBox->sheet->buf, listBox->x, listBox->y,
            listBox->x + listBox->width, listBox->y + listBox->height,
            COL_FFFFFF, listBox->sheet->bxsize);
  for (int i = listBox->now_min; i < listBox->now_max; i++) {
    listBox->btns[i]->hide = false;
    listBox->btns[i]->x = listBox->x;
    listBox->btns[i]->y = listBox->y + (i - listBox->now_min) * 16;
    Button_Draw_NoFrame(listBox->btns[i]);
  }
  refresh_all(listBox);
}
void* realloc(void* ptr, uint32_t size) {
  void* new = malloc(size);
  if (ptr) {
    memcpy(new, ptr, *(int*)((int)ptr - 4));
    free(ptr);
  }
  return new;
}
void init_listBox() {
  listBox_Node = NewList();
}
listBox_t* MakeListBox(struct SHEET* sheet, int x, int y, int w, int h) {
  listBox_t* listBox = (listBox_t*)malloc(sizeof(listBox_t));
  listBox->x = x;
  listBox->y = y;
  listBox->width = w;
  listBox->height = h;
  listBox->sheet = sheet;
  listBox->item_num = 0;
  listBox->item = (char**)malloc(sizeof(char*) * h / 16);
  listBox->now_max = 0;
  listBox->now_min = 0;
  listBox->view_max = h / 16;
  listBox->itm_arr_sz = h / 16;
  listBox->btns = (Button**)malloc(sizeof(Button*) * h / 16);

  draw_listBox(listBox);
  make_textbox(sheet, x, y, w, h, COL_FFFFFF);
  SDraw_Box(sheet->buf, x, y, x + 8 - 1, y + 16 - 1, COL_FFFFFF, sheet->bxsize);
  listBox->button1 =
      MakeButton(x + w - 16 + 16, y - 28, 16, 16, sheet, "", Button1_Click);
  listBox->button2 = MakeButton(x + w - 16 + 16, y + h - 28 - 16, 16, 16, sheet,
                                "", Button2_Click);
  for (int i = 0; i != 5; i++) {  // 画↑ ↓
    for (int j = 0; j != (2 * i + 1); j++) {
      SDraw_Px(sheet->buf, x + w + 10 + i - j, y + h - 6 - i, COL_000000,
               sheet->bxsize);
      SDraw_Px(sheet->buf, x + w + 10 + i - j, y + 6 + i, COL_000000,
               sheet->bxsize);
    }
  }
  sheet_refresh(sheet, 0, 0, sheet->bxsize, sheet->bysize);
  AddVal(listBox, listBox_Node);
  listBox->cont = GetLastCount(listBox_Node);
  return listBox;
}
void DeleteListBox(listBox_t* listBox) {
  DeleteButton(listBox->button1);
  DeleteButton(listBox->button2);
  for (int i = 0; i < listBox->item_num; i++) {
    DeleteButton_NoFrame(listBox->btns[i]);
    free(listBox->item[i]);
  }
  free(listBox->item);
  DeleteVal(listBox->cont, listBox_Node);
  free(listBox);
}
void AddItem(listBox_t* listBox, char* item, void(*OnClick())) {
  // printk("BUF=%08x\n", listBox->sheet->buf);
  if (!OnClick) {
    OnClick = NormalHandler;
  }
  if (listBox->view_max > listBox->item_num) {
    //   printk("listBox->view_max <= listBox->item_num\n");
    listBox->now_max++;
  } else {
    //   printk("VIEW MAX=%d\n", listBox->view_max);
    //   printk("ITEM NUM=%d\n", listBox->item_num);
  }
  if (listBox->item_num >= listBox->itm_arr_sz) {
    //    printk("ITEM ARR SZ=%d\n", listBox->itm_arr_sz);
    listBox->itm_arr_sz *= 2;
    listBox->item =
        (char**)realloc(listBox->item, sizeof(char*) * listBox->itm_arr_sz);
    listBox->btns =
        (Button**)realloc(listBox->btns, sizeof(Button*) * listBox->itm_arr_sz);
    listBox->item[listBox->item_num] = malloc(sizeof(char) * strlen(item));
    strcpy(listBox->item[listBox->item_num], item);
    listBox->btns[listBox->item_num] = MakeButton_NoFrame(
        0, 0, listBox->width, 16, listBox->sheet,
        listBox->item[listBox->item_num], OnClick, COL_FFFFFF, true);
    listBox->item_num++;
    for (int i = 0; i < listBox->itm_arr_sz; i++) {
      //      printk("Re:%s\n", listBox->item[i]);
    }

    // Next(listBox);
    return;
  }
  listBox->item[listBox->item_num] = malloc(sizeof(char) * strlen(item));
  strcpy(listBox->item[listBox->item_num], item);
  listBox->btns[listBox->item_num] = MakeButton_NoFrame(
      0, 0, listBox->width, 16, listBox->sheet,
      listBox->item[listBox->item_num], OnClick, COL_FFFFFF, true);
  listBox->item_num++;
  Draw_All(listBox);
}
void RemoveItem(listBox_t* listBox, int index) {
  if (index >= listBox->item_num) {
    return;
  }
  DeleteButton_NoFrame(listBox->btns[index]);
  free(listBox->item[index]);
  for (int i = index; i < listBox->item_num - 1; i++) {
    listBox->item[i] = listBox->item[i + 1];
    listBox->btns[i] = listBox->btns[i + 1];
  }
  listBox->item_num--;
  if (listBox->now_max > listBox->item_num) {
    listBox->now_max = listBox->item_num;
  }
  Draw_All(listBox);
}
void Next(listBox_t* listBox) {
  if (listBox->now_max == listBox->now_min) {
    return;
  }
  if (listBox->now_max == listBox->item_num) {
    //   printk("now_max == item_num\n");
    return;
  }
  listBox->now_min++;
  listBox->now_max++;
  listBox->btns[listBox->now_min]->hide = true;
  //  printk("?now_min:%d\n", listBox->now_min);
  // printk("?now_max:%d\n", listBox->now_max);
  Draw_All(listBox);
}
void Prev(listBox_t* listBox) {
  if (listBox->now_max == listBox->now_min) {
    return;
  }
  if (listBox->now_min == 0) {
    return;
  }
  listBox->now_min--;
  listBox->now_max--;
  listBox->btns[listBox->now_max]->hide = true;
  Draw_All(listBox);
}
void Button1_Click() {
  Button* button = button_cl;
  for (int i = 1; FindForCount(i, listBox_Node) != NULL; i++) {
    listBox_t* listBox = (listBox_t*)FindForCount(i, listBox_Node)->val;
    if (listBox->button1 == button) {
      for (int i = 0; i != 5; i++) {
        for (int j = 0; j != (2 * i + 1); j++) {
          SDraw_Px(button->buf->buf, listBox->x + listBox->width + 10 + i - j,
                   listBox->y + 6 + i, COL_000000, button->buf->bxsize);
        }
      }
      sheet_refresh(button->buf, listBox->x + listBox->width, listBox->y,
                    listBox->x + listBox->width + 16, listBox->y + 16);
      Prev(listBox);
      break;
    }
  }
}
void Button2_Click() {
  Button* button = button_cl;
  for (int i = 1; FindForCount(i, listBox_Node) != NULL; i++) {
    listBox_t* listBox = (listBox_t*)FindForCount(i, listBox_Node)->val;
    if (listBox->button2 == button) {
      for (int i = 0; i != 5; i++) {
        for (int j = 0; j != (2 * i + 1); j++) {
          SDraw_Px(button->buf->buf, listBox->x + listBox->width + 10 + i - j,
                   listBox->y + listBox->height - 6 - i, COL_000000,
                   button->buf->bxsize);
        }
      }
      sheet_refresh(button->buf, listBox->x + listBox->width,
                    listBox->y + listBox->height - 16,
                    listBox->x + listBox->width + 16,
                    listBox->y + listBox->height);
      Next(listBox);
      break;
    }
  }
}
void ListBox_Roll_Up() {
  extern struct SHTCTL* shtctl;
  struct SHEET* top = shtctl->sheets[shtctl->top - 1];  // 焦点窗口
  for (int i = 1; FindForCount(i, listBox_Node) != NULL; i++) {
    listBox_t* listBox = (listBox_t*)FindForCount(i, listBox_Node)->val;
    if (listBox->sheet == top) {
      Button* button = listBox->button2;
      for (int i = 0; i != 5; i++) {
        for (int j = 0; j != (2 * i + 1); j++) {
          SDraw_Px(button->buf->buf, listBox->x + listBox->width + 10 + i - j,
                   listBox->y + 6 + i, COL_000000, button->buf->bxsize);
        }
      }
      sheet_refresh(button->buf, listBox->x + listBox->width, listBox->y,
                    listBox->x + listBox->width + 16, listBox->y + 16);
      Prev(listBox);
    }
  }
}
void ListBox_Roll_Down() {
  extern struct SHTCTL* shtctl;
  struct SHEET* top = shtctl->sheets[shtctl->top - 1];  // 焦点窗口
  for (int i = 1; FindForCount(i, listBox_Node) != NULL; i++) {
    listBox_t* listBox = (listBox_t*)FindForCount(i, listBox_Node)->val;
    if (listBox->sheet == top) {
      Button* button = listBox->button2;
      for (int i = 0; i != 5; i++) {
        for (int j = 0; j != (2 * i + 1); j++) {
          SDraw_Px(button->buf->buf, listBox->x + listBox->width + 10 + i - j,
                   listBox->y + listBox->height - 6 - i, COL_000000,
                   button->buf->bxsize);
        }
      }
      sheet_refresh(button->buf, listBox->x + listBox->width,
                    listBox->y + listBox->height - 16,
                    listBox->x + listBox->width + 16,
                    listBox->y + listBox->height);
      Next(listBox);
    }
  }
}
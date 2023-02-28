/**
 * @file loadbox.c
 * @author min0911_
 * @brief Powerint DOS Gui 进度条控件的实现
 * @version 0.1
 * @date 2022-07-18
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <dos.h>
#include <gui.h>
List *LoadBox_List = NULL; // 存放所有的LoadBox控件的链表
void init_LoadBox() {
  LoadBox_List = NewList(); // 创建一个链表用于存放所有的LoadBox控件
}
static void Draw_LoadBox(loadBox *lb) {
  // 画一个矩形框
  struct SHEET *sht = lb->sheet;
  int x0 = lb->position.x, y0 = lb->position.y;
  int x1 = x0 + lb->position.w, y1 = y0 + lb->position.h;
  boxfill(sht->buf, sht->bxsize, COL_C6C6C6, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
  boxfill(sht->buf, sht->bxsize, COL_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
  boxfill(sht->buf, sht->bxsize, COL_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
  boxfill(sht->buf, sht->bxsize, COL_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
  boxfill(sht->buf, sht->bxsize, COL_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
  boxfill(sht->buf, sht->bxsize, COL_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
  boxfill(sht->buf, sht->bxsize, COL_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
  boxfill(sht->buf, sht->bxsize, COL_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
  sheet_refresh(sht, x0 - 3, y0 - 3, x1 + 3, y1 + 3);

  // 画进度条
  // 1.计算进度条的长度
  int len = (lb->position.w - 1) * lb->load_progress / 100;

  // 2.画进度条
  if (len != 0) {
    boxfill(lb->sheet->buf, lb->sheet->bxsize, argb(0, 10, 36, 106),
            lb->position.x + 1, lb->position.y + 1, lb->position.x + len,
            lb->position.y + lb->position.h - 1);
  }
  sheet_refresh(lb->sheet, lb->position.x, lb->position.y,
                lb->position.x + lb->position.w + 1,
                lb->position.y + lb->position.h + 1);
}
/**
 * @brief 创建一个新的LoadBox控件
 *
 * @param sheet      窗口图层
 * @param x          控件的x坐标
 * @param y          控件的y坐标
 * @param w          控件的宽度
 * @param h          控件的高度
 * @return loadBox   返回一个LoadBox控件的指针
 */
loadBox *MakeLoadBox(struct SHEET *sheet, int x, int y, int w, int h) {
  loadBox *lb = (loadBox *)malloc(sizeof(loadBox)); // 创建一个新的LoadBox控件
  /* 初始化LoadBox控件在窗口上的位置 */
  lb->position.h = h;
  lb->position.w = w;
  lb->position.x = x;
  lb->position.y = y;
  lb->sheet = sheet;     // 将窗口图层赋值给LoadBox控件的图层
  lb->load_progress = 0; // 初始化进度条的进度为0
  AddVal((int)lb, LoadBox_List); // 将LoadBox控件加入到链表中
  lb->count =
      GetLastCount(LoadBox_List); // 获取链表中的最后一个LoadBox控件的编号
  Draw_LoadBox(lb);               // 画出LoadBox控件
  return lb;
}
void set_LoadBox_Progress(loadBox *lb, int progress) {
  if (progress > 100)
    progress = 100;
  if (progress < 0)
    progress = 0;
  lb->load_progress = progress;
  Draw_LoadBox(lb); // 画进度条
}
int get_LoadBox_Progress(loadBox *lb) { return lb->load_progress; }
void DeleteLoadBox(loadBox *lb) {
  DeleteVal(lb->count, LoadBox_List); // 从链表中删除LoadBox控件
  free(lb);                           // 释放LoadBox控件的内存
}

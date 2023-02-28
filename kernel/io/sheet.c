/**
 *
 * @file sheet.c
 * @author kawai (30天自制操作系统)
 * @note 改进(char---vram_t) （zhouzhihao & min0911_）
 */
#include <io.h>
struct int4 {
  int a, b, c, d;
};
typedef struct int4 int4;
#define SHEET_USE 1

struct SHTCTL* shtctl_init(vram_t* vram, int xsize, int ysize) {
  struct SHTCTL* ctl;
  int i;
  ctl = (struct SHTCTL*)page_malloc(sizeof(struct SHTCTL));
  if (ctl == 0) {
    goto err;
  }
  ctl->map = (unsigned char*)page_malloc(xsize * ysize);
  if (ctl->map == 0) {
    page_free((int)ctl, sizeof(struct SHTCTL));
    goto err;
  }
  ctl->vram = vram;
  ctl->xsize = xsize;
  ctl->ysize = ysize;
  ctl->top = -1; /* 没有一张SHEET */
  for (i = 0; i < MAX_SHEETS; i++) {
    ctl->sheets0[i].flags = 0; /* 标记为未使用 */
    ctl->sheets0[i].ctl = ctl; /* 记录所属*/
    // printk("ctl->sheets0[%d].ctl = %d\n", i, ctl->sheets0[i].ctl->top);
  }
err:
  return ctl;
}
void ctl_free(struct SHTCTL* ctl) {
  if (ctl != 0) {
    if (ctl->map != 0) {
      page_free((int)ctl->map, ctl->xsize * ctl->ysize);
    }
    page_free((int)ctl, sizeof(struct SHTCTL));
  }
}
struct SHEET* sheet_alloc(struct SHTCTL* ctl) {
  struct SHEET* sht;
  int i;
  for (i = 0; i < MAX_SHEETS; i++) {
    if (ctl->sheets0[i].flags == 0) {
      sht = &ctl->sheets0[i];
      sht->flags = SHEET_USE; /* 标记为正在使用*/
      sht->height = -1;       /* 隐藏 */
      // printk("sheet_alloc: sht->ctl->top = %d\n", sht->ctl->top);
      return sht;
    }
  }
  return 0; /* 所有的SHEET都处于正在使用状态*/
}

void sheet_setbuf(struct SHEET* sht,
                  vram_t* buf,
                  int xsize,
                  int ysize,
                  int col_inv) {
  sht->buf = buf;
  sht->bxsize = xsize;
  sht->bysize = ysize;
  sht->col_inv = col_inv;
  return;
}

void sheet_updown(struct SHEET* sht, int height) {
  // printk("%d\n", height);
  struct SHTCTL* ctl = sht->ctl;
  // printk("T = %d\n", ctl->top);
  int h, old = sht->height; /* 存储设置前的高度信息 */
  if (height > ctl->top + 1) {
    height = ctl->top + 1;
  }
  if (height < -1) {
    // printk("?????? %d\n", height);
    height = -1;
  }
  sht->height = height; /* 设定高度 */

  /* 下面主要是进行sheets[]的重新排列 */
  if (old > height) { /* 比以前低 */
    if (height >= 0) {
      /* 把中间的往上提 */
      for (h = old; h > height; h--) {
        ctl->sheets[h] = ctl->sheets[h - 1];
        ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
      sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                       sht->vy0 + sht->bysize, height + 1);
      sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                       sht->vy0 + sht->bysize, height + 1, old);
    } else { /* 隐藏 */
      if (ctl->top > old) {
        /* 把上面的降下来 */
        for (h = old; h < ctl->top; h++) {
          ctl->sheets[h] = ctl->sheets[h + 1];
          ctl->sheets[h]->height = h;
        }
      }
      ctl->top--; /* 由于显示中的图层减少了一个，所以最上面的图层高度下降 */
      sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                       sht->vy0 + sht->bysize, 0);
      sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                       sht->vy0 + sht->bysize, 0, old - 1);
    }
  } else if (old < height) { /* 比以前高 */
    if (old >= 0) {
      /* 把中间的拉下去 */
      for (h = old; h < height; h++) {
        ctl->sheets[h] = ctl->sheets[h + 1];
        ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
    } else { /* 由隐藏状态转为显示状态 */
             /* 将已在上面的提上来 */
      for (h = ctl->top; h >= height; h--) {
        ctl->sheets[h + 1] = ctl->sheets[h];
        ctl->sheets[h + 1]->height = h + 1;
      }
      ctl->sheets[height] = sht;
      ctl->top++; /* 由于已显示的图层增加了1个，所以最上面的图层高度增加 */
    }
    sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                     sht->vy0 + sht->bysize, height);
    sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                     sht->vy0 + sht->bysize, height,
                     height); /* 按新图层信息重新绘制画面 */
  }
  return;
}

void sheet_refresh(struct SHEET* sht, int bx0, int by0, int bx1, int by1) {
  if (sht->height >= 0) { /* 如果正在显示，则按新图层的信息刷新画面*/
    sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1,
                     sht->vy0 + by1, sht->height, sht->height);
  } else {
    //("SHEET: refresh error %d\n", sht->height);
  }
  return;
}

void sheet_refreshsub(struct SHTCTL* ctl,
                      int vx0,
                      int vy0,
                      int vx1,
                      int vy1,
                      int h0,
                      int h1) {
  int h, bx, by, vx, vy, bx0, by0, bx1, by1, bx2, sid4, i, i1, *p;
  int4 *q, *r;
  vram_t *buf, *vram = ctl->vram;
  unsigned char *map = ctl->map;
  unsigned char sid;
  struct SHEET* sht;

  /* 如果refresh的范围超出了画面则修正 */
  if (vx0 < 0) {
    vx0 = 0;
  }
  if (vy0 < 0) {
    vy0 = 0;
  }
  if (vx1 > ctl->xsize) {
    vx1 = ctl->xsize;
  }
  if (vy1 > ctl->ysize) {
    vy1 = ctl->ysize;
  }
  for (h = h0; h <= h1; h++) {
    sht = ctl->sheets[h];
    buf = sht->buf;
    sid = sht - ctl->sheets0;

    /* 使用vx0～vy1，对bx0～by1进行倒推 */
    bx0 = vx0 - sht->vx0;
    by0 = vy0 - sht->vy0;
    bx1 = vx1 - sht->vx0;
    by1 = vy1 - sht->vy0;
    if (bx0 < 0) {
      bx0 = 0;
    } /* 处理刷新范围在图层外侧 */
    if (by0 < 0) {
      by0 = 0;
    }
    if (bx1 > sht->bxsize) {
      bx1 = sht->bxsize;
    } /* 应对不同的重叠方式 */
    if (by1 > sht->bysize) {
      by1 = sht->bysize;
    }
    if ((sht->vx0 & 3) == 0) {
      /* 4字节型*/
      i = (bx0 + 3) / 4; /* bx0除以4（小数进位）*/
      i1 = bx1 / 4;      /* bx1除以4（小数舍去）*/
      i1 = i1 - i;
      sid4 = sid | sid << 8 | sid << 16 | sid << 24;
      for (by = by0; by < by1; by++) {
        vy = sht->vy0 + by;
        for (bx = bx0; bx < bx1 && (bx & 3) != 0; bx++) {
          /*前面被4除多余的部分逐个字节写入*/
          vx = sht->vx0 + bx;
          if (map[vy * ctl->xsize + vx] == sid) {
            vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
          }
        }
        vx = sht->vx0 + bx;
        p = (int*)&map[vy * ctl->xsize + vx];
        q = (int4*)&vram[vy * ctl->xsize + vx];
        r = (int4*)&buf[by * sht->bxsize + bx];
        for (i = 0; i < i1; i++) {
          /* 4的倍数部分*/
          if (p[i] == sid4) {
            q[i] = r[i]; /*估计大多数会是这种情况，因此速度会变快*/
          } else {
            bx2 = bx + i * 4;
            vx = sht->vx0 + bx2;
            if (map[vy * ctl->xsize + vx + 0] == sid) {
              vram[vy * ctl->xsize + vx + 0] = buf[by * sht->bxsize + bx2 + 0];
            }
            if (map[vy * ctl->xsize + vx + 1] == sid) {
              vram[vy * ctl->xsize + vx + 1] = buf[by * sht->bxsize + bx2 + 1];
            }
            if (map[vy * ctl->xsize + vx + 2] == sid) {
              vram[vy * ctl->xsize + vx + 2] = buf[by * sht->bxsize + bx2 + 2];
            }
            if (map[vy * ctl->xsize + vx + 3] == sid) {
              vram[vy * ctl->xsize + vx + 3] = buf[by * sht->bxsize + bx2 + 3];
            }
          }
        }
        for (bx += i1 * 4; bx < bx1; bx++) {
          /*后面被4除多余的部分逐个字节写入*/
          vx = sht->vx0 + bx;
          if (map[vy * ctl->xsize + vx] == sid) {
            vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
          }
        }
      }
    } else {
      /* 1字节型*/
      for (by = by0; by < by1; by++) {
        vy = sht->vy0 + by;
        for (bx = bx0; bx < bx1; bx++) {
          vx = sht->vx0 + bx;
          if (map[vy * ctl->xsize + vx] == sid) {
            vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
          }
        }
      }
    }
  }
}

void sheet_refreshmap(struct SHTCTL* ctl,
                      int vx0,
                      int vy0,
                      int vx1,
                      int vy1,
                      int h0) {
  int h, bx, by, vx, vy, bx0, by0, bx1, by1,*p,sid4,bx2,by2;
  vram_t* buf;
  unsigned char sid, *map = ctl->map;
  struct SHEET* sht;

	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for (h = h0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		sid = sht - ctl->sheets0; /* 将进行了减法计算的地址作为图层号码使用 */
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		if (sht->col_inv == -1) {
			if ((sht->vx0 & 3) == 0 && (bx0 & 3) == 0 && (bx1 & 3) == 0) {
				/*无透明色图层专用的高速版（4字节型）*/
				bx1 = (bx1 - bx0) / 4; /* MOV次数*/
				sid4 = sid | sid << 8 | sid << 16 | sid << 24;
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					vx = sht->vx0 + bx0;
					p = (int *) &map[vy * ctl->xsize + vx];
					for (bx = 0; bx < bx1; bx++) {
						p[bx] = sid4;
					}
				}
			} else {
				/*无透明色图层专用的高速版（1字节型）*/
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1; bx++) {
						vx = sht->vx0 + bx;
						map[vy * ctl->xsize + vx] = sid;
					}
				}
			}
		} else {
			/*有透明色图层用的普通版*/
			for (by = by0; by < by1; by++) {
				vy = sht->vy0 + by;
				for (bx = bx0; bx < bx1; bx++) {
					vx = sht->vx0 + bx;
					if (buf[by * sht->bxsize + bx] != sht->col_inv) {
						map[vy * ctl->xsize + vx] = sid;
					}
				}
			}
		}
	}
	return;
}

void sheet_slide(struct SHEET* sht, int vx0, int vy0) {
  struct SHTCTL* ctl = sht->ctl;
  int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
  sht->vx0 = vx0;
  sht->vy0 = vy0;
  if (sht->height >= 0) { /* 如果正在显示，则按新图层的信息刷新画面 */
    sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize,
                     old_vy0 + sht->bysize, 0);
    sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize,
                     sht->height);
    sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize,
                     old_vy0 + sht->bysize, 0, sht->height - 1);
    sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize,
                     sht->height, sht->height);
  }
  return;
}
// void sheet_slide_ctl(struct SHTCTL* ctl,struct SHEET* sht, int old_vx0,int
// old_vy0,int vx0, int vy0) {
//   sht->vx0 = vx0;
//   sht->vy0 = vy0;
//   if (sht->height >= 0) { /* 如果正在显示，则按新图层的信息刷新画面 */
//     sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize,
//                      old_vy0 + sht->bysize, 0);
//     sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize,
//                      sht->height);
//     sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize,
//                      old_vy0 + sht->bysize, 0, sht->height - 1);
//     sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize,
//                      sht->height, sht->height);
//   }
//   return;
// }
void sheet_free(struct SHEET* sht) {
  if (sht->height >= 0) {
    sheet_updown(sht, -1); /* 如果处于显示状态，则先设定为隐藏 */
  }
  sht->flags = 0; /* "未使用"标志 */
  return;
}

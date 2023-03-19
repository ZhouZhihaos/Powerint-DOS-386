#include <dos.h>
static unsigned char *stack;
void *aligned_malloc(size_t size, int alignment) {
  // 分配足够的内存, 这里的算法很经典, 早期的STL中使用的就是这个算法

  // 首先是维护FreeBlock指针占用的内存大小
  const int pointerSize = sizeof(void *);

  // alignment - 1 + pointerSize这个是FreeBlock内存对齐需要的内存大小
  // 前面的例子sizeof(T) = 20, __alignof(T) = 16,
  // g_MaxNumberOfObjectsInPool = 1000
  // 那么调用本函数就是alignedMalloc(1000 * 20, 16)
  // 那么alignment - 1 + pointSize = 19
  const int requestedSize = size + alignment - 1 + pointerSize;

  // 分配的实际大小就是20000 + 19 = 20019
  void *raw = malloc(requestedSize);

  // 这里实Pool真正为对象实例分配的内存地址
  uint32_t *start = (uint32_t)raw + pointerSize;
  // 向上舍入操作
  // 解释一下, __ALIGN - 1指明的是实际内存对齐的粒度
  // 例如__ALIGN = 8时, 我们只需要7就可以实际表示8个数(0~7)
  // 那么~(__ALIGN - 1)就是进行舍入的粒度
  // 我们将(bytes) + __ALIGN-1)就是先进行进位, 然后截断
  // 这就保证了我是向上舍入的
  // 例如byte = 100, __ALIGN = 8的情况
  // ~(__ALIGN - 1) = (1 000)B
  // ((bytes) + __ALIGN-1) = (1 101 011)B
  // (((bytes) + __ALIGN-1) & ~(__ALIGN - 1)) = (1 101 000 )B = (104)D
  // 104 / 8 = 13, 这就实现了向上舍入
  // 对于byte刚好满足内存对齐的情况下, 结果保持byte大小不变
  // 记得《Hacker's Delight》上面有相关的计算
  // 这个表达式与下面给出的等价
  // ((((bytes) + _ALIGN - 1) * _ALIGN) / _ALIGN)
  // 但是SGI STL使用的方法效率非常高
  void *aligned =
      (void *)(((uint32_t)start + alignment - 1) & ~(alignment - 1));

  // 这里维护一个指向malloc()真正分配的内存
  *(void **)((uint32_t *)aligned - pointerSize) = raw;

  // 返回实例对象真正的地址
  return aligned;
}

void aligned_free(void *aligned_ptr) {
  if (aligned_ptr) {
    free(((uint32_t *)aligned_ptr)[-1]);
  }
}
void inthandler36(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx,
                  int eax) {
  // PowerintDOS API
  struct TASK *task = NowTask();
  int cs_base = task->cs_base;
  int ds_base = task->ds_base;
  int alloc_addr = task->alloc_addr; // malloc地址
  int alloc_size = task->alloc_size;
  char *memman = task->memman;
  (void)(cs_base);
  if (eax == 0x01) {
    intreturn(eax, ebx, ecx, 0x302e3761, esi, edi, ebp); // 0.7a
  } else if (eax == 0x02) {
    printchar((edx & 0x000000ff));
  } else if (eax == 0x03) {
    if (running_mode == POWERINTDOS) {
      if (ebx == 0x01) {
        SwitchToText8025();
      } else if (ebx == 0x02) {
        SwitchTo320X200X256();
      } else if (ebx == 0x03) {
        Draw_Char(ecx, edx, esi, edi);
      } else if (ebx == 0x04) {
        PrintChineseChar(ecx, edx, edi, esi);
      } else if (ebx == 0x05) {
        Draw_Box(ecx, edx, esi, edi, ebp);
      } else if (ebx == 0x06) {
        Draw_Px(ecx, edx, esi);
      } else if (ebx == 0x07) {
        Draw_Str(ecx, edx, (char *)esi + ds_base, edi);
      } else if (ebx == 0x08) {
        PrintChineseStr(ecx, edx, edi, (unsigned char *)esi + ds_base);
      }
    }
  } else if (eax == 0x04) {
    gotoxy(edx, ecx);
  } else if (eax == 0x05) {
    print((char *)edx + ds_base);
  } else if (eax == 0x06) {
    sleep(edx);
  } else if (eax == 0x08) {
    int size = (ecx - 1) / 128 + 1;
    for (int i = 0, j = 0; i != alloc_size / 128; i++) {
      if (j == size) {
        for (int k = j; k != 0; k--) {
          memman[i - k] = 0xff;
        }
        // for (int l = 0; l != 128; l++) {
        //   unsigned char c = (unsigned)memman[l];
        //   printk("%02x ", c);
        // }
        intreturn(eax, ebx, ecx, (i - j) * 128 + (alloc_addr - ds_base), esi,
                  edi, ebp);
        return;
      }
      if (memman[i] == 0x00) {
        j++;
      } else {
        j = 0;
      }
    }
    printk("malloc error\n");
    intreturn(eax, ebx, ecx, 0, esi, edi, ebp);
  } else if (eax == 0x09) {
    ecx = ((ecx - 1) + 128) / 128;
    int ad = (edx & 0xfffffff0) - (alloc_addr - ds_base);
    edx = ad / 128;
    if (edx > alloc_size / 128)
      return;
    for (int i = edx; i < ecx + edx; i++) {
      memman[i] = 0x00;
    }
    // printk("DS_BASE:%08x ALLOC_ADDR:%08x FREE:%08x ~ %08x\n", ds_base,
    //        alloc_addr, ad + (alloc_addr - ds_base),
    //        ad + (alloc_addr - ds_base) + ecx * 128);
    // for (int j = 0; j != 128; j++) {
    //  unsigned char c = (unsigned)memman[j];
    //  printk("%02x ", c);
    // }
    // printk("\n");
    clean((char *)(ad + alloc_addr), ecx * 128);
  } else if (eax == 0x0c) {
    Text_Draw_Box(ecx, ebx, esi, edx, (unsigned char)edi);
  } else if (eax == 0x0e) {
    intreturn(eax, ebx, get_y(), get_x(), esi, edi, ebp);
  } else if (eax == 0x0d) {
    beep(ebx, ecx, edx);
  } else if (eax == 0x0f) {
    if (running_mode == POWERDESKTOP) {
      extern int gmx, gmy;
      struct SHEET *sht_win = (struct SHEET *)task->TTY->reserved[1];
      for (;;) {
        if ((mdec.btn & 0x01) != 0) {
          intreturn(eax, ebx, (gmx - sht_win->vx0 - 5) / 8,
                    (gmy - sht_win->vy0 - 24) / 16, 1, edi, ebp);
          return;
        } else if ((mdec.btn & 0x02) != 0) {
          intreturn(eax, ebx, (gmx - sht_win->vx0 - 5) / 8,
                    (gmy - sht_win->vy0 - 24) / 16, 2, edi, ebp);
          return;
        } else if ((mdec.btn & 0x04) != 0) {
          intreturn(eax, ebx, (gmx - sht_win->vx0 - 5) / 8,
                    (gmy - sht_win->vy0 - 24) / 16, 3, edi, ebp);
          return;
        } else if (mdec.roll != MOUSE_ROLL_NONE) {
          intreturn(eax, ebx, NowTask()->mx, NowTask()->my, 3 + mdec.roll, edi,
                    ebp);
          mdec.roll = MOUSE_ROLL_NONE;
          break;
        }
      }
    } else if (running_mode == POWERINTDOS) {
      struct TASK *task = NowTask();
      int i, mx1 = task->mx, my1 = task->my, bufx = task->mx * 8,
             bufy = task->my * 16;
      int bx = mx1;
      int by = my1;
      int bmp =
          *(char *)(task->TTY->vram + by * task->TTY->xsize * 2 + bx * 2 + 1);
      mouse_ready(&mdec);
      for (;;) {
        if (fifo8_status(TaskGetMousefifo(task)) == 0) {
          io_stihlt();
        } else {
          // printk("mx = %d my = %d\n",task->mx,task->my);
          i = fifo8_get(TaskGetMousefifo(task));
          if (mouse_decode(&mdec, i) != 0) {
            if (task->TTY != now_tty() && task->TTY->using1 == 1) {
              continue;
            }
            if ((mdec.btn & 0x01) != 0) {
              intreturn(eax, ebx, task->mx, task->my, 1, edi, ebp);
              break;
            } else if ((mdec.btn & 0x02) != 0) {
              intreturn(eax, ebx, task->mx, task->my, 2, edi, ebp);
              break;
            } else if ((mdec.btn & 0x04) != 0) {
              intreturn(eax, ebx, task->mx, task->my, 3, edi, ebp);
              break;
            } else if (mdec.roll != MOUSE_ROLL_NONE) {
              intreturn(eax, ebx, task->mx, task->my, 3 + mdec.roll, edi, ebp);
              break;
            }
            mx1 = task->mx;
            my1 = task->my;
            bufx += mdec.x;
            bufy += mdec.y;

            task->mx = bufx / 8;
            task->my = bufy / 16;

            if (bufx > (task->TTY->xsize - 1) * 8) {
              bufx = (task->TTY->xsize - 1) * 8;
            } else if (bufx < 0) {
              bufx = 0;
            }
            if (bufy > (task->TTY->ysize - 1) * 16) {
              bufy = (task->TTY->ysize - 1) * 16;
            } else if (bufy < 0) {
              bufy = 0;
            }
            *(char *)(task->TTY->vram + my1 * task->TTY->xsize * 2 + mx1 * 2 +
                      1) = bmp;
            bmp = *(char *)(task->TTY->vram + task->my * task->TTY->xsize * 2 +
                            task->mx * 2 + 1);
            *(char *)(task->TTY->vram + task->my * task->TTY->xsize * 2 +
                      task->mx * 2 + 1) = ~bmp;
            mouse_sleep(&mdec);
            sleep(50);
            mouse_ready(&mdec);
          }
        }
      }
      mouse_sleep(&mdec);
      *(char *)(task->TTY->vram + task->my * task->TTY->xsize * 2 +
                task->mx * 2 + 1) = bmp;
      task->mx = mx1;
      task->my = my1;
    }
  } else if (eax == 0x16) {
    if (ebx == 0x01) {
      intreturn(eax, ebx, ecx, getch(), esi, edi, ebp);
    } else if (ebx == 0x02) {
      intreturn(eax, ebx, ecx, input_char_inSM(), esi, edi, ebp);
    } else if (ebx == 0x03) {
      input((char *)(edx + ds_base), ecx);
    }
  } else if (eax == 0x19) {
    command_run((char *)(edx + ds_base));
  } else if (eax == 0x1a) {
    if (ebx == 0x01) {
      struct FILEINFO *finfo;
      finfo = Get_File_Address((char *)(edx + ds_base));
      if (finfo != 0) {
        intreturn(eax, ebx, ecx, finfo->size, esi, edi, ebp);
      } else {
        intreturn(eax, ebx, ecx, -1, esi, edi, ebp);
      }
      // intreturn(eax, ebx, ecx, finfo->size, esi, edi, ebp);
    } else if (ebx == 0x02) {
      struct FILEINFO *finfo;
      finfo = Get_File_Address((char *)(ds_base + edx));
      FILE *fp = fopen((char *)(ds_base + edx), "r");
      char *p = (char *)fp->buf;
      char *q = (char *)ds_base + esi;
      int i;
      if (fp != 0) {
        for (i = 0; i != finfo->size; i++) {
          q[i] = p[i];
        }
        fclose(fp);
        intreturn(1, ebx, ecx, edx, esi, edi, ebp);
      } else {
        intreturn(0, ebx, ecx, edx, esi, edi, ebp);
      }
    } else if (ebx == 0x03) {
      char *FilePath = (char *)(ds_base + edx);
      mkfile(FilePath);
    } else if (ebx == 0x04) {
      char *FilePath = (char *)(ds_base + edx);
      if (task->change_dict_times == 0) {
        mkdir(FilePath, 0);
      } else {
        struct FILEINFO *finfo =
            dict_search(".", task->directory,
                        drive_ctl.drives[task->drive_number].RootMaxFiles);
        mkdir(FilePath, finfo->clustno);
      }
    } else if (ebx == 0x05) {
      char *FilePath = (char *)(ds_base + edx);
      char *Ptr = (char *)ds_base + esi;
      int length = ecx;
      int offset = edi;
      EDIT_FILE(FilePath, Ptr, length, offset);
    }
  } else if (eax == 0x1b) {
    int i;
    char *bes = (char *)(edx + ds_base);
    for (i = 0; i < strlen(task->line); i++) {
      bes[i] = task->line[i];
    }
    bes[i] = 0;
  } else if (eax == 0x1c) {
    intreturn(Copy((char *)(edx + ds_base), (char *)(esi + ds_base)), ebx, ecx,
              edx, esi, edi, ebp);
  } else if (eax == 0x1d) {
    intreturn(kbhit(), ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x1e) {
    extern uint32_t app_num;
    app_num--;
    task->running = 0;
    WakeUp(GetTask(1)); // 别睡了，起来干活
  } else if (eax == 0x20) {
    // VBE驱动API
    if (running_mode == POWERINTDOS) {
      if (ebx == 0x01) {
        intreturn(SwitchVBEMode(ecx), ebx, ecx, edx, esi, edi, ebp);
      } else if (ebx == 0x02) {
        intreturn(check_vbe_mode(ecx, (struct VBEINFO *)VBEINFO_ADDRESS), ebx,
                  ecx, edx, esi, edi, ebp);
      } else if (ebx == 0x05) {
        intreturn(set_mode(ecx, edx, 32), ebx, ecx, edx, esi, edi, ebp);
      }
    }
  } else if (eax == 0x21) {
    if (running_mode == POWERINTDOS) {
      if (ebx == 0x01) {
        SwitchToText8025_BIOS();
        clear();
      } else if (ebx == 0x02) {
        SwitchTo320X200X256_BIOS();
      }
    }
  } else if (eax == 0x22) {
    //任务API
    if (ebx == 0x03) {
      task->forever = 1;
    } else if (ebx == 0x04) {
      SendIPCMessage(ecx, (void *)(ds_base + edx), esi, asynchronous);
    } else if (ebx == 0x05) {
      GetIPCMessage((void *)(ds_base + edx), ecx);
    } else if (ebx == 0x06) {
      intreturn(IPCMessageLength(ecx), ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x07) {
      intreturn(task->sel / 8 - 103, ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x08) {
      intreturn(haveMsg(), ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x09) {
      getMsgAll((void *)(ds_base + edx));
    } else if (ebx == 0x0a) {
      io_cli(); // 防止任务提前运行
      struct TASK *t =
          AddUserTask((char *)(ecx + ds_base), task->level, task->ss1 - 8, edx,
                      task->ss1, task->ss1, esi);
      // printk("task->ss1=%d\n",task->ss1);
      t->alloc_addr = task->alloc_addr;
      t->memman = task->memman;
      t->alloc_size = task->alloc_size;
      t->app = 1;
      t->forever = task->forever;
      t->tss.esp0 = (int)page_malloc(64 * 1024) + 64 * 1024;
      t->tss.ss0 = 1 * 8;
      t->cs_base = task->cs_base;
      t->ds_base = task->ds_base;
      char *kfifo = (char *)page_kmalloc(sizeof(struct FIFO8));
      char *mfifo = (char *)page_kmalloc(sizeof(struct FIFO8));
      char *kbuf = (char *)page_kmalloc(4096);
      char *mbuf = (char *)page_kmalloc(4096);
      fifo8_init((struct FIFO8 *)kfifo, 4096, (unsigned char *)kbuf);
      fifo8_init((struct FIFO8 *)mfifo, 4096, (unsigned char *)mbuf);
      TaskSetFIFO(t, (struct FIFO8 *)kfifo, (struct FIFO8 *)mfifo);
      t->is_child = 1;
      t->TTY = task->TTY;
      t->thread.father = task;
      // command_run("tl");
      // intreturn(t,ebx,ecx,edx,esi,edi,ebp);
      io_sti(); // 让任务运行
    } else if (ebx == 0x0b) {
      TaskLock();
    } else if (ebx == 0x0c) {
      TaskUnLock();
    } else if (ebx == 0x0d) {
      SubTask((struct TASK *)ecx);
    }
  } else if (eax == 0x23) {
    if (ebx == 0x01) {
      intreturn(task->TTY->color, ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x02) {
      task->TTY->color = ecx;
    }
  } else if (eax == 0x24) {
    // 计时器API
    if (ebx == 0x00) {
      // printf("Task:%s timer init\n",task->name);
      io_cli();
      task->timer = timer_alloc();
      task->timer->fifo = (struct FIFO8 *)page_malloc(sizeof(struct FIFO8));
      task->timer->fifo->buf =
          (unsigned char *)page_malloc(50 * sizeof(unsigned char));
      fifo8_init(task->timer->fifo, 50, task->timer->fifo->buf);
      timer_init(task->timer, task->timer->fifo, 1);
      io_sti();
    } else if (ebx == 0x01) {
      timer_settime(task->timer, ecx);
    } else if (ebx == 0x02) {
      if (fifo8_get(task->timer->fifo) == 1) {
        intreturn(1, ebx, ecx, edx, esi, edi, ebp);
      } else {
        intreturn(0, ebx, ecx, edx, esi, edi, ebp);
      }
    } else if (ebx == 0x03) {
      page_free((void *)task->timer->fifo->buf, 50 * sizeof(unsigned char));
      page_free((void *)task->timer->fifo, sizeof(struct FIFO8));
      timer_free(task->timer);
    }
  } else if (eax == 0x25) {
    intreturn(format(ebx), ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x26) {
    // CMOS时间
    if (ebx == 0x00) {
      intreturn(get_hour_hex(), ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x01) {
      intreturn(get_min_hex(), ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x02) {
      intreturn(get_sec_hex(), ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x03) {
      intreturn(get_day_of_month(), ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x04) {
      intreturn(get_day_of_week(), ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x05) {
      intreturn(get_mon_hex(), ebx, ecx, edx, esi, edi, ebp);
    } else if (ebx == 0x06) {
      intreturn(get_year(), ebx, ecx, edx, esi, edi, ebp);
    }
  } else if (eax == 0x27) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO *vbe = (struct VBEINFO *)VBEINFO_ADDRESS;
      // printk("x =%d y=%d color =%08x\n", ebx, ecx, edx);
      SDraw_Px((vram_t *)vbe->vram, ebx, ecx, edx, vbe->xsize);
    }
  } else if (eax == 0x28) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO *vbe = (struct VBEINFO *)VBEINFO_ADDRESS;
      vram_t *r = (vram_t *)vbe->vram;
      intreturn(r[ebx * vbe->xsize + ecx], ebx, ecx, edx, esi, edi, ebp);
    }
  } else if (eax == 0x29) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO *vbe = (struct VBEINFO *)VBEINFO_ADDRESS;
      vram_t *r = (vram_t *)vbe->vram;
      memcpy((void *)(ebx + ds_base), r, vbe->xsize * vbe->ysize * 4);
    }
  } else if (eax == 0x2a) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO *vbe = (struct VBEINFO *)VBEINFO_ADDRESS;
      vram_t *r = (vram_t *)vbe->vram;
      int x = ebx;
      int y = ecx;
      int w = edx;
      int h = esi;
      unsigned int *buffer = (unsigned int *)(edi + ds_base);
      for (int i = x; i < x + w; i++) {
        for (int j = y; j < y + h; j++) {
          r[j * vbe->xsize + i] = buffer[(j - y) * w + (i - x)];
        }
      }
    }
  } else if (eax == 0x2b) {
    if (running_mode == POWERINTDOS) {
      int a, c;
      a = 0;
      c = ebx;
      struct VBEINFO *vbe = (struct VBEINFO *)VBEINFO_ADDRESS;
      vram_t *vram_buffer = (vram_t *)vbe->vram;
      for (; c <= vbe->ysize; c++, a++) {
        for (int i = 0; i < vbe->xsize; i++) {
          // VBEDraw_Px(i,a,VBEGet_Px(i,c));
          vram_buffer[a * vbe->xsize + i] = vram_buffer[c * vbe->xsize + i];
        }
      }
      SDraw_Box(vram_buffer, 0, a, vbe->xsize, vbe->ysize, 0x0, vbe->xsize);
    }
  } else if (eax == 0x2c) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO *vbe = (struct VBEINFO *)VBEINFO_ADDRESS;
      vram_t *vram_buffer = (vram_t *)vbe->vram;
      (void)(vram_buffer);
      SDraw_Box((vram_t *)vbe->vram, ebx, ecx, edx, esi, edi, vbe->xsize);
    }
  } else if (eax == 0x2d) {
    intreturn(NTPTimeStamp(get_year(), get_mon_hex(), get_day_of_month(),
                           get_hour_hex(), get_min_hex(), get_sec_hex()),
              ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x2e) {
    intreturn(timerctl.count, ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x2f) {
    // Maskirq(0);
    // NowTask()->fpu_use =1;
    extern int st_task;
    st_task = Get_Tid(task);
    asm("fninit");
  }
  return;
}

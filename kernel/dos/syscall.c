#include <dos.h>
int buf[32] = {0};
int s[32] = {0};
int keybuf[32] = {0};
int mousebuf[32] = {0};
int keyfifo[32] = {0};
int mousefifo[32] = {0};
int format(char drive);
static unsigned char *stack;
void inthandler36(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx,
                  int eax) {
  // PowerintDOS API
  // printk("API (%08x)\n", eax);
  struct TASK *task = NowTask();
  int cs_base = task->cs_base;
  int ds_base = task->ds_base;
  int alloc_addr = task->alloc_addr; // malloc地址
  int alloc_size = task->alloc_size;
  char *memman = task->memman;
  if (eax == 0x01) {
    intreturn(eax, ebx, ecx, 0x302e3663, esi, edi, ebp); // 0.6c
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
        PrintChineseStr(ecx, edx, edi, (char *)esi + ds_base);
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
        // printk("DS_BASE:%08x ALLOC_ADDR:%08x ALLOC:%08x ~ %08x\n", ds_base,
        //        alloc_addr, (i - j) * 128 + (alloc_addr - ds_base));
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
      extern unsigned int gmx, gmy;
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
        }
      }
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
      char *p = fp->buf;
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
    Copy((char *)(edx + ds_base), (char *)(esi + ds_base));
  } else if (eax == 0x1d) {
    intreturn(kbhit(), ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x1e) {
    extern uint32_t app_num;
    app_num--;
    task->running = 0;
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
    if (ebx == 0x01) {
      io_cli();
      Maskirq(0);
      int bp = 0;
      for (; bp != 32;) {
        if (buf[bp] == 0 && s[bp] == 0 && keyfifo[bp] == 0 &&
            mousefifo[bp] == 0)
          break;
        else
          bp++;
      }
      if (bp == 31) {
        intreturn(eax, ebx, -1, edx, esi, edi, ebp);
        ClearMaskIrq(0);
        io_sti();
        return;
      }
      s[bp] = page_malloc(1024);
      if (s[bp] >= alloc_addr + 512 * 1024 || s[bp] < alloc_addr) { // 内存不够
        page_free(s[bp], 1024);
        intreturn(eax, ebx, -1, edx, esi, edi, ebp);
        ClearMaskIrq(0);
        io_sti();
        return;
      }
      s[bp] -= ds_base;
      s[bp] += 1024;
      buf[bp] = (int)AddTask((char *)(edx + ds_base), task->level,
                             task->ss1 - 8, ecx, task->ss1, task->ss1, s[bp]);
      Maskirq(0);
      keybuf[bp] = page_malloc(128);
      mousebuf[bp] = page_malloc(128);
      keyfifo[bp] = page_malloc(sizeof(struct FIFO8));
      mousefifo[bp] = page_malloc(sizeof(struct FIFO8));
      if (mousefifo[bp] >= alloc_addr + 512 * 1024 ||
          mousefifo[bp] < alloc_addr) { // 内存不够（只判断最后一个）
        page_free(buf[bp], sizeof(struct TASK));
        page_free(s[bp] + ds_base - 1024, 1024);
        page_free(keybuf[bp], 128);
        page_free(mousebuf[bp], 128);
        page_free(keyfifo[bp], sizeof(struct FIFO8));
        page_free(mousefifo[bp], sizeof(struct FIFO8));
        intreturn(eax, ebx, -1, edx, esi, edi, ebp);
        ClearMaskIrq(0);
        io_sti();
        return;
      }
      fifo8_init((struct FIFO8 *)keyfifo[bp], 128, keybuf[bp]);
      fifo8_init((struct FIFO8 *)mousefifo[bp], 128, mousebuf[bp]);
      TaskSetFIFO((struct TASK *)buf[bp], (struct FIFO8 *)keyfifo[bp],
                  (struct FIFO8 *)mousefifo[bp]);
      struct TASK *ttask = (struct TASK *)(buf[bp]);
      ttask->ds_base = task->ds_base;
      ttask->cs_base = task->cs_base;
      ttask->alloc_addr = task->alloc_addr;
      ttask->alloc_size = task->alloc_size;
      ttask->memman = task->memman;
      stack = page_kmalloc(4 * 1024);
      ttask->esp0 = stack + 4 * 1024;
      intreturn(eax, ebx, (ttask->sel / 8) - (task->sel / 8), edx, esi, edi,
                ebp); // 返回子进程（线程）ID号
      ClearMaskIrq(0);
      io_sti();
    } else if (ebx == 0x02) {
      Maskirq(0);
      if (ecx > 31 || ecx < 1)
        return;
      struct TASK *ttask = (struct TASK *)buf[ecx - 1];
      page_free(buf[ecx - 1], sizeof(struct TASK));
      page_free(s[ecx - 1] + ds_base - 1024, 1024);
      page_free(keybuf[ecx - 1], 32);
      page_free(mousebuf[ecx - 1], 128);
      page_free(keyfifo[ecx - 1], sizeof(struct FIFO8));
      page_free(mousefifo[ecx - 1], sizeof(struct FIFO8));
      page_free(stack, 4 * 1024);
      buf[ecx - 1] = 0;
      s[ecx - 1] = 0;
      keyfifo[ecx - 1] = 0;
      mousefifo[ecx - 1] = 0;
      ClearMaskIrq(0);
      SleepTask(ttask);
      ttask->running = 0;
    } else if (ebx == 0x03) {
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
      struct TASK *t = AddTask(ecx + ds_base, task->level, task->tss.cs, edx,
                               task->tss.ds, task->tss.ss, esi);
      t->alloc_addr = task->alloc_addr;
      t->memman = task->memman;
      t->alloc_size = task->alloc_size;
      t->app = 1;
      t->forever = task->forever;
      t->esp0 = (unsigned char *)page_malloc(64 * 1024) + 64 * 1024;
      t->cs_base = task->cs_base;
      t->ds_base = task->ds_base;
      TaskSetFIFO(t, TaskGetKeyfifo(task), TaskGetMousefifo(task));
      t->is_child = 1;
      t->thread.father = task;
      // command_run("tl");
      // intreturn(t,ebx,ecx,edx,esi,edi,ebp);
      io_sti(); // 让任务运行
    } else if (ebx == 0x0b) {
      TaskLock();
    } else if (ebx == 0x0c) {
      TaskUnLock();
    } else if (ebx == 0x0d) {
      SubTask(ecx);
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
      task->timer->fifo->buf = (char *)page_malloc(50 * sizeof(unsigned char));
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
      struct VBEINFO *vbe = VBEINFO_ADDRESS;
      // printk("x =%d y=%d color =%08x\n", ebx, ecx, edx);
      SDraw_Px(vbe->vram, ebx, ecx, edx, vbe->xsize);
    }
  } else if (eax == 0x28) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO *vbe = VBEINFO_ADDRESS;
      vram_t *r = vbe->vram;
      intreturn(r[ebx * vbe->xsize + ecx], ebx, ecx, edx, esi, edi, ebp);
    }
  } else if (eax == 0x29) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO *vbe = VBEINFO_ADDRESS;
      vram_t *r = vbe->vram;
      memcpy(ebx + ds_base, r, vbe->xsize * vbe->ysize * 4);
    }
  } else if (eax == 0x2a) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO *vbe = VBEINFO_ADDRESS;
      vram_t *r = vbe->vram;
      memcpy(r, ebx + ds_base, vbe->xsize * vbe->ysize * 4);
    }
  } else if (eax == 0x2b) {
    if (running_mode == POWERINTDOS) {
      int a, c;
      a = 0;
      c = ebx;
      struct VBEINFO *vbe = VBEINFO_ADDRESS;
      vram_t *vram_buffer = vbe->vram;
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
      struct VBEINFO *vbe = VBEINFO_ADDRESS;
      vram_t *vram_buffer = vbe->vram;
      SDraw_Box(vbe->vram, ebx, ecx, edx, esi, edi, vbe->xsize);
    }
  }
  return;
}
int format(char drive) {
  // A,B盘——软盘
  // C盘——IDE/SATA硬盘主分区
  // D,E,F...盘——IDE/USB/SATA存储介质/分区/虚拟磁盘
  FILE *fp = fopen("tskdrv:\\boot.bin", "r");
  void *read_in = page_malloc(fp->size);
  fread(read_in, fp->size, 1, fp);
  if (!(drive - 'A')) {
    // printf("3K FloppyDisk: %d bytes\n", 2880 * 512);
    // printf("INT 13H DriveNumber: 0\n");
    // printf("RootDictFiles: 224\n");
    // printf("drive_ctl.drives[%d].ClustnoBytes: 512 "
    //        "bytes\n",
    //        NowTask()->drive_number);
    *(unsigned char *)(&read_in[BPB_SecPerClus]) = 1;
    *(unsigned short *)(&read_in[BPB_RootEntCnt]) = 224;
    *(unsigned short *)(&read_in[BPB_TotSec16]) = 2880;
    *(unsigned int *)(&read_in[BPB_TotSec32]) = 2880;
    *(unsigned char *)(&read_in[BS_DrvNum]) = 0;
    write_floppy_for_ths(0, 0, 1, read_in, 1);
    unsigned int *fat = (unsigned int *)page_malloc(9 * 512);
    fat[0] = 0x00fffff0;
    write_floppy_for_ths(0, 0, 2, fat, 9);
    write_floppy_for_ths(0, 0, 11, fat, 9);
    page_free((void *)fat, 9 * 512);
    void *null_sec = page_malloc(512);
    for (int i = 0; i < 224 * 32 / 512; i++) {
      write_floppy_for_ths(0, 0, 20 + i, null_sec, 1);
    }
    page_free(null_sec, 512);
  } else if (drive != 'B') {
    // struct IDEHardDiskInfomationBlock* info = drivers_idehdd_info();
    // printk("drive=%c %d\n", drive, have_vdisk(drive));
    if (!have_vdisk(drive) && !DiskReady(drive)) {
      // printf("Couldn't find Disk.\n");
      return;
    }
    if (DiskReady(drive)) {
      // printf("IDE HardDisk ID:%s\n", ide_devices[drive - 'C'].Model);
    }

    // printf("Disk: %d bytes\n", disk_Size(drive));
    // printf("RootDictFiles: %d\n",
    //        14 * (((disk_Size(drive) / 4096) / 512 + 1) * 512) / 32);
    // printf("ClustnoBytes: %d bytes\n",
    //        ((disk_Size(drive) / 4096) / 512 + 1) * 512);
    *(unsigned char *)(&read_in[BPB_SecPerClus]) =
        ((disk_Size(drive) / 4096) / 512 + 1);
    *(unsigned short *)(&read_in[BPB_RootEntCnt]) =
        14 * (((disk_Size(drive) / 4096) / 512 + 1) * 512) / 32;
    // printk("Sectors:%d\n", ide_devices[drive - 'C'].Size /power
    if (disk_Size(drive) / 512 > 65535) {
      *(unsigned short *)(&read_in[BPB_TotSec16]) = 0;
    } else {
      *(unsigned short *)(&read_in[BPB_TotSec16]) = disk_Size(drive) / 512;
    }
    *(unsigned int *)(&read_in[BPB_TotSec32]) = disk_Size(drive) / 512;
    *(unsigned char *)(&read_in[BS_DrvNum]) = drive - 'C' + 0x80;
    Disk_Write(0, 1, (unsigned short *)read_in, drive);
    unsigned int *fat = (unsigned int *)page_malloc(9 * 512);
    fat[0] = 0x00fffff0;
    Disk_Write(1, 9, (unsigned short *)fat, drive);
    Disk_Write(10, 9, (unsigned short *)fat, drive);
    page_free((void *)fat, 9 * 512);
    void *null_sec = page_malloc(512);
    clean((char *)null_sec, 512);
    for (int i = 0;
         i < 14 * (((disk_Size(drive) / 4096) / 512 + 1) * 512) / 32 * 32 / 512;
         i++) {
      Disk_Write(19, 1, (unsigned short *)null_sec, drive);
    }
    page_free(null_sec, 512);
    // page_free((void*)info, 256 * sizeof(short));
  }
  page_free(read_in, fp->size);
  fclose(fp);
  return 0;
}
#include <dos.h>
void kbd_press(uint8_t dat, uint32_t task) {
  fifo8_put(get_task(task)->Pkeyfifo, dat);
}
void kbd_up(uint8_t dat, uint32_t task) {
  fifo8_put(get_task(task)->Ukeyfifo, dat);
}
enum { EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX };
void inthandler36(int edi,
                  int esi,
                  int ebp,
                  int esp,
                  int ebx,
                  int edx,
                  int ecx,
                  int eax) {
  // PowerintDOS API
  struct TASK* task = current_task();
  int ds_base = task->ds_base;
  int alloc_addr = task->alloc_addr;  // malloc地址
  int alloc_size = task->alloc_size;
  int* reg = &eax + 1; /* eax后面的地址*/
                            /*强行改写通过PUSHAD保存的值*/
  /* reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP */
  /* reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */
  if (eax == 0x01) {
    reg[EDX] = 0x302e3762;
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
        Draw_Str(ecx, edx, (char*)esi + ds_base, edi);
      } else if (ebx == 0x08) {
        PrintChineseStr(ecx, edx, edi, (unsigned char*)esi + ds_base);
      }
    }
  } else if (eax == 0x04) {
    gotoxy(edx, ecx);
  } else if (eax == 0x05) {
    //printf("get it! %08x\n",edx);
    print((char*)edx + ds_base);
  } else if (eax == 0x06) {
    sleep(edx);
  } else if (eax == 0x08) {
    reg[EDX] = (alloc_addr - ds_base);
  } else if (eax == 0x09) {
    reg[EDX] = alloc_size;
  } else if (eax == 0x0c) {
    Text_Draw_Box(ecx, ebx, esi, edx, (unsigned char)edi);
  } else if (eax == 0x0e) {
    reg[ECX] = get_y();
    reg[EDX] = get_x();
  } else if (eax == 0x0d) {
    beep(ebx, ecx, edx);
  } else if (eax == 0x0f) {
    if (running_mode == POWERDESKTOP) {
      extern int gmx, gmy;
      struct SHEET* sht_win = (struct SHEET*)task->TTY->reserved[1];
      for (;;) {
        if ((mdec.btn & 0x01) != 0) {
          reg[ECX] = (gmx - sht_win->vx0 - 5) / 8;
          reg[EDX] = (gmy - sht_win->vy0 - 24) / 16;
          reg[ESI] = 1;
          return;
        } else if ((mdec.btn & 0x02) != 0) {
          reg[ECX] = (gmx - sht_win->vx0 - 5) / 8;
          reg[EDX] = (gmy - sht_win->vy0 - 24) / 16;
          reg[ESI] = 2;
          return;
        } else if ((mdec.btn & 0x04) != 0) {
          reg[ECX] = (gmx - sht_win->vx0 - 5) / 8;
          reg[EDX] = (gmy - sht_win->vy0 - 24) / 16;
          reg[ESI] = 3;
          return;
        } else if (mdec.roll != MOUSE_ROLL_NONE) {
          reg[ECX] = current_task()->mx;
          reg[EDX] = current_task()->my;
          reg[ESI] = 3 + mdec.roll;
          mdec.roll = MOUSE_ROLL_NONE;
          break;
        }
      }
    } else if (running_mode == POWERINTDOS) {
      struct TASK* task = current_task();
      int i, mx1 = task->mx, my1 = task->my, bufx = task->mx * 8,
             bufy = task->my * 16;
      int bx = mx1;
      int by = my1;
      int bmp =
          *(char*)(task->TTY->vram + by * task->TTY->xsize * 2 + bx * 2 + 1);
      mouse_ready(&mdec);
      for (;;) {
        if (fifo8_status(task_get_mouse_fifo(task)) == 0) {
          io_stihlt();
        } else {
          // printk("mx = %d my = %d\n",task->mx,task->my);
          i = fifo8_get(task_get_mouse_fifo(task));
          if (mouse_decode(&mdec, i) != 0) {
            if (task->TTY != now_tty() && task->TTY->using1 == 1) {
              continue;
            }
            if ((mdec.btn & 0x01) != 0) {
              reg[ECX] = task->mx;
              reg[EDX] = task->my;
              reg[ESI] = 1;
              break;
            } else if ((mdec.btn & 0x02) != 0) {
              reg[ECX] = task->mx;
              reg[EDX] = task->my;
              reg[ESI] = 2;
              break;
            } else if ((mdec.btn & 0x04) != 0) {
              reg[ECX] = task->mx;
              reg[EDX] = task->my;
              reg[ESI] = 3;
              break;
            } else if (mdec.roll != MOUSE_ROLL_NONE) {
              reg[ECX] = task->mx;
              reg[EDX] = task->my;
              reg[ESI] = 3 + mdec.roll;
              break;
            }
            mx1 = task->mx;
            my1 = task->my;
            bufx += mdec.x;
            bufy += mdec.y;

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
            task->mx = bufx / 8;
            task->my = bufy / 16;
            *(char*)(task->TTY->vram + my1 * task->TTY->xsize * 2 + mx1 * 2 +
                     1) = bmp;
            bmp = *(char*)(task->TTY->vram + task->my * task->TTY->xsize * 2 +
                           task->mx * 2 + 1);
            *(char*)(task->TTY->vram + task->my * task->TTY->xsize * 2 +
                     task->mx * 2 + 1) = ~bmp;
            //  mouse_sleep(&mdec);
            //  sleep(50);
            mouse_ready(&mdec);
          }
        }
      }
      mouse_sleep(&mdec);
      *(char*)(task->TTY->vram + task->my * task->TTY->xsize * 2 +
               task->mx * 2 + 1) = bmp;
      task->mx = mx1;
      task->my = my1;
    }
  } else if (eax == 0x16) {
    if (ebx == 0x01) {
      reg[EDX] = getch();
    } else if (ebx == 0x02) {
      reg[EDX] = input_char_inSM();
    } else if (ebx == 0x03) {
      input((char*)(edx + ds_base), ecx);
    }
  } else if (eax == 0x19) {
    command_run((char*)(edx + ds_base));
  } else if (eax == 0x1a) {
    if (ebx == 0x01) {
      int fsize = vfs_filesize((char*)(edx + ds_base));
      if (fsize != -1) {
        reg[EDX] = fsize;
      } else {
        reg[EDX] = -1;
      }
    } else if (ebx == 0x02) {
      int fsize = vfs_filesize((char*)(ds_base + edx));
      FILE* fp = fopen((char*)(ds_base + edx), "r");
      char* p = (char*)fp->buffer;
      char* q = (char*)ds_base + esi;
      int i;
      if (fp != 0) {
        for (i = 0; i != fsize; i++) {
          q[i] = p[i];
        }
        fclose(fp);
        reg[EAX] = 1;
      } else {
        reg[EAX] = 1;
      }
    } else if (ebx == 0x03) {
      char* FilePath = (char*)(ds_base + edx);
      vfs_createfile(FilePath);
    } else if (ebx == 0x04) {
      char* FilePath = (char*)(ds_base + edx);
      char buf[270];
      sprintf(buf, "mkdir %s", FilePath);
      command_run(buf);
    } else if (ebx == 0x05) {
      printk("EDIT_FILE\n");
      char* FilePath = (char*)(ds_base + edx);
      char* Ptr = (char*)ds_base + esi;
      int length = ecx;
      int offset = edi;
      EDIT_FILE(FilePath, Ptr, length, offset);
    }
  } else if (eax == 0x1b) {
    int i;
    char* bes = (char*)(edx + ds_base);
    for (i = 0; i < strlen(task->line); i++) {
      bes[i] = task->line[i];
    }
    bes[i] = 0;
  } else if (eax == 0x1c) {
    reg[EAX] = Copy((char*)(edx + ds_base), (char*)(esi + ds_base));
  } else if (eax == 0x1d) {
    reg[EAX] = kbhit();
  } else if (eax == 0x1e) {
    while (FindForCount(1, vfs_now->path) != NULL) {
      // printk("%d\n",vfs_now->path->ctl->all);
      page_free(FindForCount(vfs_now->path->ctl->all, vfs_now->path)->val,
                 255);
      DeleteVal(vfs_now->path->ctl->all, vfs_now->path);
      extern struct TASK* mouse_use_task;
      if (mouse_use_task == task) {
        mouse_sleep(&mdec);
      }
    }
    DeleteList(vfs_now->path);
    page_free((void *)vfs_now, sizeof(vfs_t));
    extern uint32_t app_num;
    app_num--;
    printk("at the last\n");
    task->running = 0;
    task_wake_up(get_task(1));  // 别睡了，起来干活
    task_sleep(current_task());
    while (1)
      ;
  } else if (eax == 0x20) {
    // VBE驱动API
    if (running_mode == POWERINTDOS) {
      if (ebx == 0x01) {
        reg[EAX] = SwitchVBEMode(ecx);
      } else if (ebx == 0x02) {
        reg[EAX] = check_vbe_mode(ecx, (struct VBEINFO*)VBEINFO_ADDRESS);
      } else if (ebx == 0x05) {
        reg[EAX] = set_mode(ecx, edx, 32);
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
      send_ipc_message(ecx, (void*)(ds_base + edx), esi, asynchronous);
    } else if (ebx == 0x05) {
      get_ipc_message((void*)(ds_base + edx), ecx);
    } else if (ebx == 0x06) {
      reg[EAX] = ipc_message_len(ecx);
    } else if (ebx == 0x07) {
      reg[EAX] = task->sel / 8 - 103;
    } else if (ebx == 0x08) {
      reg[EAX] = have_msg();
    } else if (ebx == 0x09) {
      get_msg_all((void*)(ds_base + edx));
    } else if (ebx == 0x0a) {
      io_cli();  // 防止任务提前运行
      extern int init_ok_flag;
      init_ok_flag = 0;
      struct TASK* t =
          register_user_task((char*)(ecx + ds_base), task->level, task->cs_start, edx,
                      task->ss_start, task->ss_start, esi);
      init_ok_flag = 1;
      t->alloc_addr = task->alloc_addr;
      t->alloc_size = task->alloc_size;
      t->app = 1;
      t->forever = task->forever;
      t->tss.esp0 = (int)page_malloc(64 * 1024) + 64 * 1024;
      t->tss.ss0 = 1 * 8;
      t->cs_base = task->cs_base;
      t->ds_base = task->ds_base;
      t->nfs = task->nfs;
      char* kfifo = (char*)page_malloc(sizeof(struct FIFO8));
      char* mfifo = (char*)page_malloc(sizeof(struct FIFO8));
      char* kbuf = (char*)page_malloc(4096);
      char* mbuf = (char*)page_malloc(4096);
      fifo8_init((struct FIFO8*)kfifo, 4096, (unsigned char*)kbuf);
      fifo8_init((struct FIFO8*)mfifo, 4096, (unsigned char*)mbuf);
      task_set_fifo(t, (struct FIFO8*)kfifo, (struct FIFO8*)mfifo);
      t->is_child = 1;
      t->TTY = task->TTY;
      t->thread.father = task;
      // command_run("tl");
      io_sti();  // 让任务运行
    } else if (ebx == 0x0b) {
      task_lock();
    } else if (ebx == 0x0c) {
      task_unlock();
    } else if (ebx == 0x0d) {
      task_delete((struct TASK*)ecx);
    }
  } else if (eax == 0x23) {
    if (ebx == 0x01) {
      reg[EAX] = task->TTY->color;
    } else if (ebx == 0x02) {
      task->TTY->color = ecx;
    }
  } else if (eax == 0x24) {
    // 计时器API
    if (ebx == 0x00) {
      // printf("Task:%s timer init\n",task->name);
      io_cli();
      task->timer = timer_alloc();
      task->timer->fifo = (struct FIFO8*)page_malloc(sizeof(struct FIFO8));
      task->timer->fifo->buf =
          (unsigned char*)page_malloc(50 * sizeof(unsigned char));
      fifo8_init(task->timer->fifo, 50, task->timer->fifo->buf);
      timer_init(task->timer, task->timer->fifo, 1);
      io_sti();
    } else if (ebx == 0x01) {
      timer_settime(task->timer, ecx);
    } else if (ebx == 0x02) {
      if (fifo8_get(task->timer->fifo) == 1) {
        reg[EAX] = 1;
      } else {
        reg[EAX] = 0;
      }
    } else if (ebx == 0x03) {
      page_free((void*)task->timer->fifo->buf, 50 * sizeof(unsigned char));
      page_free((void*)task->timer->fifo, sizeof(struct FIFO8));
      timer_free(task->timer);
    }
  } else if (eax == 0x25) {
    reg[EAX] = vfs_format(ebx, "FAT");
  } else if (eax == 0x26) {
    // CMOS时间
    if (ebx == 0x00) {
      reg[EAX] = get_hour_hex();
    } else if (ebx == 0x01) {
      reg[EAX] = get_min_hex();
    } else if (ebx == 0x02) {
      reg[EAX] = get_sec_hex();
    } else if (ebx == 0x03) {
      reg[EAX] = get_day_of_month();
    } else if (ebx == 0x04) {
      reg[EAX] = get_day_of_week();
    } else if (ebx == 0x05) {
      reg[EAX] = get_mon_hex();
    } else if (ebx == 0x06) {
      reg[EAX] = get_year();
    }
  } else if (eax == 0x27) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO* vbe = (struct VBEINFO*)VBEINFO_ADDRESS;
      // printk("x =%d y=%d color =%08x\n", ebx, ecx, edx);
      SDraw_Px((vram_t*)vbe->vram, ebx, ecx, edx, vbe->xsize);
    }
  } else if (eax == 0x28) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO* vbe = (struct VBEINFO*)VBEINFO_ADDRESS;
      vram_t* r = (vram_t*)vbe->vram;
      reg[EAX] = r[ebx * vbe->xsize + ecx];
    }
  } else if (eax == 0x29) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO* vbe = (struct VBEINFO*)VBEINFO_ADDRESS;
      vram_t* r = (vram_t*)vbe->vram;
      memcpy((void*)(ebx + ds_base), r, vbe->xsize * vbe->ysize * 4);
    }
  } else if (eax == 0x2a) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO* vbe = (struct VBEINFO*)VBEINFO_ADDRESS;
      vram_t* r = (vram_t*)vbe->vram;
      int x = ebx;
      int y = ecx;
      int w = edx;
      int h = esi;
      unsigned int* buffer = (unsigned int*)(edi + ds_base);
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
      struct VBEINFO* vbe = (struct VBEINFO*)VBEINFO_ADDRESS;
      vram_t* vram_buffer = (vram_t*)vbe->vram;
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
      struct VBEINFO* vbe = (struct VBEINFO*)VBEINFO_ADDRESS;
      vram_t* vram_buffer = (vram_t*)vbe->vram;
      (void)(vram_buffer);
      SDraw_Box((vram_t*)vbe->vram, ebx, ecx, edx, esi, edi, vbe->xsize);
    }
  } else if (eax == 0x2d) {
    reg[EAX] = ntp_time_stamp(get_year(), get_mon_hex(), get_day_of_month(),
                            get_hour_hex(), get_min_hex(), get_sec_hex());
  } else if (eax == 0x2e) {
    reg[EAX] = timerctl.count;
  } else if (eax == 0x2f) {
    task->fpu = 0;
    task->fpu_flag = 0;
  } else if (eax == 0x30) {
    current_task()->Pkeyfifo = malloc(sizeof(struct FIFO8));
    current_task()->Ukeyfifo = malloc(sizeof(struct FIFO8));
    unsigned char* kbuf = (unsigned char*)page_malloc(4096);
    unsigned char* mbuf = (unsigned char*)page_malloc(4096);
    fifo8_init(current_task()->Pkeyfifo, 4096, kbuf);
    fifo8_init(current_task()->Ukeyfifo, 4096, mbuf);
    current_task()->keyboard_press = kbd_press;
    current_task()->keyboard_release = kbd_up;
  } else if (eax == 0x31) {
    reg[EAX] = fifo8_status(current_task()->Pkeyfifo);
  } else if (eax == 0x32) {
    reg[EAX] = fifo8_status(current_task()->Ukeyfifo);
  } else if (eax == 0x33) {
    reg[EAX] = fifo8_get(current_task()->Pkeyfifo);
  } else if (eax == 0x34) {
    reg[EAX] = fifo8_get(current_task()->Ukeyfifo);
  } else if (eax == 0x35) {
     char* new_buf = (char*)page_malloc(task->gdt_data[1] + ebx);
     memcpy(new_buf,(void *)task->gdt_data[2], task->gdt_data[1]);
     page_free((void *)task->gdt_data[2], task->gdt_data[1]);
     task->gdt_data[2] = (uint32_t)new_buf;
     task->gdt_data[1] = task->gdt_data[1] + ebx;
     set_segmdesc((struct SEGMENT_DESCRIPTOR* )task->gdt_data[0], task->gdt_data[1], task->gdt_data[2],
                  task->gdt_data[3]);
     if(task->cs_base == task->ds_base) {
       set_segmdesc((struct SEGMENT_DESCRIPTOR* )task->gdt_data[4], task->gdt_data[1], task->gdt_data[2],
                  AR_CODE32_ER | 3 << 5);
       task->cs_base = (uint32_t)new_buf;
     }
     task->ds_base = (uint32_t)new_buf;
     task->alloc_size = task->gdt_data[1];
     task->alloc_addr = (void *)new_buf;
  }
  return;
}

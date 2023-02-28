// （驱动）应用程序和批处理文件的处理函数
#include <ELF.h>
#include <cmd.h>
#include <dos.h>
int app_task_num = -1; // 应用程序的任务号（-1代表没在运行应用程序）
int run_bat(char *cmdline) {
  // 运行批处理文件
  struct FILEINFO *finfo;
  char *file;
  int i, j = 0;
  char *name = page_malloc(300);
  char *file1 = page_malloc(1024);
  clean(name, 300);
  for (i = 0; i < strlen(cmdline); i++) {
    if (cmdline[i] <= ' ') {
      break;
    }
    name[i] = cmdline[i];
  }
  name[i] = 0;
  finfo = Get_File_Address(
      name); //获取文件所在的地址，保存为FILEINFO数据结构，以便我们获取信息
  if (finfo == 0) {
    finfo = Path_Find_File(name, Path_Addr);
    Path_Find_FileName(name, name, Path_Addr);
  }
  if (finfo == 0) //没找到这个文件
  {
    //加上后缀再试一遍
    name = strcat(name, ".BAT");
    finfo = Get_File_Address(name);
  }
  if (finfo == 0) {
    finfo = Path_Find_File(name, Path_Addr);
    Path_Find_FileName(name, name, Path_Addr);
  }
  if (finfo != 0) {
    if (stricmp(".BAT", &name[strlen(name) - 4]) != 0) {
      page_free(file1, 1024);
      page_free(name, 300);
      return 0;
    }
    FILE *fp = fopen(name, "r");
    file = fp->buf;
    //读取每行的内容，然后调用命令解析函数（command_run）
    for (i = 0; i != finfo->size; i++) {
      if (file[i] == 0x0d && file[i + 1] == 0x0a) {
        command_run(file1);
        i++;
        j = 0;
        int p;
        clean(file1, 1024);
        continue;
      }
      file1[j] = file[i];
      j++;
    }
    int p;
    command_run(file1);
    fclose(fp);
    page_free(file1, 1024);
    page_free(name, 300);
    return 1;
  } else {
    page_free(file1, 1024);
    page_free(name, 300);
    return 0;
  }
}
uint32_t app_num = 0;
struct TASK *start_drv(char *cmdline) {
  struct TASK *result;
  struct FILEINFO *finfo;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
  char *name, *p, *q,
      *alloc; // name:文件名，p:代码段，q:数据段，alloc:分配的内存
  unsigned char *stack; // 用于应用程序系统调用及硬件中断的堆栈
  struct TASK *app_task; //任务
  int i, segsiz, datsiz, dathrb, esp;
  name = (char *)page_malloc(300); //分配300字节内存
  clean(name, 300); //清空内存(为了避免脏数据，这里全部填充为0)
  for (i = 0; i < strlen(cmdline); i++) {
    if (cmdline[i] == ' ') {
      //如果碰到空格（参数部分），就说明文件名的部分结束了，这时候我们直接跳出循环即可
      break;
    }
    name[i] = cmdline[i];
  }
  name[i] =
      0; //置字符串结束符0
         //虽然前面已经全部填充为0了，这里为了保险，还是重新加上字符串结束符（'\0'）
  finfo = Get_File_Address(
      name); //获取文件所在的地址，保存为FILEINFO数据结构，以便我们获取信息
  if (finfo == 0) {
    finfo = Path_Find_File(name, Path_Addr);
    Path_Find_FileName(name, name, Path_Addr);
  }
  if (finfo == 0) //没找到这个文件
  {
    //加上后缀再试一遍
    name = strcat(name, ".DRV");
    finfo = Get_File_Address(name);
  }
  if (finfo == 0) {
    finfo = Path_Find_File(name, Path_Addr);
    Path_Find_FileName(name, name, Path_Addr);
  }
  if (finfo != 0) {
    if (stricmp(".DRV", &name[strlen(name) - 4]) != 0) {
      page_free((void *)name, 300);
      return 0;
    }
    // 代码段的物理内存必须是连续的
    FILE *fp = fopen(name, "r");
    if (fp->buf[0] ==
        'A') // A=ASM,这是汇编语言编写的程序（不分代码段和数据段，所以我们将他们两个分开，以便我们编写代码）
    {
      p = (char *)page_malloc(
          finfo->size + 64 * 1024 +
          512 *
              1024); //分配程序的代码段（由于这里是汇编语言的程序，所以在代码段后面要加上512KB的待分配内存）
      memcpy(
          p, fp->buf,
          finfo
              ->size); // 将文件拷贝到代码段中（由于是只有一个段的程序，所以直接无脑拷贝即可）
      fclose(fp);
      set_segmdesc(gdt + 3 + app_num * 2, finfo->size - 1, (int)p,
                   AR_CODE32_ER); //设置他们的段描述符
      set_segmdesc(gdt + 4 + app_num * 2,
                   finfo->size - 1 + 64 * 1024 + 512 * 1024, (int)p,
                   AR_DATA32_RW);
      io_cli();
      struct TASK *this_task =
          AddTask(name, 2, ((3 + app_num * 2) * 8), 1, ((4 + app_num * 2) * 8),
                  ((4 + app_num * 2) * 8), finfo->size + 64 * 1024);
      this_task->cs_base = p;
      this_task->ds_base = p;
      this_task->alloc_addr = p + finfo->size + 64 * 1024;
      this_task->alloc_size = 512 * 1024;
      stack = (unsigned char *)page_malloc(64 * 1024);
      this_task->esp0 = stack + 64 * 1024;
      this_task->directory = NowTask()->directory;
      strcpy(this_task->path, NowTask()->path);
      this_task->drive = NowTask()->drive;
      this_task->drive_number = NowTask()->drive_number;
      this_task->change_dict_times = NowTask()->change_dict_times;
      char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *kbuf = (char *)page_kmalloc(4096);
      char *mbuf = (char *)page_kmalloc(4096);
      char *memman = (char *)page_malloc(4 * 1024);
      this_task->memman = memman;
      fifo8_init(kfifo, 4096, kbuf);
      fifo8_init(mfifo, 4096, mbuf);
      TaskSetFIFO(this_task, kfifo, mfifo);
      this_task->TTY = NowTask()->TTY;
      app_task_num = this_task->sel / 8 - 103;
      app_num++;
      SleepTaskFIFO(NowTask());
      io_sti();
      if (1) {
        io_cli();
        change_page_task_id(this_task->sel / 8 - 103, p,
                            finfo->size + 64 * 1024 + 512 * 1024);
        change_page_task_id(this_task->sel / 8 - 103, stack, 64 * 1024);
        change_page_task_id(this_task->sel / 8 - 103, memman, 4 * 1024);
        change_page_task_id(this_task->sel / 8 - 103, kfifo,
                            sizeof(struct FIFO8));
        change_page_task_id(this_task->sel / 8 - 103, mfifo,
                            sizeof(struct FIFO8));
        change_page_task_id(this_task->sel / 8 - 103, kbuf, 4096);
        change_page_task_id(this_task->sel / 8 - 103, mbuf, 4096);
        io_sti();
        WakeUp(NowTask());
        app_task_num = -1;
        print("\n");
        result = this_task;
        goto end;
      }

      // task_sr1会帮我们结束程序
      // SubTask(GetTaskForName(name)); //程序退出咯
      WakeUp(NowTask());
      app_task_num = -1;
      page_kfree((int)kfifo, sizeof(struct FIFO8));
      page_kfree((int)mfifo, sizeof(struct FIFO8));
      page_kfree((int)kbuf, 4096);
      page_kfree((int)mbuf, 4096);
      page_free((int)memman, 4 * 1024);
      page_free((int)stack, 64 * 1024);
      page_free((int)p, finfo->size + 64 * 1024 + 512 * 1024);
      print("\n");
    } else if (finfo->size >= 36 && strncmp(fp->buf + 4, "Hari", 4) == 0)
    // Hari=C, C程序 代码拥有两个段的程序（代码段和数据段）
    {
      /*
              有些代码的逻辑和运行汇编语言程序是一样的，
              这里只注释新的代码
      */
      p = (char *)page_malloc(finfo->size);
      memcpy(p, fp->buf, finfo->size);
      fclose(fp);
      // hrb文件头（是的，就是那个《30天》的文件头，挺简单的）
      // hrb文件头的结构是：
      /**
       * 偏移地址       描述
       *  0x14         段大小
       *  0x0c         esp与数据存储的地址
       *  0x10         数据大小
       *  0x14         数据存储的地址
       */
      segsiz = *((int *)(p + 0x0000));
      esp = *((int *)(p + 0x000c));
      datsiz = *((int *)(p + 0x0010));
      dathrb = *((int *)(p + 0x0014));
      q = (char *)page_malloc(segsiz +
                              512 * 1024 * 4 * 2 * 2 * 2); //分配数据段的内存
      set_segmdesc(gdt + 3 + app_num * 2, finfo->size - 1, (int)p,
                   AR_CODE32_ER);
      set_segmdesc(gdt + 4 + app_num * 2,
                   segsiz - 1 + 512 * 1024 * 4 * 2 * 2 * 2, (int)q,
                   AR_DATA32_RW);
      for (i = 0; i < datsiz; i++) {
        q[esp + i] = p[dathrb + i]; //这里通过头数据拷贝数据段数据
      }
      io_cli();
      char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *kbuf = (char *)page_kmalloc(4096);
      char *mbuf = (char *)page_kmalloc(4096);
      char *memman = (char *)page_malloc(4 * 1024 * 4 * 2 * 2 * 2);
      // printk("kfifo:%08x,mfifo:%08x\n",kfifo,mfifo);
      // printk("kbuf:%08x,mbuf:%08x\n",kbuf,mbuf);
      // printk("p:%08x\n",p);
      // printk("q:%08x\n",q);
      struct TASK *this_task =
          AddTask(name, 2, ((3 + app_num * 2) * 8), 0x1b,
                  ((4 + app_num * 2) * 8), ((4 + app_num * 2) * 8), esp);
      this_task->cs_base = (int)p;
      this_task->ds_base = (int)q;
      this_task->alloc_addr = q + segsiz;
      this_task->alloc_size = 512 * 1024 * 4 * 2 * 2 * 2;
      this_task->memman = memman;
      stack = (unsigned char *)page_malloc(64 * 1024);
      this_task->esp0 = stack + 64 * 1024;
      this_task->directory = NowTask()->directory;
      this_task->line = NowTask()->line;
      strcpy(this_task->path, NowTask()->path);
      this_task->drive = NowTask()->drive;
      this_task->drive_number = NowTask()->drive_number;
      this_task->change_dict_times = NowTask()->change_dict_times;
      fifo8_init(kfifo, 4096, kbuf);
      fifo8_init(mfifo, 4096, mbuf);
      TaskSetFIFO(this_task, kfifo, mfifo);
      this_task->TTY = NowTask()->TTY;
      app_task_num = this_task->sel / 8 - 103;
      app_num++;
      SleepTaskFIFO(NowTask());
      io_sti();
      if (1) {
        io_cli();
        change_page_task_id(this_task->sel / 8 - 103, p, finfo->size);
        change_page_task_id(this_task->sel / 8 - 103, q,
                            segsiz + 512 * 1024 * 4 * 2 * 2 * 2);
        change_page_task_id(this_task->sel / 8 - 103, stack, 64 * 1024);
        change_page_task_id(this_task->sel / 8 - 103, memman, 4 * 1024);
        change_page_task_id(this_task->sel / 8 - 103, kfifo,
                            sizeof(struct FIFO8));
        change_page_task_id(this_task->sel / 8 - 103, mfifo,
                            sizeof(struct FIFO8));
        change_page_task_id(this_task->sel / 8 - 103, kbuf, 4096);
        change_page_task_id(this_task->sel / 8 - 103, mbuf, 4096);
        io_sti();
        WakeUp(NowTask());
        app_task_num = -1;
        print("\n");
        result = this_task;
        goto end;
      }

      // SubTask(GetTaskForName(name));
      WakeUp(NowTask());
      app_task_num = -1;
      // printk("kfifo:%08x,mfifo:%08x\n",kfifo,mfifo);
      // printk("kbuf:%08x,mbuf:%08x\n",kbuf,mbuf);
      // printk("p:%08x\n",p);
      // printk("q:%08x\n",q);
      page_kfree((int)kfifo, sizeof(struct FIFO8));
      page_kfree((int)mfifo, sizeof(struct FIFO8));
      page_kfree((int)kbuf, 4096);
      page_kfree((int)mbuf, 4096);
      page_free((int)memman, 4 * 1024);
      page_free((int)stack, 64 * 1024);
      page_free((int)p, finfo->size);
      page_free((int)q, segsiz + 512 * 1024 * 4 * 2 * 2 * 2);
      print("\n");
    } else if (elf32Validate(fp->buf)) {
      // printk("----------ProGram Running Malloc Info-----------");
      p = (char *)page_malloc(finfo->size);
      memcpy(p, fp->buf, finfo->size);
      fclose(fp);
      Elf32_Ehdr *ehdr = (Elf32_Ehdr *)p;
      segsiz = finfo->size;
      q = (char *)page_malloc(segsiz + 512 * 1024 * 4 +
                              512 * 1024); //分配数据段的内存
      set_segmdesc(gdt + 3 + app_num * 2, finfo->size - 1, (int)p,
                   AR_CODE32_ER);
      set_segmdesc(gdt + 4 + app_num * 2,
                   segsiz - 1 + 512 * 1024 * 4 + 512 * 1024, (int)q,
                   AR_DATA32_RW);
      // printf("[log]p(start)=%08x p(end)=%08x\n", p, p + finfo->size);
      // printf("[log]q(start)=%08x q(end)=%08x\n", q,
      //       q + segsiz + 512 * 1024 + 512 * 1024);
      elf32LoadData(ehdr, q);
      io_cli();
      char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *kbuf = (char *)page_kmalloc(4096);
      char *mbuf = (char *)page_kmalloc(4096);
      char *memman = (char *)page_malloc(4 * 1024 * 4);
      // printk("kfifo:%08x,mfifo:%08x\n",kfifo,mfifo);
      // printk("kbuf:%08x,mbuf:%08x\n",kbuf,mbuf);
      // printk("p:%08x\n",p);
      // printk("q:%08x\n",q);
      // printf("%08x\n", memman);
      struct TASK *this_task =
          AddTask(name, 2, ((3 + app_num * 2) * 8), ehdr->e_entry,
                  ((4 + app_num * 2) * 8), ((4 + app_num * 2) * 8),
                  segsiz + 512 * 1024 * 4 + 512 * 1024);
      this_task->cs_base = (int)p;
      this_task->ds_base = (int)q;
      this_task->alloc_addr = q + segsiz;
      this_task->alloc_size = 512 * 1024 * 4;
      this_task->memman = memman;
      stack = (unsigned char *)page_malloc(64 * 1024);
      this_task->esp0 = stack + 64 * 1024;
      this_task->directory = NowTask()->directory;
      strcpy(this_task->path, NowTask()->path);
      this_task->drive = NowTask()->drive;
      this_task->drive_number = NowTask()->drive_number;
      this_task->change_dict_times = NowTask()->change_dict_times;
      fifo8_init(kfifo, 4096, kbuf);
      fifo8_init(mfifo, 4096, mbuf);
      TaskSetFIFO(this_task, kfifo, mfifo);
      this_task->TTY = NowTask()->TTY;
      app_task_num = this_task->sel / 8 - 103;
      app_num++;
      SleepTaskFIFO(NowTask());
      io_sti();
      if (1) {
        io_cli();
        change_page_task_id(this_task->sel / 8 - 103, p, finfo->size);
        change_page_task_id(this_task->sel / 8 - 103, q, segsiz + 512 * 1024);
        change_page_task_id(this_task->sel / 8 - 103, stack, 64 * 1024);
        change_page_task_id(this_task->sel / 8 - 103, memman, 4 * 1024);
        change_page_task_id(this_task->sel / 8 - 103, kfifo,
                            sizeof(struct FIFO8));
        change_page_task_id(this_task->sel / 8 - 103, mfifo,
                            sizeof(struct FIFO8));
        change_page_task_id(this_task->sel / 8 - 103, kbuf, 4096);
        change_page_task_id(this_task->sel / 8 - 103, mbuf, 4096);
        io_sti();
        WakeUp(NowTask());
        app_task_num = -1;
        print("\n");
        result = this_task;
        goto end;
      }

      // SubTask(GetTaskForName(name));
      WakeUp(NowTask());
      app_task_num = -1;
      // printk("kfifo:%08x,mfifo:%08x\n",kfifo,mfifo);
      // printk("kbuf:%08x,mbuf:%08x\n",kbuf,mbuf);
      // printk("p:%08x\n",p);
      // printk("q:%08x\n",q);
      page_kfree((int)kfifo, sizeof(struct FIFO8));
      page_kfree((int)mfifo, sizeof(struct FIFO8));
      page_kfree((int)kbuf, 4096);
      page_kfree((int)mbuf, 4096);
      page_free((int)memman, 4 * 1024);
      page_free((int)stack, 64 * 1024);
      page_free((int)p, finfo->size);
      page_free((int)q, segsiz + 512 * 1024 + 512 * 1024);
      print("\n");
      // printk("----------ProGram Running Malloc Info
      // End-----------\n");
    } else {
      // 未知的文件类型
      print("Isn't Powerint DOS 386 Execute File.\n\n");
    }
  end:
    page_free((int)name, 300); //将name字符指针所占用的内存释放
    return result;
  }
  page_free((int)name, 300); //将name字符指针所占用的内存释放
  return NULL;               //找不到文件
}

int cmd_app(char *cmdline) {
  struct FILEINFO *finfo;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
  char *name, *p, *q,
      *alloc; // name:文件名，p:代码段，q:数据段，alloc:分配的内存
  unsigned char *stack; // 用于应用程序系统调用及硬件中断的堆栈
  struct TASK *app_task; //任务
  int i, segsiz, datsiz, dathrb, esp;
  name = (char *)page_malloc(300); //分配300字节内存
  clean(name, 300); //清空内存(为了避免脏数据，这里全部填充为0)
  for (i = 0; i < strlen(cmdline); i++) {
    if (cmdline[i] == ' ') {
      //如果碰到空格（参数部分），就说明文件名的部分结束了，这时候我们直接跳出循环即可
      break;
    }
    name[i] = cmdline[i];
  }
  name[i] =
      0; //置字符串结束符0
         //虽然前面已经全部填充为0了，这里为了保险，还是重新加上字符串结束符（'\0'）
  finfo = Get_File_Address(
      name); //获取文件所在的地址，保存为FILEINFO数据结构，以便我们获取信息
  if (finfo == 0) {
    finfo = Path_Find_File(name, Path_Addr);
    Path_Find_FileName(name, name, Path_Addr);
  }
  if (finfo == 0) //没找到这个文件
  {
    //加上后缀再试一遍
    name = strcat(name, ".BIN");
    finfo = Get_File_Address(name);
  }
  if (finfo == 0) {
    finfo = Path_Find_File(name, Path_Addr);
    Path_Find_FileName(name, name, Path_Addr);
  }
  if (finfo != 0) {
    if (stricmp(".BIN", &name[strlen(name) - 4]) != 0) {
      page_free((void *)name, 300);
      return 0;
    }
    // 代码段的物理内存必须是连续的
    FILE *fp = fopen(name, "r");
    if (fp->buf[0] ==
        'A') // A=ASM,这是汇编语言编写的程序（不分代码段和数据段，所以我们将他们两个分开，以便我们编写代码）
    {
      p = (char *)page_malloc(
          finfo->size + 64 * 1024 +
          512 *
              1024); //分配程序的代码段（由于这里是汇编语言的程序，所以在代码段后面要加上512KB的待分配内存）
      memcpy(
          p, fp->buf,
          finfo
              ->size); // 将文件拷贝到代码段中（由于是只有一个段的程序，所以直接无脑拷贝即可）
      fclose(fp);
      set_segmdesc(gdt + 3 + app_num * 2, finfo->size - 1, (int)p,
                   AR_CODE32_ER); //设置他们的段描述符
      set_segmdesc(gdt + 4 + app_num * 2,
                   finfo->size - 1 + 64 * 1024 + 512 * 1024, (int)p,
                   AR_DATA32_RW);
      io_cli();
      struct TASK *this_task =
          AddTask(name, 2, ((3 + app_num * 2) * 8), 1, ((4 + app_num * 2) * 8),
                  ((4 + app_num * 2) * 8), finfo->size + 64 * 1024);
      this_task->cs_base = p;
      this_task->ds_base = p;
      this_task->alloc_addr = p + finfo->size + 64 * 1024;
      this_task->alloc_size = 512 * 1024;
      stack = (unsigned char *)page_malloc(64 * 1024);
      this_task->esp0 = stack + 64 * 1024;
      this_task->directory = NowTask()->directory;
      this_task->line = NowTask()->line;
      strcpy(this_task->path, NowTask()->path);
      this_task->drive = NowTask()->drive;
      this_task->drive_number = NowTask()->drive_number;
      this_task->change_dict_times = NowTask()->change_dict_times;
      char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *kbuf = (char *)page_kmalloc(4096);
      char *mbuf = (char *)page_kmalloc(4096);
      char *memman = (char *)page_malloc(4 * 1024);
      this_task->memman = memman;
      fifo8_init(kfifo, 4096, kbuf);
      fifo8_init(mfifo, 4096, mbuf);
      TaskSetFIFO(this_task, kfifo, mfifo);
      this_task->TTY = NowTask()->TTY;
      this_task->app = 1;
      app_task_num = this_task->sel / 8 - 103;
      app_num++;
      this_task->forever = 0;
      SleepTaskFIFO(NowTask());
      io_sti();
      while (GetTaskForName(name) != 0) {
        if (this_task->forever == 1) {
          io_cli();
          change_page_task_id(this_task->sel / 8 - 103, p,
                              finfo->size + 64 * 1024 + 512 * 1024);
          change_page_task_id(this_task->sel / 8 - 103, stack, 64 * 1024);
          change_page_task_id(this_task->sel / 8 - 103, memman, 4 * 1024);
          change_page_task_id(this_task->sel / 8 - 103, kfifo,
                              sizeof(struct FIFO8));
          change_page_task_id(this_task->sel / 8 - 103, mfifo,
                              sizeof(struct FIFO8));
          change_page_task_id(this_task->sel / 8 - 103, kbuf, 4096);
          change_page_task_id(this_task->sel / 8 - 103, mbuf, 4096);
          this_task->app = 0;
          io_sti();
          WakeUp(NowTask());
          app_task_num = -1;
          print("\n");
          goto end;
        }
      }
      // task_sr1会帮我们结束程序
      // SubTask(GetTaskForName(name)); //程序退出咯
      WakeUp(NowTask());
      app_task_num = -1;
      page_kfree((int)kfifo, sizeof(struct FIFO8));
      page_kfree((int)mfifo, sizeof(struct FIFO8));
      page_kfree((int)kbuf, 4096);
      page_kfree((int)mbuf, 4096);
      page_free((int)memman, 4 * 1024);
      page_free((int)stack, 64 * 1024);
      page_free((int)p, finfo->size + 64 * 1024 + 512 * 1024);
      print("\n");
    } else if (finfo->size >= 36 && strncmp(fp->buf + 4, "Hari", 4) == 0)
    // Hari=C, C程序 代码拥有两个段的程序（代码段和数据段）
    {
      /*
              有些代码的逻辑和运行汇编语言程序是一样的，
              这里只注释新的代码
      */

      int now = NowTask()->level;
      p = (char *)page_malloc(finfo->size);
      memcpy(p, fp->buf, finfo->size);
      fclose(fp);
      // hrb文件头（是的，就是那个《30天》的文件头，挺简单的）
      // hrb文件头的结构是：
      /**
       * 偏移地址       描述
       *  0x14         段大小
       *  0x0c         esp与数据存储的地址
       *  0x10         数据大小
       *  0x14         数据存储的地址
       */
      segsiz = *((int *)(p + 0x0000));
      esp = *((int *)(p + 0x000c));
      datsiz = *((int *)(p + 0x0010));
      dathrb = *((int *)(p + 0x0014));
      q = (char *)page_malloc(segsiz +
                              512 * 1024 * 4 * 2 * 2 * 2); //分配数据段的内存
      set_segmdesc(gdt + 3 + app_num * 2, finfo->size - 1, (int)p,
                   AR_CODE32_ER);
      set_segmdesc(gdt + 4 + app_num * 2,
                   segsiz - 1 + 512 * 1024 * 4 * 2 * 2 * 2, (int)q,
                   AR_DATA32_RW);
      for (i = 0; i < datsiz; i++) {
        q[esp + i] = p[dathrb + i]; //这里通过头数据拷贝数据段数据
      }
      //   ChangeLevel(NowTask(),3);
      io_cli();
      char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *kbuf = (char *)page_kmalloc(4096);
      char *mbuf = (char *)page_kmalloc(4096);
      char *memman = (char *)page_malloc(4 * 1024 * 4 * 2 * 2 * 2);
      struct TASK *this_task =
          AddTask(name, 2, ((3 + app_num * 2) * 8), 0x1b,
                  ((4 + app_num * 2) * 8), ((4 + app_num * 2) * 8), esp);
      this_task->cs_base = (int)p;
      this_task->ds_base = (int)q;
      this_task->alloc_addr = q + segsiz;
      this_task->alloc_size = 512 * 1024 * 4 * 2 * 2 * 2;
      this_task->memman = memman;
      this_task->app = 1;
      stack = (unsigned char *)page_malloc(64 * 1024);
      this_task->esp0 = stack + 64 * 1024;
      this_task->directory = NowTask()->directory;
      this_task->line = NowTask()->line;
      strcpy(this_task->path, NowTask()->path);
      this_task->drive = NowTask()->drive;
      this_task->drive_number = NowTask()->drive_number;
      this_task->change_dict_times = NowTask()->change_dict_times;
      fifo8_init(kfifo, 4096, kbuf);
      fifo8_init(mfifo, 4096, mbuf);
      TaskSetFIFO(this_task, kfifo, mfifo);
      this_task->TTY = NowTask()->TTY;
      app_task_num = this_task->sel / 8 - 103;
      app_num++;
      this_task->forever = 0;
      SleepTaskFIFO(NowTask());
      io_sti();
      while (GetTaskForName(name) != 0 && this_task->running) {
        // printk("App(%s):Run. --> %08x\n",name,GetTaskForName(name));
        if (this_task->forever == 1) {
          io_cli();
          change_page_task_id(this_task->sel / 8 - 103, p, finfo->size);
          change_page_task_id(this_task->sel / 8 - 103, q,
                              segsiz + 512 * 1024 * 4 * 2 * 2 * 2);
          change_page_task_id(this_task->sel / 8 - 103, stack, 64 * 1024);
          change_page_task_id(this_task->sel / 8 - 103, memman,
                              4 * 1024 * 4 * 2 * 2 * 2);
          change_page_task_id(this_task->sel / 8 - 103, kfifo,
                              sizeof(struct FIFO8));
          change_page_task_id(this_task->sel / 8 - 103, mfifo,
                              sizeof(struct FIFO8));
          change_page_task_id(this_task->sel / 8 - 103, kbuf, 4096);
          change_page_task_id(this_task->sel / 8 - 103, mbuf, 4096);
          //   ChangeLevel(NowTask(),now);
          //   ChangeLevel(this_task,2);
          this_task->app = 0;
          io_sti();
          WakeUp(NowTask());
          app_task_num = -1;
          print("\n");
          goto end;
        }
      }
      // printf("Run Done!!!!!!!!!!!\n");
      // ChangeLevel(NowTask(),now);
      //  SubTask(GetTaskForName(name));
      WakeUp(NowTask());
      app_task_num = -1;
      // printk("kfifo:%08x,mfifo:%08x\n",kfifo,mfifo);
      // printk("kbuf:%08x,mbuf:%08x\n",kbuf,mbuf);
      // printk("p:%08x\n",p);
      // printk("q:%08x\n",q);
      page_kfree((int)kfifo, sizeof(struct FIFO8));
      page_kfree((int)mfifo, sizeof(struct FIFO8));
      page_kfree((int)kbuf, 4096);
      page_kfree((int)mbuf, 4096);
      page_free((int)memman, 4 * 1024 * 4 * 2 * 2 * 2);
      page_free((int)stack, 64 * 1024);
      page_free((int)p, finfo->size);
      page_free((int)q, segsiz + 512 * 1024 * 4 * 2 * 2 * 2);
      print("\n");
    } else if (elf32Validate(fp->buf)) {
      // printk("----------ProGram Running Malloc Info-----------");
      p = (char *)page_malloc(finfo->size);
      memcpy(p, fp->buf, finfo->size);
      fclose(fp);
      Elf32_Ehdr *ehdr = (Elf32_Ehdr *)p;
      segsiz = finfo->size;
      q = (char *)page_malloc(segsiz + 512 * 1024 * 4 +
                              512 * 1024); //分配数据段的内存
      set_segmdesc(gdt + 3 + app_num * 2, finfo->size - 1, (int)p,
                   AR_CODE32_ER);
      set_segmdesc(gdt + 4 + app_num * 2,
                   segsiz - 1 + 512 * 1024 * 4 + 512 * 1024, (int)q,
                   AR_DATA32_RW);
      // printf("[log]p(start)=%08x p(end)=%08x\n", p, p + finfo->size);
      // printf("[log]q(start)=%08x q(end)=%08x\n", q,
      //        q + segsiz + 512 * 1024 + 512 * 1024);
      elf32LoadData(ehdr, q);
      io_cli();
      char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
      char *kbuf = (char *)page_kmalloc(4096);
      char *mbuf = (char *)page_kmalloc(4096);
      char *memman = (char *)page_malloc(4 * 1024 * 4);
      // printk("kfifo:%08x,mfifo:%08x\n",kfifo,mfifo);
      // printk("kbuf:%08x,mbuf:%08x\n",kbuf,mbuf);
      // printk("p:%08x\n",p);
      // printk("q:%08x\n",q);
      // printf("%08x\n", memman);
      struct TASK *this_task =
          AddTask(name, 2, ((3 + app_num * 2) * 8), ehdr->e_entry,
                  ((4 + app_num * 2) * 8), ((4 + app_num * 2) * 8),
                  segsiz + 512 * 1024 * 4 + 512 * 1024);
      this_task->cs_base = (int)p;
      this_task->ds_base = (int)q;
      this_task->alloc_addr = q + segsiz;
      this_task->alloc_size = 512 * 1024 * 4;
      this_task->memman = memman;
      this_task->app = 1;
      stack = (unsigned char *)page_malloc(64 * 1024);
      this_task->esp0 = stack + 64 * 1024;
      this_task->directory = NowTask()->directory;
      strcpy(this_task->path, NowTask()->path);
      this_task->drive = NowTask()->drive;
      this_task->drive_number = NowTask()->drive_number;
      this_task->change_dict_times = NowTask()->change_dict_times;
      fifo8_init(kfifo, 4096, kbuf);
      fifo8_init(mfifo, 4096, mbuf);
      TaskSetFIFO(this_task, kfifo, mfifo);
      this_task->TTY = NowTask()->TTY;
      app_task_num = this_task->sel / 8 - 103;
      app_num++;
      this_task->forever = 0;
      SleepTaskFIFO(NowTask());
      io_sti();
      while (GetTaskForName(name) != 0) {
        if (this_task->forever == 1) {
          io_cli();
          change_page_task_id(this_task->sel / 8 - 103, p, finfo->size);
          change_page_task_id(this_task->sel / 8 - 103, q, segsiz + 512 * 1024);
          change_page_task_id(this_task->sel / 8 - 103, stack, 64 * 1024);
          change_page_task_id(this_task->sel / 8 - 103, memman, 4 * 1024);
          change_page_task_id(this_task->sel / 8 - 103, kfifo,
                              sizeof(struct FIFO8));
          change_page_task_id(this_task->sel / 8 - 103, mfifo,
                              sizeof(struct FIFO8));
          change_page_task_id(this_task->sel / 8 - 103, kbuf, 4096);
          change_page_task_id(this_task->sel / 8 - 103, mbuf, 4096);
          io_sti();
          WakeUp(NowTask());
          app_task_num = -1;
          print("\n");
          goto end;
        }
      }
      // SubTask(GetTaskForName(name));
      WakeUp(NowTask());
      app_task_num = -1;
      // printk("kfifo:%08x,mfifo:%08x\n",kfifo,mfifo);
      // printk("kbuf:%08x,mbuf:%08x\n",kbuf,mbuf);
      // printk("p:%08x\n",p);
      // printk("q:%08x\n",q);
      page_kfree((int)kfifo, sizeof(struct FIFO8));
      page_kfree((int)mfifo, sizeof(struct FIFO8));
      page_kfree((int)kbuf, 4096);
      page_kfree((int)mbuf, 4096);
      page_free((int)memman, 4 * 1024);
      page_free((int)stack, 64 * 1024);
      page_free((int)p, finfo->size);
      page_free((int)q, segsiz + 512 * 1024 + 512 * 1024);
      print("\n");
      // printk("----------ProGram Running Malloc Info End-----------\n");
    } else {
      // 未知的文件类型
      print("Isn't Powerint DOS 386 Execute File.\n\n");
    }
  end:
    page_free((int)name, 300); //将name字符指针所占用的内存释放
    return 1;
  }
  page_free((int)name, 300); //将name字符指针所占用的内存释放
  return 0;                  //找不到文件
}
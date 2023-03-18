// Powerint DOS 386
// Copyright (C) 2021-2022 zhouzhihao & min0911
#include <dos.h>
uint32_t running_mode = POWERINTDOS;  // 运行模式
uint32_t Path_Addr;
struct DRIVE_CTL drive_ctl;
unsigned char *font, *ascfont, *hzkfont;
unsigned char* IVT;
void shell(void) {
  ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);
  init_networkCTL();
  init_network();
  init_card();
  init_palette();
  struct TASK* task = NowTask();
  drive_ctl.drives[task->drive_number].SectorBytes =
      *(unsigned short*)(0x7c00 + BPB_BytsPerSec);
  drive_ctl.drives[task->drive_number].RootMaxFiles =
      *(unsigned short*)(0x7c00 + BPB_RootEntCnt);
  drive_ctl.drives[task->drive_number].ClustnoBytes =
      drive_ctl.drives[task->drive_number].SectorBytes *
      *(unsigned char*)(0x7c00 + BPB_SecPerClus);
  drive_ctl.drives[task->drive_number].RootDictAddress =
      (*(unsigned char*)(0x7c00 + BPB_NumFATs) *
           *(unsigned short*)(0x7c00 + BPB_FATSz16) +
       *(unsigned short*)(0x7c00 + BPB_RsvdSecCnt)) *
      drive_ctl.drives[task->drive_number].SectorBytes;
  drive_ctl.drives[task->drive_number].FileDataAddress =
      drive_ctl.drives[task->drive_number].RootDictAddress +
      drive_ctl.drives[task->drive_number].RootMaxFiles * 32;
  if (*(unsigned short*)(0x7c00 + BPB_TotSec16) != 0) {
    drive_ctl.drives[task->drive_number].imgTotalSize =
        *(unsigned short*)(0x7c00 + BPB_TotSec16) *
        drive_ctl.drives[task->drive_number].SectorBytes;
  } else {
    drive_ctl.drives[task->drive_number].imgTotalSize =
        *(unsigned int*)(0x7c00 + BPB_TotSec32) *
        drive_ctl.drives[task->drive_number].SectorBytes;
  }
  drive_ctl.drives[task->drive_number].Fat1Address =
      *(unsigned short*)(0x7c00 + BPB_RsvdSecCnt) *
      drive_ctl.drives[task->drive_number].SectorBytes;
  drive_ctl.drives[task->drive_number].Fat2Address =
      drive_ctl.drives[task->drive_number].Fat1Address +
      *(unsigned short*)(0x7c00 + BPB_FATSz16) *
          drive_ctl.drives[task->drive_number].SectorBytes;
  uint32_t sec = drive_ctl.drives[task->drive_number].FileDataAddress /
                 drive_ctl.drives[task->drive_number].SectorBytes;
  drive_ctl.drives[task->drive_number].ADR_DISKIMG = (unsigned int)malloc(
      drive_ctl.drives[task->drive_number].FileDataAddress);
  Disk_Read(0, sec, (void*)drive_ctl.drives[task->drive_number].ADR_DISKIMG,
            task->drive);
  drive_ctl.drives[task->drive_number].fat = malloc(3072 * sizeof(int));
  drive_ctl.drives[task->drive_number].FatClustnoFlags =
      malloc(3072 * sizeof(char));
  read_fat((unsigned char*)(drive_ctl.drives[task->drive_number].ADR_DISKIMG +
                            drive_ctl.drives[task->drive_number].Fat1Address),
           drive_ctl.drives[task->drive_number].fat,
           drive_ctl.drives[task->drive_number].FatClustnoFlags);
  drive_ctl.drives[task->drive_number].root_directory =
      (struct FILEINFO*)malloc(
          drive_ctl.drives[task->drive_number].RootMaxFiles * 32);
  memcpy((void*)drive_ctl.drives[task->drive_number].root_directory,
         (void*)drive_ctl.drives[task->drive_number].ADR_DISKIMG +
             drive_ctl.drives[task->drive_number].RootDictAddress,
         drive_ctl.drives[task->drive_number].RootMaxFiles * 32);
  task->directory = drive_ctl.drives[task->drive_number].root_directory;
  drive_ctl.drives[task->drive_number].directory_list = (struct LIST*)NewList();
  drive_ctl.drives[task->drive_number].directory_clustno_list =
      (struct LIST*)NewList();
  struct FILEINFO* finfo = task->directory;
  for (int i = 0; i != drive_ctl.drives[task->drive_number].RootMaxFiles; i++) {
    if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
      AddVal(finfo[i].clustno,
             (struct List*)drive_ctl.drives[task->drive_number]
                 .directory_clustno_list);
      void* directory_alloc =
          page_malloc(drive_ctl.drives[task->drive_number].ClustnoBytes);
      uint32_t sec1 = (drive_ctl.drives[task->drive_number].FileDataAddress +
                       (finfo[i].clustno - 2) *
                           drive_ctl.drives[task->drive_number].ClustnoBytes) /
                      drive_ctl.drives[task->drive_number].SectorBytes;
      Disk_Read(sec1,
                drive_ctl.drives[task->drive_number].ClustnoBytes /
                    drive_ctl.drives[task->drive_number].SectorBytes,
                directory_alloc, task->drive);
      AddVal(
          (int)directory_alloc,
          (struct List*)(drive_ctl.drives[task->drive_number].directory_list));
    }
    if (finfo[i].name[0] == 0) {
      break;
    }
  }
  for (int i = 1;
       FindForCount(
           i,
           (struct List*)drive_ctl.drives[task->drive_number].directory_list) !=
       NULL;
       i++) {
    struct List* list = FindForCount(
        i, (struct List*)drive_ctl.drives[task->drive_number].directory_list);
    finfo = (struct FILEINFO*)list->val;
    for (int j = 0; j != drive_ctl.drives[task->drive_number].ClustnoBytes / 32;
         j++) {
      if (finfo[j].type == 0x10 && finfo[j].name[0] != 0xe5 &&
          strncmp(".", (char*)finfo[j].name, 1) != 0 &&
          strncmp("..", (char*)finfo[j].name, 2) != 0) {
        AddVal(finfo[j].clustno,
               (struct List*)drive_ctl.drives[task->drive_number]
                   .directory_clustno_list);
        void* directory_alloc =
            page_malloc(drive_ctl.drives[task->drive_number].ClustnoBytes);
        uint32_t sec1 =
            (drive_ctl.drives[task->drive_number].FileDataAddress +
             (finfo[j].clustno - 2) *
                 drive_ctl.drives[task->drive_number].ClustnoBytes) /
            drive_ctl.drives[task->drive_number].SectorBytes;
        Disk_Read(sec1,
                  drive_ctl.drives[task->drive_number].ClustnoBytes /
                      drive_ctl.drives[task->drive_number].SectorBytes,
                  directory_alloc, task->drive);
        AddVal(
            (int)directory_alloc,
            (struct List*)drive_ctl.drives[task->drive_number].directory_list);
      }
      if (finfo[j].name[0] == 0) {
        break;
      }
    }
  }
  /*到这里 系统的初始化才真正结束*/
  font = (unsigned char*)"other\\font.bin";
  FILE* fp = fopen("other\\font.bin", "r");
  ascfont = fp->buf;
  fp = fopen("other\\hzk16", "r");
  hzkfont = fp->buf;
  fp = fopen("path.sys", "r");
  Path_Addr = (uint32_t)fp->buf;
  clear();
  printf("Please choose your boot mode:\n");
  printf("1. TextMode 80 * 25\n");
  printf("2. HighTextMode 128 * 48\n");
  printf("3. GraphicMode 1024 * 768\n\n");
  printf("Input:");
  unsigned char choice;
  for (;;) {
    choice = getch();
    if (choice == '1') {
      running_mode = POWERINTDOS;
      clear();
      break;
    } else if (choice == '2') {
      running_mode = HIGHTEXTMODE;
      SwitchToHighTextMode();
      break;
    } else if (choice == '3') {
      running_mode = POWERDESKTOP;
      graphic();
    }
  }
  if (fsz("AUTOEXEC.BAT") == -1) {
    printf("Boot Warning:No AUTOEXEC.BAT in Drive %c\n", task->drive);
  } else {
    run_bat("AUTOEXEC.BAT");
  }
  extern struct tty* tty_default;
  tty_set(task, tty_default);
  shell_handler();
}
void shell_handler() {
  struct TASK* task = NowTask();
  task->line = (char*)page_kmalloc(1024);
  strcpy(task->path, "");
  while (1) {
    printchar(task->drive);
    print(":\\");
    print(task->path);
    print(">");
    clean(task->line, 1024);
    input(task->line, 1024);
    command_run(task->line);
  }
}
struct tty* now_tty() {
  extern struct List* tty_list;
  struct tty* n;
  for (int j = 1; FindForCount(j, tty_list) != 0; j++) {
    n = (struct tty*)FindForCount(j, tty_list)->val;
    if ((now_tty_TextMode(n) && running_mode == POWERINTDOS) ||
        (now_tty_HighTextMode(n) && running_mode == HIGHTEXTMODE) ||
        (now_tty_GraphicMode(n) && running_mode == POWERDESKTOP)) {
      return n;
    }
  }
  return NULL;
}

void task_sr1() {
  // 提供安全结束进程
  extern int tasknum;
  while (1) {
  re:
    // printk("Wake UP.\n");
    if (Get_Running_Task_Num() == 1) {
      while (Get_Running_Task_Num() == 1) {
        // printk("1\n");
        asm volatile("hlt");
      }
    }
    for (int i = 1; i != tasknum + 1; i++) {
      struct TASK* task = GetTask(i);
      if (task->running == 0) {  // 进程没有运行
        __SubTask(task);
        goto re;
      }
    }
    SleepTask(NowTask());
  }
}
void com_input(char* ptr, int len) {
  int i;
  for (i = 0; i != len; i++) {
    char in = read_serial();
    if (in == '\r') {
      ptr[i] = 0;
      printk("\n");
      break;
    } else if (in == '\b') {
      if (i == 0) {
        i--;
        continue;
      }
      i--;
      ptr[i] = 0;
      i--;
      printk("\b");
      printk(" ");
      printk("\b");
      continue;
    }
    printk("%c", in);
    ptr[i] = in;
  }
}
void task_sr2() {
  for (;;) {
    printk("Debug> ");
    char buf[150];
    com_input(buf, 150);
    printk("Recved Command:%s\n", buf);
    if (strcmp("show_all", buf) == 0) {
      for (int i = 1; GetTask(i) != 0; i++) {
        printk("Task %s,CS:EIP=%04x:%08x Sleep=%d,%d lock=%d is_child=%d\n",
               GetTask(i)->name, GetTask(i)->last_cs,GetTask(i)->last_eip, GetTask(i)->sleep,
               GetTask(i)->fifosleep, GetTask(i)->lock, GetTask(i)->is_child);
      }
    } else {
      printk("Bad Command\n");
    }
    // printk("Task Running.\n");
  }
  // while (1) {
  //   if (IPCMessageStatus() != 0) {
  //     printk("Get Message.\n");
  //     int tid = NowTask()->IPC_header.from_tid[0];
  //     int length = IPCMessageLength(tid);
  //     int* data = page_malloc(length);
  //     GetIPCMessage(data, tid);
  //     printk("will set_mode %dx%dx%d\n",data[0],data[1],data[2]);
  //     int result = _set_mode(data[0],data[1],data[2]);
  //     printk("Set OK.\n");
  //     unsigned int *r = data[3];
  //     *r = result;
  //   }
  // }
}

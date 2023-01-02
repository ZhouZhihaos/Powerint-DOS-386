// Powerint DOS 386
// Copyright (C) 2021-2022 zhouzhihao & min0911
#include <dos.h>
typedef struct {
  int rw;
  int number;
  int lba;
  unsigned char* buffer;
  unsigned char* ok_flag;
} packet_header;
typedef struct {
  uint8_t res[2];
  uint16_t sectors_of_number;
  uint16_t off;
  uint16_t seg;
  uint64_t lba;
} __attribute__((packed)) packet;
int running_mode = POWERINTDOS;  // 运行模式
uint32_t Path_Addr;
struct DRIVE_CTL drive_ctl;
char* line;
char *font, *ascfont, *hzkfont;
void shell(void) {
  unsigned char buffer[512];
  ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);

  // for (int i = 0; i < 512; i++) {
  //   printf("%02x ", buffer[i]);
  // }
  // for (;;)
  //   ;
  init_palette();
  clear();
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
  drive_ctl.drives[task->drive_number].ADR_DISKIMG =
      malloc(drive_ctl.drives[task->drive_number].FileDataAddress);
  Disk_Read(0, sec, drive_ctl.drives[task->drive_number].ADR_DISKIMG,
            task->drive);
  drive_ctl.drives[task->drive_number].fat = malloc(3072 * sizeof(int));
  drive_ctl.drives[task->drive_number].FatClustnoFlags =
      malloc(3072 * sizeof(char));
  read_fat(drive_ctl.drives[task->drive_number].ADR_DISKIMG +
               drive_ctl.drives[task->drive_number].Fat1Address,
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
  drive_ctl.drives[task->drive_number].directory_list = NewList();
  drive_ctl.drives[task->drive_number].directory_clustno_list = NewList();
  struct FILEINFO* finfo = task->directory;
  for (int i = 0; i != drive_ctl.drives[task->drive_number].RootMaxFiles; i++) {
    if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
      AddVal(finfo[i].clustno,
             drive_ctl.drives[task->drive_number].directory_clustno_list);
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
      AddVal(directory_alloc,
             drive_ctl.drives[task->drive_number].directory_list);
    }
    if (finfo[i].name[0] == NULL) {
      break;
    }
  }
  for (int i = 1;
       FindForCount(i, drive_ctl.drives[task->drive_number].directory_list) !=
       NULL;
       i++) {
    struct List* list =
        FindForCount(i, drive_ctl.drives[task->drive_number].directory_list);
    finfo = (struct FILEINFO*)list->val;
    for (int j = 0; j != drive_ctl.drives[task->drive_number].ClustnoBytes / 32;
         j++) {
      if (finfo[j].type == 0x10 && finfo[j].name[0] != 0xe5 &&
          strncmp(".", finfo[j].name, 1) != 0 &&
          strncmp("..", finfo[j].name, 2) != 0) {
        AddVal(finfo[j].clustno,
               drive_ctl.drives[task->drive_number].directory_clustno_list);
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
        AddVal(directory_alloc,
               drive_ctl.drives[task->drive_number].directory_list);
      }
      if (finfo[j].name[0] == NULL) {
        break;
      }
    }
  }
  /*到这里 系统的初始化才真正结束*/
  font = "other\\font.bin";
  FILE* fp = fopen("other\\font.bin", "r");
  ascfont = fp->buf;
  fp = fopen("other\\hzk16", "r");
  hzkfont = fp->buf;
  fp = fopen("path.sys", "r");
  Path_Addr = fp->buf;
  strcpy(task->path, "");
  line = (char*)page_kmalloc(1024);
  // Set_Font(font);
  if (fsz("AUTOEXEC.BAT") == -1) {
    printf("Boot Warning:No AUTOEXEC.BAT in Drive %c\n", task->drive);
  } else {
    run_bat("AUTOEXEC.BAT");
  }
  // 如果执行到这里
  // 说明没有进入Power Desktop
  // clear();
  // if (fsz("USER\\USER.INI") == -1 || fsz("USER\\PASSWORD.INI") == -1) {
  //   if (fsz("CREUS.BIN") != -1) {
  //     cmd_app("CREUS.BIN");
  //     command_run("REBOOT");
  //   }
  // } else {
  //   struct FILEINFO* user_finfo = Get_File_Address("USER\\USER.INI");
  //   struct FILEINFO* password_finfo =
  //       Get_File_Address("USER\\PASSWORD.INI");
  //   char* user = malloc(fsz("USER\\USER.INI"));
  //   char* password = malloc(fsz("USER\\PASSWORD.INI"));
  //   file_loadfile(user_finfo->clustno, user_finfo->size, user,
  //                 drive_ctl.drives[NowTask()->drive_number].fat, 0);
  //   file_loadfile(password_finfo->clustno, password_finfo->size, password,
  //                 drive_ctl.drives[NowTask()->drive_number].fat, 0);
  //   char inpass[32];
  //   char inusr[32];
  //   printf("Login:\n");
  // rei:
  //   printf("User:");

  //   input(inusr, 32);
  //   printf("Password:");

  //   input(inpass, 32);
  //   if (strcmp(user, inusr) != 0 || strcmp(password, inpass) != 0) {
  //     sleep(100);
  //     printf("User or Password Error\n");
  //     clean(inusr, 32);
  //     clean(inpass, 32);
  //     goto rei;
  //   }
  //   free(user);
  //   free(password);
  // }
  // printf("\nLogin successful done!\n\n");

pass:
  init_palette();
  while (1) {
    printchar(task->drive);
    print(":\\");
    print(task->path);
    print(">");
    clean(line, 1024);
    input(line, 1024);
    command_run(line);
  }
}

void task_sr1() {
  // 提供安全结束进程
  extern int tasknum;
  while (1) {
  re:
    for (int i = 1; i != tasknum + 1; i++) {
      struct TASK* task = GetTask(i);
      if (task->running == 0) {  // 进程没有运行
        // printf("Has no running task\n");
        __SubTask(task);
        goto re;
      }
    }
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
  for (;;)
    ;
  // while (1) {
  //   if (IPCMessageStatus() != 0) {
  //     int tid = NowTask()->IPC_header.from_tid[0];
  //     int length = IPCMessageLength(tid);
  //     printk("Task_sr2: Get IPC Message. From:Task ID %d\n", tid);
  //     void* data = page_malloc(length);
  //     GetIPCMessage(data, tid);
  //     packet_header* h = (packet_header*)data;
  //     Maskirq(12);
  //     io_cli();
  //     if (h->rw == 0) {
  //       // printk("Read.\n");
  //       for (int a = 0; a < h->number; a++) {
  //         io_cli();
  //         packet p;
  //         p.res[0] = 16;
  //         p.res[1] = 0;
  //         p.lba = h->lba;
  //         p.off = 0;
  //         p.seg = 0x800;  // 0x8000物理地址 （0x800:0）
  //         p.sectors_of_number = 1;
  //         memcpy(0x7e00, &p, sizeof(packet));
  //         regs16_t r;
  //         r.ax = 0x4200;
  //         r.ds = 0x7e0;
  //         r.si = 0x0;
  //         r.dx = 0x0080;
  //         INT(0x13, &r);
  //         memcpy(h->buffer + a * 512, 0x8000, 512);
  //       }
  //       // printk("(unsigned char*)(h->ok_flag) = %08x\n", (unsigned char
  //       // *)(h->ok_flag));
  //       *(unsigned char*)(h->ok_flag) = 0xff;
  //     } else {
  //       for (int a = 0; a < h->number; a++) {
  //         io_cli();
  //         memcpy(h->buffer + a * 512, 0x8000, 512);
  //         packet p;
  //         p.res[0] = 16;
  //         p.res[1] = 0;
  //         p.lba = h->lba;
  //         p.off = 0;
  //         p.seg = 0x800;  // 0x8000物理地址 （0x800:0）
  //         p.sectors_of_number = 1;
  //         memcpy(0x7e00, &p, sizeof(packet));
  //         regs16_t r;
  //         r.ax = 0x4300;
  //         r.ds = 0x7e0;
  //         r.si = 0x0;
  //         r.dx = 0x0080;
  //         INT(0x13, &r);
  //       }
  //       *(unsigned char*)(h->ok_flag) = 0xff;
  //     }
  //     io_sti();
  //     ClearMaskIrq(12);
  //   } else {
  //   }
  // }
}

#include <dosldr.h>
struct DRIVE_CTL drive_ctl;
struct TASK MainTask;
void *malloc(int size);
void *memcpy(void *s, const void *ct, size_t n);
void DOSLDR_MAIN() {
  struct MEMMAN *memman = MEMMAN_ADDR;
  unsigned int memtotal;
  memtotal = memtest(0x00400000, 0xbfffffff);
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x00006c00);
  memman_free(memman, 0x00007e00, 0x00097200);
  memman_free(memman, 0x00600000, memtotal - 0x00600000);
  clear();
  init_gdtidt();
  init_pic();
  io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */
  init_floppy();
  bool flags_once = false;
  char default_drive;
  unsigned int default_drive_number;
  if (!flags_once) {
    if (*(unsigned char *)(0x7c00 + BS_DrvNum) >= 0x80) {
      default_drive_number =
          *(unsigned char *)(0x7c00 + BS_DrvNum) - 0x80 + 0x02;
    } else {
      default_drive_number = *(unsigned char *)(0x7c00 + BS_DrvNum);
    }
    default_drive = default_drive_number + 0x41;
    flags_once = true;
  }
  NowTask()->drive = default_drive;
  NowTask()->drive_number = default_drive_number;
  NowTask()->change_dict_times = 0;
  struct TASK *task = NowTask();
  drive_ctl.drives[task->drive_number].SectorBytes =
      *(unsigned short *)(0x7c00 + BPB_BytsPerSec);
  drive_ctl.drives[task->drive_number].RootMaxFiles =
      *(unsigned short *)(0x7c00 + BPB_RootEntCnt);
  drive_ctl.drives[task->drive_number].ClustnoBytes =
      drive_ctl.drives[task->drive_number].SectorBytes *
      *(unsigned char *)(0x7c00 + BPB_SecPerClus);
  drive_ctl.drives[task->drive_number].RootDictAddress =
      (*(unsigned char *)(0x7c00 + BPB_NumFATs) *
           *(unsigned short *)(0x7c00 + BPB_FATSz16) +
       *(unsigned short *)(0x7c00 + BPB_RsvdSecCnt)) *
      drive_ctl.drives[task->drive_number].SectorBytes;
  printf("RD = %08x\n", drive_ctl.drives[task->drive_number].RootDictAddress);
  drive_ctl.drives[task->drive_number].FileDataAddress =
      drive_ctl.drives[task->drive_number].RootDictAddress +
      drive_ctl.drives[task->drive_number].RootMaxFiles * 32;
  if (*(unsigned short *)(0x7c00 + BPB_TotSec16) != 0) {
    drive_ctl.drives[task->drive_number].imgTotalSize =
        *(unsigned short *)(0x7c00 + BPB_TotSec16) *
        drive_ctl.drives[task->drive_number].SectorBytes;
  } else {
    drive_ctl.drives[task->drive_number].imgTotalSize =
        *(unsigned int *)(0x7c00 + BPB_TotSec32) *
        drive_ctl.drives[task->drive_number].SectorBytes;
  }
  drive_ctl.drives[task->drive_number].Fat1Address =
      *(unsigned short *)(0x7c00 + BPB_RsvdSecCnt) *
      drive_ctl.drives[task->drive_number].SectorBytes;
  drive_ctl.drives[task->drive_number].Fat2Address =
      drive_ctl.drives[task->drive_number].Fat1Address +
      *(unsigned short *)(0x7c00 + BPB_FATSz16) *
          drive_ctl.drives[task->drive_number].SectorBytes;
  uint32_t sec = drive_ctl.drives[task->drive_number].FileDataAddress /
                 drive_ctl.drives[task->drive_number].SectorBytes;
  drive_ctl.drives[task->drive_number].ADR_DISKIMG =
      page_malloc(drive_ctl.drives[task->drive_number].FileDataAddress);
  Disk_Read(0, sec, drive_ctl.drives[task->drive_number].ADR_DISKIMG,
            task->drive);
  drive_ctl.drives[task->drive_number].fat = page_malloc(3072 * sizeof(int));
  drive_ctl.drives[task->drive_number].FatClustnoFlags =
      page_malloc(3072 * sizeof(char));
  read_fat(drive_ctl.drives[task->drive_number].ADR_DISKIMG +
               drive_ctl.drives[task->drive_number].Fat1Address,
           drive_ctl.drives[task->drive_number].fat,
           drive_ctl.drives[task->drive_number].FatClustnoFlags);
  drive_ctl.drives[task->drive_number].root_directory =
      (struct FILEINFO *)page_malloc(
          drive_ctl.drives[task->drive_number].RootMaxFiles * 32);
  memcpy((void *)drive_ctl.drives[task->drive_number].root_directory,
         (void *)drive_ctl.drives[task->drive_number].ADR_DISKIMG +
             drive_ctl.drives[task->drive_number].RootDictAddress,
         drive_ctl.drives[task->drive_number].RootMaxFiles * 32);
  task->directory = drive_ctl.drives[task->drive_number].root_directory;
  drive_ctl.drives[task->drive_number].directory_list = NewList();
  drive_ctl.drives[task->drive_number].directory_clustno_list = NewList();
  struct FILEINFO *finfo = task->directory;
  for (int i = 0; i != drive_ctl.drives[task->drive_number].RootMaxFiles; i++) {
    if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
      AddVal(finfo[i].clustno,
             drive_ctl.drives[task->drive_number].directory_clustno_list);
      void *directory_alloc =
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
    struct List *list =
        FindForCount(i, drive_ctl.drives[task->drive_number].directory_list);
    finfo = (struct FILEINFO *)list->val;
    for (int j = 0; j != drive_ctl.drives[task->drive_number].ClustnoBytes / 32;
         j++) {
      if (finfo[j].type == 0x10 && finfo[j].name[0] != 0xe5 &&
          strncmp(".", finfo[j].name, 1) != 0 &&
          strncmp("..", finfo[j].name, 2) != 0) {
        AddVal(finfo[j].clustno,
               drive_ctl.drives[task->drive_number].directory_clustno_list);
        void *directory_alloc =
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
  printf("DOSLDR 386 v0.1\n");
  printf("Copyright zhouzhihao & min0911 2022\n");
  printf("memtotal=%dMB\n", memtotal / 1024 / 1024);
  char path[15] = " :\\kernel.bin";
  path[0] = NowTask()->drive;
  printf("Load file:%s\n", path);
  FILE *fp = fopen(path, "rb");
  // printf("fp = %08x\n%d\n",fp, fp->size);
  unsigned char *s = 0x00280000;
  printf("Will load in %08x size = %08x\n", s, fsz(path));
  fread(s, fsz(path), 1, fp);
  fclose(fp);
  printf("Loading...\n");
  // printf("ESP:%08x\n",*(unsigned int *)(0x00280000 + 12));

  _IN();
  for (;;)
    ;
}
struct TASK *NowTask() {
  return &MainTask;
}
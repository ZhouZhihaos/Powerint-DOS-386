#include <dosldr.h>
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
  init_vdisk();
  init_vfs();
  init_floppy();
  Register_fat_fileSys();
  ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);
  bool flags_once = false;
  char default_drive;
  unsigned int default_drive_number;
  if (!flags_once) {
    if (memcmp((void *)"FAT12   ", (void *)0x7c00 + BS_FileSysType, 8) == 0 ||
        memcmp((void *)"FAT16   ", (void *)0x7c00 + BS_FileSysType, 8) == 0) {
      if (*(unsigned char *)(0x7c00 + BS_DrvNum) >= 0x80) {
        default_drive_number =
            *(unsigned char *)(0x7c00 + BS_DrvNum) - 0x80 + 0x02;
      } else {
        default_drive_number = *(unsigned char *)(0x7c00 + BS_DrvNum);
      }
    } else if (memcmp((void *)"FAT32   ",
                      (void *)0x7c00 + BPB_Fat32ExtByts + BS_FileSysType,
                      8) == 0) {
      if (*(unsigned char *)(0x7c00 + BPB_Fat32ExtByts + BS_DrvNum) >= 0x80) {
        default_drive_number =
            *(unsigned char *)(0x7c00 + BPB_Fat32ExtByts + BS_DrvNum) - 0x80 +
            0x02;
      } else {
        default_drive_number =
            *(unsigned char *)(0x7c00 + BPB_Fat32ExtByts + BS_DrvNum);
      }
    }
    default_drive = default_drive_number + 0x41;
    flags_once = true;
  }
  NowTask()->drive = default_drive;
  NowTask()->drive_number = default_drive_number;
  vfs_mount_disk(NowTask()->drive, NowTask()->drive);
  vfs_change_disk(NowTask()->drive);
  printf("DOSLDR 386 v0.2\n");
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

  printf("Loading...\n");
  // printf("ESP:%08x\n",*(unsigned int *)(0x00280000 + 12));
  _IN();
  for (;;)
    ;
}
struct TASK *NowTask() {
  return &MainTask;
}
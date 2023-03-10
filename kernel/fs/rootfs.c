#include <dos.h>

// do nothing
static void Read(char drive,
                 unsigned char* buffer,
                 unsigned int number,
                 unsigned int lba) {
  printf("[root fs]don't try to read!\n");
}
static void Write(char drive,
                  unsigned char* buffer,
                  unsigned int number,
                  unsigned int lba) {
  printf("[root fs]don't try to write!\n");
}
// -----
void init_rootfs() {
  vdisk vd;
  strcpy(vd.DriveName, "root fs");
  vd.Read =Read;
  vd.size = 114514;
  vd.Write = Write;
  vd.flag = 1;
  register_vdisk(vd);
}
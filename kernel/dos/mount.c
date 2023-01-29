/* MOUNT.C : 映射文件为disk */
#include <dos.h>
static void MountDiskRead(char drive, unsigned char *buffer,
                          unsigned int number, unsigned int lba);
static void MountDiskWrite(char drive, unsigned char *buffer,
                           unsigned int number, unsigned int lba);
typedef struct {
  char drive;
  FILE *fp;
  int flag;
  vdisk vd;
} mount_disk;
mount_disk md[255];
void init_mount_disk() {
  for (int i = 0; i < 255; i++) {
    md[i].flag = 0;
  }
}
int mount(char *fileName) {
  for (int i = 0; i < 255; i++) {
    if (md[i].flag == 0) {
      FILE *fp = fopen(fileName, "rw");
      md[i].vd.flag = 1;
      md[i].vd.Read = MountDiskRead;
      md[i].vd.Write = MountDiskWrite;
      md[i].vd.size = fp->size;
      md[i].flag = 1;
      md[i].fp = fp;
      md[i].drive = register_vdisk(md[i].vd);
      return md[i].drive; // 成功
    }
  }
  printf("[mount]not found.\n");
  return 0; // 失败
}
void unmount(char drive) {
  for (int i = 0; i < 255; i++) {
    if (md[i].flag && md[i].drive == drive) {
      md[i].flag = 0;
      logout_vdisk(md[i].drive);
      fclose(md[i].fp);
      return;
    }
  }
}
static void MountDiskRead(char drive, unsigned char *buffer,
                          unsigned int number, unsigned int lba) {
  for (int i = 0; i < 255; i++) {
    if (md[i].flag && md[i].drive == drive) {
      memcpy(buffer, md[i].fp->buf + lba * 512, number * 512);
      return;
    }
  }
}
static void MountDiskWrite(char drive, unsigned char *buffer,
                           unsigned int number, unsigned int lba) {
  for (int i = 0; i < 255; i++) {
    if (md[i].flag && md[i].drive == drive) {
      memcpy(md[i].fp->buf + lba * 512, buffer, number * 512);
      return;
    }
  }
}
#include <drivers.h>
int getReadyDisk();  // init.c
vdisk vdisk_ctl[255];
int init_vdisk() {
  for (int i = 0; i < 255; i++) {
    vdisk_ctl[i].flag = 0;  // 设置为未使用
  }
}
int register_vdisk(vdisk vd) {
  for (int i = 0; i < 255; i++) {
    if (!vdisk_ctl[i].flag) {
      vdisk_ctl[i] = vd;                  // 找到了！
      return i + ('C' + getReadyDisk());  // 注册成功，返回drive
    }
  }
  return 0;  // 注册失败
}
int logout_vdisk(char drive) {
  int indx = drive - ('C' + getReadyDisk());
  if (indx > 255) {
    return 0;  // 失败
  }
  if (vdisk_ctl[indx].flag) {
    vdisk_ctl[indx].flag = 0;  // 设置为没有
    return 1;                  // 成功
  } else {
    return 0;  // 失败
  }
}
int rw_vdisk(char drive,
             unsigned int lba,
             unsigned char* buffer,
             unsigned int number,
             int read) {
  int indx = drive - ('C' + getReadyDisk());
  if (indx > 255) {
    return 0;  // 失败
  }
  if (vdisk_ctl[indx].flag) {
    if (read) {
      vdisk_ctl[indx].Read(drive, buffer, number, lba);
    } else {
      vdisk_ctl[indx].Write(drive, buffer, number, lba);
    }
    return 1;  // 成功
  } else {
    return 0;  // 失败
  }
}
bool have_vdisk(char drive) {
  int indx = drive - ('C' + getReadyDisk());
  if (indx > 255) {
    return 0;  // 失败
  }
  if (vdisk_ctl[indx].flag) {
    return 1;                  // 成功
  } else {
    return 0;  // 失败
  }
}
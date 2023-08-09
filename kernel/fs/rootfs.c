#include <dos.h>

// do nothing
static void Read(char drive, unsigned char *buffer, unsigned int number,
                 unsigned int lba) {
  uint32_t **buf = (uint32_t *)buffer;
  **buf = 1;
  printf("[dev fs]don't try to read!\n");
}
static void Write(char drive, unsigned char *buffer, unsigned int number,
                  unsigned int lba) {
  printf("[dev fs]don't try to write!\n");
}
static bool dev_check(uint8_t disk_number) {
  uint32_t **b = page_malloc(512);
  *b = page_malloc(4);
  uint32_t *bmp = *b;

  Disk_Read(0, 1, b, disk_number);
  if (*b != bmp) {
    return false;
  }
  if (**b) {
    printk("this.\n");
    page_free(b, 512);
    page_free(*b, 4);
    return true;
  } else {
    printk("false.\n");
    page_free(b, 512);
    page_free(*b, 4);
    return false;
  }
}

static void dev_copy_cache(struct vfs_t *dest, struct vfs_t *src) { return; }
static void dev_init(struct vfs_t *vfs, uint8_t disk_number) {
  printf("init dev fs.\n");
  return;
}
static void dev_cd(struct vfs_t *vfs, char *dictName) { return; }
// -----
void init_devfs() {
  vdisk vd;
  strcpy(vd.DriveName, "dev");
  vd.Read = Read;
  vd.size = 114514;
  vd.Write = Write;
  vd.flag = 1;
  register_vdisk(vd);
  vfs_t fs;
  fs.flag = 1;
  fs.cache = NULL;
  strcpy(fs.FSName, "DEVFS");
  fs.CopyCache = dev_copy_cache;
  fs.Format = NULL;
  fs.CreateFile = NULL;
  fs.CreateDict = NULL;
  fs.DelDict = NULL;
  fs.DelFile = NULL;
  fs.ReadFile = NULL;
  fs.WriteFile = NULL;
  fs.DeleteFs = NULL;
  fs.cd = dev_cd;
  fs.FileSize = NULL;
  fs.Check = dev_check;
  fs.ListFile = NULL;
  fs.InitFs = dev_init;
  fs.RenameFile = NULL;
  fs.Attrib = NULL;
  fs.FileInfo = NULL;
  vfs_register_fs(fs);
}
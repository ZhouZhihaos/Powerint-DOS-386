// fs.c : 文件系统的实现
//目前实现了：fat12文件系统
#include <dos.h>
#include <fs.h>
static inline int get_fat12_date(unsigned short year, unsigned short month,
                                 unsigned short day) {
  year -= 1980;
  unsigned short date = 0;
  date |= (year & 0x7f) << 9;
  date |= (month & 0x0f) << 5;
  date |= (day & 0x1f);
  return date;
}
static inline int get_fat12_time(unsigned short hour, unsigned short minute) {
  unsigned short time = 0;
  time |= (hour & 0x1f) << 11;
  time |= (minute & 0x3f) << 5;
  return time;
}
void read_fat(unsigned char *img, int *fat, unsigned char *ff) {
  int i, j = 0;
  for (i = 0; i < 3072; i += 2) {
    fat[i + 0] = (img[j + 0] | img[j + 1] << 8) & 0xfff;
    fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
    j += 3;
  }
  // 保留簇
  ff[0] = true;
  ff[1] = true;
  for (i = 1; i < 3072; i++) {
    if (fat[i] > 0 && fat[i] < 0xff0) {
      ff[fat[i]] = true;
    } else if (fat[i] >= 0xff0 && fat[i] <= 0xfff) {
      ff[i + 1] = true;
    }
  }
  return;
}
void write_fat(unsigned char *img, int *fat) {
  int i, j = 0;
  for (i = 0; i < 3072; i += 2) {
    img[j + 0] = fat[i + 0] & 0xff;
    img[j + 1] = (fat[i + 0] >> 8 | fat[i + 1] << 4) & 0xff;
    img[j + 2] = (fat[i + 1] >> 4) & 0xff;
    j += 3;
  }
  return;
}
void file_loadfile(int clustno, int size, char *buf, int *fat,
                   int drive_number) {
  if (!size) {
    return;
  }
  struct TASK *task = NowTask();
  void *img = page_malloc(
      ((size - 1) / drive_ctl.drives[drive_number].ClustnoBytes + 1) *
      drive_ctl.drives[drive_number].ClustnoBytes);
  for (int i = 0;
       i != (size - 1) / drive_ctl.drives[drive_number].ClustnoBytes + 1; i++) {
    uint32_t sec =
        (drive_ctl.drives[drive_number].FileDataAddress +
         (clustno - 2) * drive_ctl.drives[drive_number].ClustnoBytes) /
        drive_ctl.drives[drive_number].SectorBytes;
    Disk_Read(sec,
              drive_ctl.drives[drive_number].ClustnoBytes /
                  drive_ctl.drives[drive_number].SectorBytes,
              img + i * drive_ctl.drives[drive_number].ClustnoBytes,
              drive_number + 0x41);
    clustno = fat[clustno];
  }
  memcpy((void *)buf, img, size);
  page_free(img, ((size - 1) / drive_ctl.drives[drive_number].SectorBytes + 1) *
                     drive_ctl.drives[drive_number].SectorBytes);
  return;
}
FILE *f_t;
void file_savefile(int clustno, int size, char *buf, int *fat,
                   unsigned char *ff, int drive_number) {
  struct TASK *task = NowTask();
  uint32_t clustall = 0;
  int tmp = clustno;
  while (fat[clustno] !=
         0xfff) { // 计算文件占多少Fat项 Fat项 = 大小 / 簇大小 + 1
    clustno = fat[clustno];
    clustall++;
  }
  int old_clustno = clustno;
  clustno = tmp;
  int alloc_size;
  if (size > (clustall + 1) *
                 drive_ctl.drives[drive_number]
                     .ClustnoBytes) { // 新大小 > (旧大小 / 簇大小 + 1) * 簇大小
    // 请求内存大小 = (新大小 / 簇大小 + 1) * 簇大小
    alloc_size =
        ((size - 1) / drive_ctl.drives[drive_number].ClustnoBytes + 1) *
        drive_ctl.drives[drive_number].ClustnoBytes;
    // 分配Fat（这里需要在写盘前分配）
    for (int size1 = size;
         size1 > ((clustall + 1) * drive_ctl.drives[drive_number].ClustnoBytes);
         size1 -= drive_ctl.drives[drive_number].ClustnoBytes) {
      for (int i = 0; i != 3072; i++) {
        if (!ff[i]) {
          fat[old_clustno] = i;
          old_clustno = i;
          ff[i] = true;
          break;
        }
      }
    }
    fat[old_clustno] = 0xfff; // 结尾Fat = 0xfff
    ff[old_clustno] = true;
  } else if (size <=
             (clustall + 1) * drive_ctl.drives[drive_number]
                                  .ClustnoBytes) { // 新大小 <= (旧大小 / 簇大小
                                                   // + 1) * 簇大小
    // 请求内存大小 = (旧大小 / 簇大小 + 1) * 簇大小
    alloc_size = (clustall + 1) * drive_ctl.drives[drive_number].ClustnoBytes;
    // 这里不分配Fat的原因是要清空更改后多余的数据
  }
  void *img = page_malloc(alloc_size);
  clean((char *)img, alloc_size);
  memcpy(img, (void *)buf, size); // 把要写入的数据复制到新请求的内存地址
  for (int i = 0;
       i != (alloc_size / drive_ctl.drives[drive_number].ClustnoBytes); i++) {
    // 计算LBA & 写盘
    uint32_t sec =
        (drive_ctl.drives[drive_number].FileDataAddress +
         (clustno - 2) * drive_ctl.drives[drive_number].ClustnoBytes) /
        drive_ctl.drives[drive_number].SectorBytes;
    Disk_Write(sec,
               drive_ctl.drives[drive_number].ClustnoBytes /
                   drive_ctl.drives[drive_number].SectorBytes,
               img + i * drive_ctl.drives[drive_number].ClustnoBytes,
               drive_number + 0x41);
    clustno = fat[clustno];
  }
  page_free(img, alloc_size);
  if (size <
      clustall * drive_ctl.drives[drive_number]
                     .ClustnoBytes) { // 新大小 < (旧大小 / 簇大小) * 簇大小
    // 分配Fat（中间情况没必要分配）
    int i;
    for (int size1 = clustall * drive_ctl.drives[drive_number].ClustnoBytes,
             i = old_clustno;
         size1 > size; size1 -= drive_ctl.drives[drive_number].ClustnoBytes) {
      fat[i] = 0;
      ff[i] = false;
      for (int j = 0; j != 3072; j++)
        if (fat[j] == i)
          i = j;
    }
    old_clustno = i;
    fat[old_clustno] = 0xfff; // 结尾Fat = 0xfff
    ff[old_clustno] = true;
  }
  file_savefat(fat, drive_number);
}
void file_saveinfo(struct FILEINFO *directory, int drive_number) {
  struct TASK *task = NowTask();
  if (directory == drive_ctl.drives[drive_number].root_directory) {
    Disk_Write(drive_ctl.drives[drive_number].RootDictAddress /
                   drive_ctl.drives[drive_number].SectorBytes,
               14 * drive_ctl.drives[drive_number].ClustnoBytes /
                   drive_ctl.drives[drive_number].SectorBytes,
               (void *)directory, drive_number + 0x41);
  } else {
    for (int i = 1;
         FindForCount(i, drive_ctl.drives[drive_number].directory_list) != NULL;
         i++) {
      struct List *list =
          FindForCount(i, drive_ctl.drives[drive_number].directory_list);
      if (list->val == directory) {
        list = FindForCount(
            i, drive_ctl.drives[drive_number].directory_clustno_list);
        Disk_Write(
            (drive_ctl.drives[drive_number].FileDataAddress +
             (list->val - 2) * drive_ctl.drives[drive_number].ClustnoBytes) /
                drive_ctl.drives[drive_number].SectorBytes,
            drive_ctl.drives[drive_number].ClustnoBytes /
                drive_ctl.drives[drive_number].SectorBytes,
            (void *)directory, drive_number + 0x41);
        break;
      }
    }
  }
}
void file_savefat(int *fat, int drive_number) {
  struct TASK *task = NowTask();
  write_fat(drive_ctl.drives[drive_number].ADR_DISKIMG +
                drive_ctl.drives[drive_number].Fat1Address,
            fat);
  write_fat(drive_ctl.drives[drive_number].ADR_DISKIMG +
                drive_ctl.drives[drive_number].Fat2Address,
            fat);
  Disk_Write(1, 9,
             drive_ctl.drives[drive_number].ADR_DISKIMG +
                 drive_ctl.drives[drive_number].Fat2Address,
             drive_number + 0x41);
  Disk_Write(10, 9,
             drive_ctl.drives[drive_number].ADR_DISKIMG +
                 drive_ctl.drives[drive_number].Fat2Address,
             drive_number + 0x41);
}
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max) {
  int i, j;
  char s[12];
  for (j = 0; j < 11; j++) {
    s[j] = ' ';
  }
  j = 0;
  for (i = 0; name[i] != 0; i++) {
    if (j >= 11) {
      return 0; /*没有找到*/
    }
    if (name[i] == '.' && j <= 8) {
      j = 8;
    } else {
      s[j] = name[i];
      if ('a' <= s[j] && s[j] <= 'z') {
        /*将小写字母转换为大写字母*/
        s[j] -= 0x20;
      }
      j++;
    }
  }
  for (i = 0; i < max;) {
    if (finfo[i].name[0] == 0x00) {
      break;
    }
    if ((finfo[i].type & 0x18) == 0) {
      for (j = 0; j < 11; j++) {
        if (finfo[i].name[j] != s[j]) {
          goto next;
        }
      }
      return finfo + i; /*找到文件*/
    }
  next:
    i++;
  }
  return 0; /*没有找到*/
}
struct FILEINFO *dict_search(char *name, struct FILEINFO *finfo, int max) {
  int i, j;
  char s[12];
  for (j = 0; j < 11; j++) {
    s[j] = ' ';
  }
  j = 0;
  for (i = 0; name[i] != 0; i++) {
    if (j >= 11) {
      return 0; /*没有找到*/
    } else {
      s[j] = name[i];
      if ('a' <= s[j] && s[j] <= 'z') {
        /*将小写字母转换为大写字母*/
        s[j] -= 0x20;
      }
      j++;
    }
  }
  for (i = 0; i < max;) {
    if (finfo[i].name[0] == 0x00) {
      break;
    }
    if (finfo[i].type == 0x10) {
      for (j = 0; j < 11; j++) {
        if (finfo[i].name[j] != s[j]) {
          goto next;
        }
      }
      return finfo + i; /* 找到文件 */
    }
  next:
    i++;
  }
  return 0; /*没有找到*/
}
struct FILEINFO *Get_File_Address(char *path1) {
  struct TASK *task = NowTask();
  struct FILEINFO *bmpDict = task->directory;
  int drive_number = task->drive_number;
  char *path = (char *)page_malloc(strlen(path1) + 1);
  strcpy(path, path1);
  strtoupper(path);
  if (strncmp("TSKDRV:\\", path, 8) == 0) {
    path += 8;
    bmpDict = drive_ctl.drives[drive_number].root_directory;
  } else {
    if (strncmp(":\\", path + 1, 2) == 0 || strncmp(":/", path + 1, 2) == 0) {
      drive_number = *path - 0x41;
      path += 3;
      bmpDict = drive_ctl.drives[drive_number].root_directory;
    }
  }
  if (path[0] == '\\' || path[0] == '/') {
    //跳过反斜杠和正斜杠
    for (int i = 0; i < strlen(path); i++) {
      if (path[i] != '\\' && path[i] != '/') {
        path += i;
        break;
      }
    }
  }
  char *temp_name = (char *)page_malloc(128);
  struct FILEINFO *finfo;
  int i = 0;
  while (1) {
    int j;
    for (j = 0; i < strlen(path); i++, j++) {
      if (path[i] == '\\' || path[i] == '/') {
        temp_name[j] = '\0';
        // printk("Got It:%s,ALL:%s\n", temp_name, path);
        i++;
        break;
      }
      temp_name[j] = path[i];
    }
    finfo = dict_search(temp_name, bmpDict,
                        drive_ctl.drives[drive_number].RootMaxFiles);
    if (finfo == 0) {
      if (path[i] != '\0') {
        page_free((int)temp_name, 128);
        page_free((int)path, strlen(path1) + 1);
        return 0;
      }
      finfo = file_search(temp_name, bmpDict,
                          drive_ctl.drives[drive_number].RootMaxFiles);
      if (finfo == 0) {
        // printk("Invalid file:%s\n", temp_name);
        page_free((int)temp_name, 128);
        page_free((int)path, strlen(path1) + 1);
        return 0;
      } else {
        goto END;
      }
    } else {
      // printk("dict_search:%s ", temp_name);
      if (finfo->clustno != 0) {
        for (int count = 1;
             FindForCount(
                 count,
                 drive_ctl.drives[drive_number].directory_clustno_list) != NULL;
             count++) {
          struct List *list = FindForCount(
              count, drive_ctl.drives[drive_number].directory_clustno_list);
          if (list->val == finfo->clustno) {
            list = FindForCount(count,
                                drive_ctl.drives[drive_number].directory_list);
            bmpDict = (struct FILEINFO *)list->val;
            // printk("finfo:%08x\n", bmpDict);
            break;
          }
        }
      } else {
        // printk("finfo:%08x\n",
        // drive_ctl.drives[drive_number].root_directory);
        bmpDict = drive_ctl.drives[drive_number].root_directory;
      }
      clean(temp_name, 128);
    }
  }
END:
  // printk("file_search:%s finfo:%08x\n", temp_name, finfo);
  page_free((int)temp_name, 128);
  page_free((int)path, strlen(path1) + 1);
  return finfo;
}
struct FILEINFO *Get_dictaddr(char *path1) {
  struct TASK *task = NowTask();
  struct FILEINFO *bmpDict = task->directory;
  int drive_number = task->drive_number;
  char *path = (char *)page_malloc(strlen(path1) + 1);
  strcpy(path, path1);
  strtoupper(path);
  if (strncmp("TSKDRV:\\", path, 8) == 0) {
    path += 8;
    bmpDict = drive_ctl.drives[drive_number].root_directory;
  } else {
    if (strncmp(":\\", path + 1, 2) == 0 || strncmp(":/", path + 1, 2) == 0) {
      drive_number = *path - 0x41;
      path += 3;
      bmpDict = drive_ctl.drives[drive_number].root_directory;
    }
  }
  if (path[0] == '\\' || path[0] == '/') {
    //跳过反斜杠和正斜杠
    for (int i = 0; i < strlen(path); i++) {
      if (path[i] != '\\' && path[i] != '/') {
        path += i;
        break;
      }
    }
  }
  char *temp_name = (char *)page_malloc(128);
  struct FILEINFO *finfo;
  int i = 0;
  while (1) {
    int j;
    for (j = 0; i < strlen(path); i++, j++) {
      if (path[i] == '\\' || path[i] == '/') {
        temp_name[j] = '\0';
        // printf("Got It:%s,ALL:%s\n", temp_name, path);
        i++;
        break;
      }
      temp_name[j] = path[i];
    }
    finfo = dict_search(temp_name, bmpDict,
                        drive_ctl.drives[drive_number].RootMaxFiles);
    if (finfo == 0) {
      goto END;
    } else {
      // printf("dict_search:%s drv_num=%02x\n", temp_name, drive_number);
      if (finfo->clustno != 0) {
        for (int count = 1;
             FindForCount(
                 count,
                 drive_ctl.drives[drive_number].directory_clustno_list) != NULL;
             count++) {
          struct List *list = FindForCount(
              count, drive_ctl.drives[drive_number].directory_clustno_list);
          if (list->val == finfo->clustno) {
            list = FindForCount(count,
                                drive_ctl.drives[drive_number].directory_list);
            // printf("change %08x to", bmpDict);
            bmpDict = (struct FILEINFO *)list->val;
            // printf("%08x\n", bmpDict);

            break;
          } else {
            // printf("list->val = %08x finfo->clustno = %08x %08x\n",
            // list->val,
            //   finfo->clustno,
            //   drive_ctl.drives[drive_number].directory_clustno_list);
          }
        }
        // printf(
        //     "Not Found %08x,and "
        //     "FindForCount(count,drive_ctl.drives[drive_number].directory_"
        //     "clustno_list)=%08x %08x\n",
        //     finfo->clustno,
        //     FindForCount(
        //         1,
        //         drive_ctl.drives[drive_number].directory_clustno_list),drive_ctl.drives[drive_number].directory_clustno_list);
      } else {
        // printf("finfo->clustno == 0\n");
        bmpDict = drive_ctl.drives[drive_number].root_directory;
      }
      clean(temp_name, 128);
      if (path[i] == '\0') {
        goto END;
      }
    }
  }
END:
  page_free((int)temp_name, 128);
  page_free((int)path, strlen(path1) + 1);
  // printf("bmpDict=%08x root=%08x\n", bmpDict,
  //       drive_ctl.drives[drive_number].root_directory);
  return bmpDict;
}
struct List *Get_wildcard_File_Address(char *path) {
  struct TASK *task = NowTask();
  struct FILEINFO *finfo = Get_dictaddr(path);
  struct List *list = NewList();
  strtoupper(path);
  int tmp = 0;
  for (int i = 0; i != strlen(path); i++) {
    if (path[i] == '\\' || path[i] == '/') {
      tmp = i + 1;
    }
  }
  path += tmp;
  char s[12];
  for (int i = 0; i != 11; i++) {
    s[i] = ' ';
  }
  for (int i = 0, j = 0; i != 11; i++, j++) {
    if (path[j] == '.') {
      i = 7;
    } else {
      s[i] = path[j];
    }
  }
  bool name = false, ext = false;
  char *p = path;
  if (p[0] == '*') {
    name = true;
  }
  for (; *p != '.'; p++)
    ;
  p++;
  if (p[0] == '*') {
    ext = true;
  }
  if (!name && !ext) {
    bool back = true;
    for (int i = 0; i != 12; i++) {
      if (s[i] == '?') {
        back = false;
        break;
      }
    }
    if (back) {
      return (struct List *)NULL;
    }
  }
  for (int i = 0; i != drive_ctl.drives[task->drive_number].RootMaxFiles; i++) {
    bool add = true;
    if (!name) {
      for (int j = 0; j != 8; j++) {
        if (finfo[i].name[j] != s[j] && s[j] != '?') {
          add = false;
          break;
        }
      }
    }
    if (!ext) {
      for (int j = 8; j != 11; j++) {
        if (finfo[i].ext[j - 8] != s[j] && s[j] != '?') {
          add = false;
          break;
        }
      }
    }
    if (add) {
      AddVal((int)(finfo + i), list);
    }
    if (finfo[i].name[0] == '\0') {
      return list;
    }
  }
}
void mkdir(char *dictname, int last_clust) {
  /*
          dictname:目录名
          last_clust:上一级目录的簇号
  */
  struct TASK *task = NowTask();
  mkfile(dictname);
  struct FILEINFO *finfo = Get_File_Address(dictname);
  FILE *fp = fopen(dictname, "wb");
  // 三个目录项（模板）
  struct FILEINFO dictmodel1; // .目录项，指向自己
  struct FILEINFO dictmodel2; // ..目录项，指向上一级目录
  struct FILEINFO null;       //空目录项（为mkfile函数提供指引）
  memcpy(null.name, "NULL       ", 11);
  null.type = 0x20; // 文件的type属性是0x20
  //将size date time这类属性全部设置为0
  null.size = 0;
  null.date = get_fat12_date(get_year(), get_mon_hex(), get_day_of_month());
  null.time = get_fat12_time(get_hour_hex(), get_min_hex());
  for (int i = 0; i != 3072; i++) {
    if (!drive_ctl.drives[task->drive_number].fat[i]) {
      null.clustno = i;
      drive_ctl.drives[task->drive_number].fat[i] = 0xfff;
      drive_ctl.drives[task->drive_number].FatClustnoFlags[i] = true;
      break;
    }
  }
  dictmodel1.name[0] = '.';
  for (int i = 1; i != 8; i++) {
    dictmodel1.name[i] = ' ';
  }
  for (int i = 0; i != 3; i++) {
    dictmodel1.ext[i] = ' ';
  }
  dictmodel1.type = 0x10;
  dictmodel1.clustno = finfo->clustno; // 指向自己
  dictmodel1.size = 0;
  dictmodel1.date =
      get_fat12_date(get_year(), get_mon_hex(), get_day_of_month());
  dictmodel1.time = get_fat12_time(get_hour_hex(), get_min_hex());
  dictmodel2.name[0] = '.';
  dictmodel2.name[1] = '.';
  for (int i = 2; i != 8; i++) {
    dictmodel2.name[i] = ' ';
  }
  for (int i = 0; i != 3; i++) {
    dictmodel2.ext[i] = ' ';
  }
  dictmodel2.clustno = last_clust;
  dictmodel2.size = 0;
  dictmodel2.date =
      get_fat12_date(get_year(), get_mon_hex(), get_day_of_month());
  dictmodel2.time = get_fat12_time(get_hour_hex(), get_min_hex());
  dictmodel2.type = 0x10;
  unsigned char *ptr = (unsigned char *)&dictmodel1;
  fseek(fp, 0, 0);
  for (int i = 0; i != 32; i++) {
    fputc(ptr[i], fp);
  }
  ptr = (unsigned char *)&dictmodel2;
  fseek(fp, 32, 0);
  for (int i = 0; i != 32; i++) {
    fputc(ptr[i], fp);
  }
  ptr = (unsigned char *)&null;
  fseek(fp, 64, 0);
  for (int i = 0; i != 32; i++) {
    fputc(ptr[i], fp);
  }
  fclose(fp);
  finfo->type = 0x10; // 是目录（文件夹的type属性是0x10）
  finfo->size = 0;
  int drive_number;
  if (strncmp(dictname + 1, ":\\", 2) == 0 ||
      strncmp(dictname + 1, ":/", 2) == 0) {
    drive_number = *dictname - 0x41;
  } else {
    drive_number = task->drive_number;
  }
  AddVal(finfo->clustno, drive_ctl.drives[drive_number].directory_clustno_list);
  // printf("Add = %08x %02x %08x\n", finfo->clustno, drive_number,
  //        drive_ctl.drives[drive_number].directory_clustno_list);

  // for (int count = 1;
  //      FindForCount(count,
  //                   drive_ctl.drives[drive_number].directory_clustno_list) !=
  //      NULL;
  //      count++) {
  //   printf("%02x ",
  //          FindForCount(count,
  //                       drive_ctl.drives[drive_number].directory_clustno_list)
  //              ->val);
  // }
  if (last_clust == 0) {
    file_saveinfo(drive_ctl.drives[drive_number].root_directory, drive_number);
  } else {
    for (int i = 1;
         FindForCount(
             i, drive_ctl.drives[drive_number].directory_clustno_list) != NULL;
         i++) {
      struct List *list = FindForCount(
          i, drive_ctl.drives[drive_number].directory_clustno_list);
      if (list->val == last_clust) {
        list = FindForCount(i, drive_ctl.drives[drive_number].directory_list);
        struct FILEINFO *d_finfo = (struct FILEINFO *)list->val;
        file_saveinfo(d_finfo, drive_number);
      }
    }
  }
  file_savefat(drive_ctl.drives[drive_number].fat, drive_number);

  void *directory_alloc =
      page_malloc(drive_ctl.drives[drive_number].ClustnoBytes);
  Disk_Read(
      (drive_ctl.drives[drive_number].FileDataAddress +
       (finfo->clustno - 2) * drive_ctl.drives[drive_number].ClustnoBytes) /
          drive_ctl.drives[drive_number].SectorBytes,
      drive_ctl.drives[drive_number].ClustnoBytes /
          drive_ctl.drives[drive_number].SectorBytes,
      directory_alloc, drive_number + 0x41);
  AddVal(directory_alloc, drive_ctl.drives[drive_number].directory_list);
  return;
}
int Copy(char *path, char *path1) {
  unsigned char *path1_file_buffer;
  if (fsz(path) == -1) {
    // printf("file not found\n");
    return -1;
  }
  // printf("-----------------------------\n");
  mkfile(path1);

  path1_file_buffer = malloc(fsz(path) + 1);
  int sz = fsz(path);
  FILE *fp = fopen(path, "rb");
  fread(path1_file_buffer, sz, 1, fp);
  fclose(fp);
  fp = fopen(path1, "wb");
  for (int i = 0; i < sz; i++) {
    fputc(path1_file_buffer[i], fp);
  }
  fclose(fp);
  free(path1_file_buffer);
  return 0;
}
struct FILEINFO *clust_sech(int clustno, struct FILEINFO *finfo, int max) {
  //通过簇号找到文件信息
  int i, j;
  j = 0;
  for (i = 0; i < max; i++) {
    if (finfo[i].clustno == clustno) {
      return finfo + i;
    }
  }
  return 0; /*没找到*/
}
int del(char *cmdline) {
  //删除某个文件
  struct TASK *task = NowTask();
  char name[30];
  int i;
  struct FILEINFO *finfo;
  for (i = 0; i < strlen(cmdline); i++) {
    name[i] = cmdline[i + 4];
  }
  finfo = Get_File_Address(name);
  if (finfo == 0 || finfo->type == 0x01 || finfo->type == 0x04) {
    return -1;
  }
  FILE *fp = fopen(name, "wb");
  for (i = 0; i != fp->size; i++) {
    fputc(0, fp);
  }
  fp->size = 0;
  fclose(fp);
  finfo->name[0] = 0xe5;
  int drive_number;
  if (strncmp(name + 1, ":\\", 2) == 0 || strncmp(name + 1, ":/", 2) == 0) {
    drive_number = *name - 0x41;
  } else {
    drive_number = task->drive_number;
  }
  drive_ctl.drives[drive_number].fat[finfo->clustno] = 0;
  drive_ctl.drives[drive_number].FatClustnoFlags[finfo->clustno] = false;
  file_saveinfo(Get_dictaddr(name), drive_number);
  file_savefat(drive_ctl.drives[drive_number].fat, drive_number);
  return 0;
}
int deldir(char *path) {
  struct TASK *task = NowTask();
  struct FILEINFO *finfo = Get_dictaddr(path);
  if (finfo == Get_dictaddr(".")) {
    return -1;
  }
  struct FILEINFO *f = task->directory;
  task->directory = finfo;
  for (int i = 2; finfo[i].name[0] != '\0'; i++) {
    if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
      char s[30];
      int j = 0;
      for (; finfo[i].name[j] != ' '; j++) {
        s[j] = finfo[i].name[j];
      }
      s[j] = '\0';
      // printf("(CALL)DEL DIR:%s\n", s);
      if (deldir(s) == -1) {
        return -1;
      }
      // return -1;
    }
  }
  for (int i = 2; finfo[i].name[0] != '\0'; i++) {
    if (finfo[i].name[0] != 0xe5 && finfo[i].type != 0x10) {
      char s[30] = {'D', 'E', 'L', ' '};
      int p = 4;
      for (int j = 0; finfo[i].name[j] != ' '; j++, p++) {
        s[p] = finfo[i].name[j];
      }
      if (finfo[i].ext[0] != ' ') {
        s[p++] = '.';
        for (int j = 0; finfo[i].ext[j] != ' ' || j != 3; j++, p++) {
          s[p] = finfo[i].ext[j];
        }
      }
      s[p] = '\0';
      // printf("(IN)DEL FILE:%s\n", s);
      if (del(s) == -1) {
        return -1;
      }
    }
  }
  task->directory = f;
  // printf("(IN)DEL SELF\n");
  struct FILEINFO *root_finfo;
  if (finfo[1].clustno == 0) {
    root_finfo = drive_ctl.drives[task->drive_number].root_directory;
  } else {
    for (int i = 1;
         FindForCount(
             i, drive_ctl.drives[task->drive_number].directory_clustno_list) !=
         NULL;
         i++) {
      if (FindForCount(
              i, drive_ctl.drives[task->drive_number].directory_clustno_list)
              ->val == finfo[1].clustno) {
        root_finfo = (struct FILEINFO *)FindForCount(
                         i, drive_ctl.drives[task->drive_number].directory_list)
                         ->val;
        // printf("FIND ROOT %08x\n", root_finfo);
      }
    }
  }
  for (int i = 0; root_finfo[i].name[0] != '\0'; i++) {
    // printf("ROOT FILE:%s\n", root_finfo[i].name);
    if (root_finfo[i].clustno == finfo[0].clustno) {
      root_finfo[i].name[0] = 0xe5;
      break;
    }
  }
  int drive_number;
  if (strncmp(path + 1, ":\\", 2) == 0 || strncmp(path + 1, ":/", 2) == 0) {
    drive_number = *path - 0x41;
  } else {
    drive_number = task->drive_number;
  }
  drive_ctl.drives[drive_number].fat[finfo->clustno] = 0;
  drive_ctl.drives[drive_number].FatClustnoFlags[finfo->clustno] = false;
  file_saveinfo(Get_dictaddr(path), drive_number);
  file_savefat(drive_ctl.drives[drive_number].fat, drive_number);
  return 0;
}
void mkfile(char *name) {
  struct TASK *task = NowTask();
  char s[12];
  int i, j;
  struct FILEINFO *finfo = Get_dictaddr(name);
  int drive_number;
  if (strncmp(name + 1, ":\\", 2) == 0 || strncmp(name + 1, ":/", 2) == 0) {
    drive_number = *name - 0x41;
  } else {
    drive_number = task->drive_number;
  }

  char *path = name;
  for (i = strlen(name); i >= 0; i--) {
    if (name[i] == '/' || name[i] == '\\') {
      name += i + 1;
      break;
    }
  }

  for (j = 0; j != 12; j++) {
    s[j] = ' ';
  }
  j = 0;
  for (i = 0; name[i] != 0; i++) {
    if (j >= 11) {
      return;
    }
    if (name[i] == '.' && j <= 8) {
      j = 8;
    } else {
      s[j] = name[i];
      if ('a' <= s[j] && s[j] <= 'z') {
        s[j] -= 0x20;
      }
      j++;
    }
  }
  for (i = 0;; i++) {
    if (finfo[i].name[0] == 0x00 || finfo[i].name[0] == 0xe5) {
      finfo = finfo + i;
      break;
    }
  }
  for (i = 0; i != 8; i++) {
    finfo->name[i] = s[i];
  }
  for (i = 8; i != 11; i++) {
    finfo->ext[i - 8] = s[i];
  }
  finfo->type = 0x20;
  for (int i = 0; i != 3072; i++) {
    if (!drive_ctl.drives[drive_number].fat[i]) {
      finfo->clustno = i;
      drive_ctl.drives[drive_number].fat[i] = 0xfff;
      drive_ctl.drives[drive_number].FatClustnoFlags[i] = true;
      break;
    }
  }
  for (i = 0; i != 10; i++) {
    finfo->reserve[i] = 0;
  }
  finfo->time = get_fat12_time(get_hour_hex(), get_min_hex());
  finfo->date = get_fat12_date(get_year(), get_mon_hex(), get_day_of_month());
  finfo->size = 0;
  file_saveinfo(Get_dictaddr(path), drive_number);
  file_savefat(drive_ctl.drives[drive_number].fat, drive_number);
  return;
}
int changedict(char *dictname) {
  // cd命令的依赖函数
  strtoupper(dictname);
  struct TASK *task = NowTask();
  struct FILEINFO *finfo =
      dict_search(dictname, task->directory,
                  drive_ctl.drives[task->drive_number].RootMaxFiles);
  //找文件夹
  if (finfo == 0) {
    //没找到
    return -1;
  }
  if (finfo->clustno == 0) {
    //根目录
    strcpy(task->path, "");
    task->change_dict_times = 0;
    task->directory = drive_ctl.drives[task->drive_number].root_directory;
    return 0;
  }
  //..进行特殊的处理
  //.不进行处理
  //其他按照下面的方式处理
  if (strcmp(dictname, "..") != 0 && strcmp(dictname, ".") != 0) {
    if (task->change_dict_times == 0) {
      strcat(task->path, dictname);
    } else {
      strcat(task->path, "\\");
      strcat(task->path, dictname);
    }
  }

  if (strcmp(dictname, "..") == 0) {
    int i;
    // print("OK\n");
    for (i = strlen(task->path) - 1; i >= 0; i--) {
      if (task->path[i] == '\\') {
        task->path[i] = '\0';
        break;
      }
    }
    task->change_dict_times -= 2; //因为下面又++了,所以这里要减2
  }
  for (int count = 1;
       FindForCount(
           count,
           drive_ctl.drives[task->drive_number].directory_clustno_list) != NULL;
       count++) {
    struct List *list = FindForCount(
        count, drive_ctl.drives[task->drive_number].directory_clustno_list);
    if (list->val == finfo->clustno) {
      list = FindForCount(count,
                          drive_ctl.drives[task->drive_number].directory_list);
      task->directory = (struct FILEINFO *)list->val;
      break;
    }
  }
  task->change_dict_times++;
  return 0;
}
int rename(char *src_name, char *dst_name) {
  strtoupper(src_name);
  strtoupper(dst_name);
  struct TASK *task = NowTask();
  char name[9], ext[4];
  int i;
  clean(name, 9);
  clean(ext, 4);
  for (i = 0; dst_name[i] != '.'; i++) {
    name[i] = dst_name[i];
  }
  i++;
  for (int j = 0; i != strlen(dst_name); i++, j++) {
    ext[j] = dst_name[i];
  }
  struct FILEINFO *finfo = Get_File_Address(src_name);
  if (finfo == 0 || finfo->type == 0x01 || finfo->type == 0x04) {
    return -1;
  }
  int drive_number;
  if (strncmp(src_name + 1, ":\\", 2) == 0 || strncmp(src_name + 1, ":/", 2) == 0) {
    drive_number = *src_name - 0x41;
  } else {
    drive_number = task->drive_number;
  }
  memset((void *)finfo->name, ' ', 11);
  for (i = 0; i != strlen(name); i++) {
    finfo->name[i] = name[i];
  }
  for (i = 0; i != strlen(ext); i++) {
    finfo->ext[i] = ext[i];
  }
  file_saveinfo(Get_dictaddr(src_name), drive_number);
  return 0;
}
int format(char drive) {
  // A,B盘——软盘
  // C盘——IDE/SATA硬盘主分区
  // D,E,F...盘——IDE/USB/SATA存储介质/分区/虚拟磁盘
  FILE *fp = fopen("tskdrv:\\boot.bin", "r");
  if (fp == 0) {
    return -1;
  }
  void *read_in = page_malloc(fp->size);
  fread(read_in, fp->size, 1, fp);
  if (!(drive - 'A')) {
    // printf("3K FloppyDisk: %d bytes\n", 2880 * 512);
    // printf("INT 13H DriveNumber: 0\n");
    // printf("RootDictFiles: 224\n");
    // printf("drive_ctl.drives[%d].ClustnoBytes: 512 "
    //        "bytes\n",
    //        NowTask()->drive_number);
    *(unsigned char *)(&((unsigned char *)read_in)[BPB_SecPerClus]) = 1;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_RootEntCnt]) = 224;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) = 2880;
    *(unsigned int *)(&((unsigned char *)read_in)[BPB_TotSec32]) = 2880;
    *(unsigned char *)(&((unsigned char *)read_in)[BS_DrvNum]) = 0;
    write_floppy_for_ths(0, 0, 1, read_in, 1);
    unsigned int *fat = (unsigned int *)page_malloc(9 * 512);
    fat[0] = 0x00fffff0;
    write_floppy_for_ths(0, 0, 2, (unsigned char *)fat, 9);
    write_floppy_for_ths(0, 0, 11, (unsigned char *)fat, 9);
    page_free((void *)fat, 9 * 512);
    void *null_sec = page_malloc(512);
    for (int i = 0; i < 224 * 32 / 512; i++) {
      write_floppy_for_ths(0, 0, 20 + i, null_sec, 1);
    }
    page_free(null_sec, 512);
  } else if (drive != 'B') {
    // struct IDEHardDiskInfomationBlock* info = drivers_idehdd_info();
    // printk("drive=%c %d\n", drive, have_vdisk(drive));
    if (!have_vdisk(drive) && !DiskReady(drive)) {
      // printf("Couldn't find Disk.\n");
      return 1;
    }
    if (DiskReady(drive)) {
      // printf("IDE HardDisk ID:%s\n", ide_devices[drive - 'C'].Model);
    }

    // printf("Disk: %d bytes\n", disk_Size(drive));
    // printf("RootDictFiles: %d\n",
    //        14 * (((disk_Size(drive) / 4096) / 512 + 1) * 512) / 32);
    // printf("ClustnoBytes: %d bytes\n",
    //        ((disk_Size(drive) / 4096) / 512 + 1) * 512);
    *(unsigned char *)(&((unsigned char *)read_in)[BPB_SecPerClus]) =
        ((disk_Size(drive) / 4096) / 512 + 1);
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_RootEntCnt]) =
        14 * (((disk_Size(drive) / 4096) / 512 + 1) * 512) / 32;
    // printk("Sectors:%d\n", ide_devices[drive - 'C'].Size /power
    if (disk_Size(drive) / 512 > 65535) {
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) = 0;
    } else {
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) =
          disk_Size(drive) / 512;
    }
    *(unsigned int *)(&((unsigned char *)read_in)[BPB_TotSec32]) =
        disk_Size(drive) / 512;
    *(unsigned char *)(&((unsigned char *)read_in)[BS_DrvNum]) =
        drive - 'C' + 0x80;
    Disk_Write(0, 1, (unsigned short *)read_in, drive);
    unsigned int *fat = (unsigned int *)page_malloc(9 * 512);
    fat[0] = 0x00fffff0;
    Disk_Write(1, 9, (unsigned short *)fat, drive);
    Disk_Write(10, 9, (unsigned short *)fat, drive);
    page_free((void *)fat, 9 * 512);
    void *null_sec = page_malloc(512);
    clean((char *)null_sec, 512);
    for (int i = 0;
         i < 14 * (((disk_Size(drive) / 4096) / 512 + 1) * 512) / 32 * 32 / 512;
         i++) {
      Disk_Write(19, 1, (unsigned short *)null_sec, drive);
    }
    page_free(null_sec, 512);
    // page_free((void*)info, 256 * sizeof(short));
  }
  page_free(read_in, fp->size);
  fclose(fp);
  return 0;
}
int attrib(char *filename, char type) {
  struct TASK *task = NowTask();
  struct FILEINFO *finfo = Get_File_Address(filename);
  int drive_number;
  if (strncmp(filename + 1, ":\\", 2) == 0 || strncmp(filename + 1, ":/", 2) == 0) {
    drive_number = *filename - 0x41;
  } else {
    drive_number = task->drive_number;
  }
  if (finfo == 0) {
    return -1;
  }
  finfo->type = type;
  file_saveinfo(Get_dictaddr(filename), drive_number);
}

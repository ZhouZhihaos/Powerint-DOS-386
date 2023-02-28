#include <dosldr.h>
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
  clustno = tmp;
  int alloc_size;
  int old_clustno = clustno + clustall;
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
  for (int i = 0; i != alloc_size / drive_ctl.drives[drive_number].SectorBytes;
       i++) {
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
             i = clustno + clustall;
         size1 > size;
         size1 -= drive_ctl.drives[drive_number].ClustnoBytes, i--) {
      fat[i] = 0;
      ff[i] = false;
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
   // printf("path1=%s\n", path1);
    struct TASK *task = NowTask();
    struct FILEINFO *bmpDict = task->directory;
    int drive_number = task->drive_number;
    char *path = (char *)page_malloc(strlen(path1) + 1);
    strcpy(path, path1);
    strtoupper(path);
    if (strncmp(":\\", path + 1, 2) == 0 || strncmp(":/", path + 1, 2) == 0) {
        drive_number = *path - 0x41;
        path += 3;
        bmpDict = drive_ctl.drives[drive_number].root_directory;
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
      if (path[i] != '\0') {
        page_free((int)temp_name, 128);
        page_free((int)path, strlen(path1) + 1);
        return 0;
      }
      finfo = file_search(temp_name, bmpDict,
                          drive_ctl.drives[drive_number].RootMaxFiles);
      if (finfo == 0) {
        //printf("Invalid file:%s\n", temp_name);
        page_free((int)temp_name, 128);
        page_free((int)path, strlen(path1) + 1);
        return 0;
      } else {
        goto END;
      }
    } else {
      // printf("dict_search:%s ", temp_name);
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
            // printf("finfo:%08x\n", bmpDict);
            break;
          }
        }
      } else {
        // printf("finfo:%08x\n",
        // drive_ctl.drives[drive_number].root_directory);
        bmpDict = drive_ctl.drives[drive_number].root_directory;
      }
      clean(temp_name, 128);
    }
  }
END:
  // printf("file_search:%s finfo:%08x\n", temp_name, finfo);
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
  if (strncmp(":\\", path + 1, 2) == 0 || strncmp(":/", path + 1, 2) == 0) {
    drive_number = *path - 0x41;
    path += 3;
    bmpDict = drive_ctl.drives[drive_number].root_directory;
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
      // printf("dict_search:%s\n", temp_name);
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
            break;
          }
        }
      } else {
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
  return bmpDict;
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
  file_saveinfo(Get_dictaddr(dictname), drive_number);
  file_savefat(drive_ctl.drives[drive_number].fat, drive_number);
  AddVal(finfo->clustno, drive_ctl.drives[drive_number].directory_clustno_list);
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
void Copy(char *path, char *path1) {
  mkfile(path1); //创建文件
  struct FILEINFO *finfo = Get_File_Address(path1);
  struct FILEINFO *finfo1 = Get_File_Address(path);
  if (!finfo1) {
    return;
  }
  FILE *fp = fopen(path, "wb");
  FILE *fp1 = fopen(path1, "wb");
  // 拷贝文件内容
  for (int i = 0; i != fp->size; i++) {
    fputc(fp->buf[i], fp1);
  }
  fclose(fp);
  fclose(fp1);
  // 拷贝文件属性
  finfo->date = finfo1->date;
  finfo->time = finfo1->time;
  finfo->type = finfo1->type;
  struct FILEINFO *dict = Get_dictaddr(path);
  struct FILEINFO *dict1 = Get_dictaddr(path1);
  if (path[1] == path1[1] && path[1] == ':' && path[2] == path1[2] &&
      (path[2] == '\\' || path[2] == '/')) {
    if (*path != *path1 &&
        (*path != NowTask()->drive || *path1 != NowTask()->drive)) {
      int dn1 = *path - 0x41;
      int dn2 = *path1 - 0x41;
      file_saveinfo(dict, dn1);
      file_saveinfo(dict1, dn2);
      return;
    }
  }
  file_saveinfo(dict, NowTask()->drive_number);
  file_saveinfo(dict1, NowTask()->drive_number);
  return;
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
void del(char *cmdline) {
  //删除某个文件
  struct TASK *task = NowTask();
  char name[30];
  int i;
  struct FILEINFO *finfo;
  for (i = 0; i < strlen(cmdline); i++) {
    name[i] = cmdline[i + 4];
  }
  finfo = Get_File_Address(name);
  if (finfo == 0) {
    print(name);
    print(" not found!\n\n");
    return;
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
  return;
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
void changedict(char *dictname) {
  // cd命令的依赖函数
  struct TASK *task = NowTask();
  struct FILEINFO *finfo =
      dict_search(dictname, task->directory,
                  drive_ctl.drives[task->drive_number].RootMaxFiles);
  //找文件夹
  if (finfo == 0) {
    //没找到
    print("Invalid directory.\n");
    return;
  }
  if (finfo->clustno == 0) {
    //根目录
    strcpy(task->path, "");
    task->change_dict_times = 0;
    task->directory = drive_ctl.drives[task->drive_number].root_directory;
    return;
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
  return;
}
void rename(char *src_name, char *dst_name) {
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
  memset((void *)finfo->name, ' ', 11);
  for (i = 0; i != strlen(name); i++) {
    finfo->name[i] = name[i];
  }
  for (i = 0; i != strlen(ext); i++) {
    finfo->ext[i] = ext[i];
  }
  file_saveinfo(task->directory, task->drive_number);
  return;
}
void clean(char* s, int len) {
  //清理某个内存区域（全部置0）
  int i;
  for (i = 0; i != len; i++) {
    s[i] = 0;
  }
  return;
}
void strtoupper(char* str) {
  while (*str != '\0') {
    if (*str >= 'a' && *str <= 'z') {
      *str -= 32;
    }
    str++;
  }
}
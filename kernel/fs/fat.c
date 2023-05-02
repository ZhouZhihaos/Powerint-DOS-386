// fat.c : fat文件系统的实现
#include <dos.h>
#include <fs.h>
static inline int get_fat_date(unsigned short year, unsigned short month,
                               unsigned short day) {
  year -= 1980;
  unsigned short date = 0;
  date |= (year & 0x7f) << 9;
  date |= (month & 0x0f) << 5;
  date |= (day & 0x1f);
  return date;
}
static inline int get_fat_time(unsigned short hour, unsigned short minute) {
  unsigned short time = 0;
  time |= (hour & 0x1f) << 11;
  time |= (minute & 0x3f) << 5;
  return time;
}
void read_fat(unsigned char *img, int *fat, unsigned char *ff, int max,
              int type) {
  if (type == 12) {
    for (int i = 0, j = 0; i < max; i += 2) {
      fat[i + 0] = (img[j + 0] | img[j + 1] << 8) & 0xfff;
      fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
      j += 3;
    }
    // 保留簇
    ff[0] = true;
    ff[1] = true;
    for (int i = 1; i < max; i++) {
      if (fat[i] > 0 && fat[i] < 0xff0) {
        ff[fat[i]] = true;
      } else if (fat[i] >= 0xff0 && fat[i] <= 0xfff) {
        ff[i + 1] = true;
      }
    }
  } else if (type == 16) {
    unsigned short *p = (unsigned short *)img;
    for (int i = 0; i != max; i++) {
      fat[i] = p[i];
    }
    ff[0] = true;
    ff[1] = true;
    for (int i = 1; i < max; i++) {
      if (fat[i] > 0 && fat[i] < 0xfff0) {
        ff[fat[i]] = true;
      } else if (fat[i] >= 0xfff0 && fat[i] <= 0xffff) {
        ff[i + 1] = true;
      }
    }
  } else if (type == 32) {
    unsigned int *p = (unsigned int *)img;
    for (int i = 0; i != max; i++) {
      fat[i] = p[i];
    }
    ff[0] = true;
    ff[1] = true;
    for (int i = 1; i < max; i++) {
      if (fat[i] > 0 && fat[i] < 0xffffff0) {
        ff[fat[i]] = true;
      } else if (fat[i] >= 0xffffff0 && fat[i] <= 0xfffffff) {
        ff[i + 1] = true;
      }
    }
  }
  return;
}
void write_fat(unsigned char *img, int *fat, int max, int type) {
  if (type == 12) {
    for (int i = 0, j = 0; i < max; i += 2) {
      img[j + 0] = fat[i + 0] & 0xff;
      img[j + 1] = (fat[i + 0] >> 8 | fat[i + 1] << 4) & 0xff;
      img[j + 2] = (fat[i + 1] >> 4) & 0xff;
      j += 3;
    }
  } else if (type == 16) {
    for (int i = 0, j = 0; i < max; i++) {
      img[j + 0] = fat[i] & 0xff;
      img[j + 1] = (fat[i] >> 8) & 0xff;
      j += 2;
    }
  } else if (type == 32) {
    for (int i = 0, j = 0; i < max; i++) {
      img[j + 0] = fat[i] & 0xff;
      img[j + 1] = (fat[i] >> 8) & 0xff;
      img[j + 2] = (fat[i] >> 16) & 0xff;
      img[j + 3] = fat[i] >> 24;
      j += 4;
    }
  }
  return;
}
void file_loadfile(int clustno, int size, char *buf, int *fat, vfs_t *vfs) {
  if (!size) {
    return;
  }
  void *img = page_malloc(((size - 1) / get_dm(vfs).ClustnoBytes + 1) *
                          get_dm(vfs).ClustnoBytes);
  for (int i = 0; i != (size - 1) / get_dm(vfs).ClustnoBytes + 1; i++) {
    uint32_t sec = (get_dm(vfs).FileDataAddress +
                    (clustno - 2) * get_dm(vfs).ClustnoBytes) /
                   get_dm(vfs).SectorBytes;
    Disk_Read(sec, get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
              img + i * get_dm(vfs).ClustnoBytes, vfs->disk_number);
    clustno = fat[clustno];
  }
  // printk("done 1 %d\n", size);
  memcpy((void *)buf, img, size);
  page_free(img, ((size - 1) / get_dm(vfs).SectorBytes + 1) *
                     get_dm(vfs).SectorBytes);
  // printk("done 2\n");
  return;
}
void file_savefile(int clustno, int size, char *buf, int *fat,
                   unsigned char *ff, vfs_t *vfs) {
  uint32_t clustall = 0;
  int tmp = clustno;
  int end = clustno_end(get_dm(vfs).type);
  while (fat[clustno] != end) { // 计算文件占多少Fat项 Fat项 = 大小 / 簇大小 + 1
    clustno = fat[clustno];
    clustall++;
  }
  int old_clustno = clustno;
  clustno = tmp;
  int alloc_size;
  if (size >
      (clustall + 1) *
          get_dm(vfs).ClustnoBytes) { // 新大小 > (旧大小 / 簇大小 + 1) * 簇大小
    // 请求内存大小 = (新大小 / 簇大小 + 1) * 簇大小
    alloc_size =
        ((size - 1) / get_dm(vfs).ClustnoBytes + 1) * get_dm(vfs).ClustnoBytes;
    // 分配Fat（这里需要在写盘前分配）
    for (int size1 = size; size1 > ((clustall + 1) * get_dm(vfs).ClustnoBytes);
         size1 -= get_dm(vfs).ClustnoBytes) {
      for (int i = 0; i != get_dm(NowTask()->nfs).FatMaxTerms; i++) {
        if (!ff[i]) {
          fat[old_clustno] = i;
          old_clustno = i;
          ff[i] = true;
          break;
        }
      }
    }
    fat[old_clustno] = end; // 结尾Fat
    ff[old_clustno] = true;
  } else if (size <=
             (clustall + 1) *
                 get_dm(vfs).ClustnoBytes) { // 新大小 <= (旧大小 / 簇大小
                                             // + 1) * 簇大小
    // 请求内存大小 = (旧大小 / 簇大小 + 1) * 簇大小
    alloc_size = (clustall + 1) * get_dm(vfs).ClustnoBytes;
    // 这里不分配Fat的原因是要清空更改后多余的数据
  }
  void *img = page_malloc(alloc_size);
  clean((char *)img, alloc_size);
  memcpy(img, (void *)buf, size); // 把要写入的数据复制到新请求的内存地址
  for (int i = 0; i != (alloc_size / get_dm(vfs).ClustnoBytes); i++) {
    // 计算LBA & 写盘
    uint32_t sec = (get_dm(vfs).FileDataAddress +
                    (clustno - 2) * get_dm(vfs).ClustnoBytes) /
                   get_dm(vfs).SectorBytes;
    Disk_Write(sec, get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
               img + i * get_dm(vfs).ClustnoBytes, vfs->disk_number);
    clustno = fat[clustno];
  }
  page_free(img, alloc_size);
  if (size <
      clustall *
          get_dm(vfs).ClustnoBytes) { // 新大小 < (旧大小 / 簇大小) * 簇大小
    // 分配Fat（中间情况没必要分配）
    int i;
    for (int size1 = clustall * get_dm(vfs).ClustnoBytes, i = old_clustno;
         size1 > size; size1 -= get_dm(vfs).ClustnoBytes) {
      fat[i] = 0;
      ff[i] = false;
      for (int j = 0; j != get_dm(NowTask()->nfs).FatMaxTerms; j++)
        if (fat[j] == i)
          i = j;
    }
    old_clustno = i;
    fat[old_clustno] = end;
    ff[old_clustno] = true;
  }
  file_savefat(fat, vfs);
}
void file_saveinfo(struct FAT_FILEINFO *directory, vfs_t *vfs) {
  if (directory == get_dm(vfs).root_directory) {
    Disk_Write(get_dm(vfs).RootDictAddress / get_dm(vfs).SectorBytes,
               14 * get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
               (void *)directory, vfs->disk_number);
  } else {
    for (int i = 1; FindForCount(i, get_dm(vfs).directory_list) != NULL; i++) {
      struct List *list = FindForCount(i, get_dm(vfs).directory_list);
      if (list->val == directory) {
        list = FindForCount(i, get_dm(vfs).directory_clustno_list);
        Disk_Write((get_dm(vfs).FileDataAddress +
                    (list->val - 2) * get_dm(vfs).ClustnoBytes) /
                       get_dm(vfs).SectorBytes,
                   get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
                   (void *)directory, vfs->disk_number);
        break;
      }
    }
  }
}
void file_savefat(int *fat, vfs_t *vfs) {
  write_fat(get_dm(vfs).ADR_DISKIMG + get_dm(vfs).Fat1Address, fat,
            get_dm(vfs).FatMaxTerms, get_dm(vfs).type);
  write_fat(get_dm(vfs).ADR_DISKIMG + get_dm(vfs).Fat2Address, fat,
            get_dm(vfs).FatMaxTerms, get_dm(vfs).type);
  Disk_Write(get_dm(vfs).Fat1Address / get_dm(vfs).SectorBytes,
             (get_dm(vfs).Fat2Address - get_dm(vfs).Fat1Address) /
                 get_dm(vfs).SectorBytes,
             get_dm(vfs).ADR_DISKIMG + get_dm(vfs).Fat2Address,
             vfs->disk_number);
  Disk_Write(get_dm(vfs).Fat1Address / get_dm(vfs).SectorBytes,
             (get_dm(vfs).Fat2Address - get_dm(vfs).Fat1Address) /
                 get_dm(vfs).SectorBytes,
             get_dm(vfs).ADR_DISKIMG + get_dm(vfs).Fat2Address,
             vfs->disk_number);
}
struct FAT_FILEINFO *file_search(char *name, struct FAT_FILEINFO *finfo,
                                 int max) {
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
struct FAT_FILEINFO *dict_search(char *name, struct FAT_FILEINFO *finfo,
                                 int max) {
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
struct FAT_FILEINFO *Get_File_Address(char *path1, vfs_t *vfs) {
  // TODO: Modifly it
  struct FAT_FILEINFO *bmpDict = get_now_dir(vfs);
  int drive_number = vfs->disk_number;
  // printk("Get_File_Address :%s\n", path1);
  char *path = (char *)page_malloc(strlen(path1) + 1);
  strcpy(path, path1);
  strtoupper(path);
  if (strncmp("/", path, 1) == 0) {
    path += 1;
    bmpDict = get_dm(vfs).root_directory;
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
  struct FAT_FILEINFO *finfo;
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
    finfo = dict_search(temp_name, bmpDict, get_dm(vfs).RootMaxFiles);
    if (finfo == 0) {
      if (path[i] != '\0') {
        page_free((int)temp_name, 128);
        page_free((int)path, strlen(path1) + 1);
        return 0;
      }
      finfo = file_search(temp_name, bmpDict, get_dm(vfs).RootMaxFiles);
      if (finfo == 0) {
        page_free((int)temp_name, 128);
        page_free((int)path, strlen(path1) + 1);
        return 0;
      } else {
        goto END;
      }
    } else {
      // printk("dict_search:%s ", temp_name);
      if (get_clustno(finfo->clustno_high, finfo->clustno_low) != 0) {
        for (int count = 1;
             FindForCount(count, get_dm(vfs).directory_clustno_list) != NULL;
             count++) {
          struct List *list =
              FindForCount(count, get_dm(vfs).directory_clustno_list);
          if (list->val ==
              get_clustno(finfo->clustno_high, finfo->clustno_low)) {
            list = FindForCount(count, get_dm(vfs).directory_list);
            bmpDict = (struct FAT_FILEINFO *)list->val;
            // printk("finfo:%08x\n", bmpDict);
            break;
          }
        }
      } else {
        // printk("finfo:%08x\n",
        // get_dm(vfs).root_directory);
        bmpDict = get_dm(vfs).root_directory;
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
struct FAT_FILEINFO *Get_dictaddr(char *path1, vfs_t *vfs) {
  // TODO: Modifly it
  struct FAT_FILEINFO *bmpDict = get_now_dir(vfs);
  int drive_number = vfs->disk_number;
  char *path = (char *)page_malloc(strlen(path1) + 1);
  strcpy(path, path1);
  strtoupper(path);
  if (strncmp("/", path, 1) == 0) {
    path += 1;
    bmpDict = get_dm(vfs).root_directory;
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
  struct FAT_FILEINFO *finfo;
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
    finfo = dict_search(temp_name, bmpDict, get_dm(vfs).RootMaxFiles);
    if (finfo == 0) {
      goto END;
    } else {
      // printf("dict_search:%s drv_num=%02x\n", temp_name, vfs->disk_number);
      if (get_clustno(finfo->clustno_high, finfo->clustno_low) != 0) {
        for (int count = 1;
             FindForCount(count, get_dm(vfs).directory_clustno_list) != NULL;
             count++) {
          struct List *list =
              FindForCount(count, get_dm(vfs).directory_clustno_list);
          if (list->val ==
              get_clustno(finfo->clustno_high, finfo->clustno_low)) {
            list = FindForCount(count, get_dm(vfs).directory_list);
            // printf("change %08x to", bmpDict);
            bmpDict = (struct FAT_FILEINFO *)list->val;
            // printf("%08x\n", bmpDict);

            break;
          } else {
            // printf("list->val = %08x finfo->clustno_low = %08x %08x\n",
            // list->val,
            //   finfo->clustno_low,
            //   get_dm(vfs).directory_clustno_list);
          }
        }
        // printf(
        //     "Not Found %08x,and "
        //     "FindForCount(count,get_dm(vfs).directory_"
        //     "clustno_list)=%08x %08x\n",
        //     finfo->clustno_low,
        //     FindForCount(
        //         1,
        //         get_dm(vfs).directory_clustno_list),get_dm(vfs).directory_clustno_list);
      } else {
        // printf("finfo->clustno_low == 0\n");
        bmpDict = get_dm(vfs).root_directory;
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
  //       get_dm(vfs).root_directory);
  return bmpDict;
}
void mkdir(char *dictname, int last_clust, vfs_t *vfs) {
  /*
          dictname:目录名
          last_clust:上一级目录的簇号
  */
  mkfile(dictname, vfs);
  // 三个目录项（模板）
  static struct FAT_FILEINFO model[3];
  // .目录项，指向自己
  // ..目录项，指向上一级目录
  // 空目录项（为mkfile函数提供指引）
  model[0].reserve = 0;
  model[1].reserve = 0;
  model[2].reserve = 0;
  memcpy(model[2].name, "NULL       ", 11);
  model[2].type = 0x20; // 文件的type属性是0x20
  // 将size date time这类属性全部设置为0
  model[2].size = 0;
  model[2].update_date =
      get_fat_date(get_year(), get_mon_hex(), get_day_of_month());
  model[2].update_time = get_fat_time(get_hour_hex(), get_min_hex());
  for (int i = 0; i != get_dm(NowTask()->nfs).FatMaxTerms; i++) {
    if (!get_dm(vfs).fat[i]) {
      model[2].clustno_low = i;
      int end = clustno_end(get_dm(vfs).type);
      get_dm(vfs).fat[i] = end;
      get_dm(vfs).FatClustnoFlags[i] = true;
      break;
    }
  }
  model[0].name[0] = '.';
  for (int i = 1; i != 8; i++) {
    model[0].name[i] = ' ';
  }
  for (int i = 0; i != 3; i++) {
    model[0].ext[i] = ' ';
  }
  struct FAT_FILEINFO *finfo1 = Get_File_Address(dictname, vfs);
  model[0].type = 0x10;
  model[0].clustno_low = finfo1->clustno_low; // 指向自己
  model[0].clustno_high = finfo1->clustno_high;
  model[0].size = 0;
  model[0].update_date =
      get_fat_date(get_year(), get_mon_hex(), get_day_of_month());
  model[0].update_time = get_fat_time(get_hour_hex(), get_min_hex());
  model[1].name[0] = '.';
  model[1].name[1] = '.';
  for (int i = 2; i != 8; i++) {
    model[1].name[i] = ' ';
  }
  for (int i = 0; i != 3; i++) {
    model[1].ext[i] = ' ';
  }
  model[1].clustno_low = last_clust;
  model[1].size = 0;
  model[1].update_date =
      get_fat_date(get_year(), get_mon_hex(), get_day_of_month());
  model[1].update_time = get_fat_time(get_hour_hex(), get_min_hex());
  model[1].type = 0x10;
  Fat_WriteFile(vfs, dictname, (char *)&model[0], 32 * 3);
  struct FAT_FILEINFO *finfo = Get_File_Address(dictname, vfs);
  finfo->type = 0x10; // 是目录（文件夹的type属性是0x10）
  finfo->size = 0;
  int drive_number = vfs->disk_number;
  AddVal(get_clustno(finfo->clustno_high, finfo->clustno_low),
         get_dm(vfs).directory_clustno_list);
  // printf("Add = %08x %02x %08x\n", finfo->clustno_low, vfs->disk_number,
  //        get_dm(vfs).directory_clustno_list);

  // for (int count = 1;
  //      FindForCount(count,
  //                   get_dm(vfs).directory_clustno_list) !=
  //      NULL;
  //      count++) {
  //   printf("%02x ",
  //          FindForCount(count,
  //                       get_dm(vfs).directory_clustno_list)
  //              ->val);
  // }
  if (last_clust == 0) {
    file_saveinfo(get_dm(vfs).root_directory, vfs);
  } else {
    for (int i = 1; FindForCount(i, get_dm(vfs).directory_clustno_list) != NULL;
         i++) {
      struct List *list = FindForCount(i, get_dm(vfs).directory_clustno_list);
      if (list->val == last_clust) {
        list = FindForCount(i, get_dm(vfs).directory_list);
        struct FAT_FILEINFO *d_finfo = (struct FAT_FILEINFO *)list->val;
        file_saveinfo(d_finfo, vfs);
      }
    }
  }
  file_savefat(get_dm(vfs).fat, vfs);

  void *directory_alloc = page_malloc(get_dm(vfs).ClustnoBytes);
  Disk_Read((get_dm(vfs).FileDataAddress +
             (get_clustno(finfo->clustno_high, finfo->clustno_low) - 2) *
                 get_dm(vfs).ClustnoBytes) /
                get_dm(vfs).SectorBytes,
            get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes, directory_alloc,
            vfs->disk_number);
  AddVal(directory_alloc, get_dm(vfs).directory_list);
  return;
}

struct FAT_FILEINFO *clust_sech(int clustno, struct FAT_FILEINFO *finfo,
                                int max) {
  //通过簇号找到文件信息
  int i, j;
  j = 0;
  for (i = 0; i < max; i++) {
    if (finfo[i].clustno_low == clustno) {
      return finfo + i;
    }
  }
  return 0; /*没找到*/
}
int del(char *cmdline, vfs_t *vfs) {
  //删除某个文件
  char *name = cmdline;
  int i;
  struct FAT_FILEINFO *finfo;
  // for (i = 0; i < strlen(cmdline); i++) {
  //   name[i] = cmdline[i + 4];
  // }
  finfo = Get_File_Address(name, vfs);
  if (finfo == 0 || finfo->type == 0x01 || finfo->type == 0x04) {
    return 0;
  }
  // FILE* fp = fopen(name, "wb");
  // for (i = 0; i != fp->size; i++) {
  //   fputc(0, fp);
  // }
  // fp->size = 0;
  // fclose(fp);
  // char* buf = malloc(finfo->size);
  // memset(buf, 0, finfo->size);
  // Fat_WriteFile(vfs, name, buf, finfo->size);
  finfo->name[0] = 0xe5;
  get_dm(vfs).fat[get_clustno(finfo->clustno_high, finfo->clustno_low)] = 0;
  get_dm(vfs)
      .FatClustnoFlags[get_clustno(finfo->clustno_high, finfo->clustno_low)] =
      false;
  file_saveinfo(Get_dictaddr(name, vfs), vfs);
  file_savefat(get_dm(vfs).fat, vfs);
  return 1;
}
int deldir(char *path, vfs_t *vfs) {
  struct FAT_FILEINFO *finfo = Get_dictaddr(path, vfs);
  if (finfo == Get_dictaddr(".", vfs)) {
    return 0;
  }
  struct FAT_FILEINFO *f = get_now_dir(vfs);
  get_now_dir(vfs) = finfo;
  for (int i = 2; finfo[i].name[0] != '\0'; i++) {
    if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
      char s[30];
      int j = 0;
      for (; finfo[i].name[j] != ' '; j++) {
        s[j] = finfo[i].name[j];
      }
      s[j] = '\0';
      // printf("(CALL)DEL DIR:%s\n", s);
      if (deldir(s, vfs) == -1) {
        return 0;
      }
      // return -1;
    }
  }
  for (int i = 2; finfo[i].name[0] != '\0'; i++) {
    if (finfo[i].name[0] != 0xe5 && finfo[i].type != 0x10) {
      char s[30];
      int p = 0;
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
      if (del(s, vfs) == -1) {
        return 0;
      }
    }
  }
  get_now_dir(vfs) = f;
  // printf("(IN)DEL SELF\n");
  struct FAT_FILEINFO *root_finfo;
  if (finfo[1].clustno_low == 0) {
    root_finfo = get_dm(vfs).root_directory;
  } else {
    for (int i = 1; FindForCount(i, get_dm(vfs).directory_clustno_list) != NULL;
         i++) {
      if (FindForCount(i, get_dm(vfs).directory_clustno_list)->val ==
          finfo[1].clustno_low) {
        root_finfo =
            (struct FAT_FILEINFO *)FindForCount(i, get_dm(vfs).directory_list)
                ->val;
        // printf("FIND ROOT %08x\n", root_finfo);
      }
    }
  }
  for (int i = 0; root_finfo[i].name[0] != '\0'; i++) {
    // printf("ROOT FILE:%s\n", root_finfo[i].name);
    if (root_finfo[i].clustno_low == finfo[0].clustno_low) {
      root_finfo[i].name[0] = 0xe5;
      break;
    }
  }
  get_dm(vfs).fat[get_clustno(finfo->clustno_high, finfo->clustno_low)] = 0;
  get_dm(vfs)
      .FatClustnoFlags[get_clustno(finfo->clustno_high, finfo->clustno_low)] =
      false;
  file_saveinfo(Get_dictaddr(path, vfs), vfs);
  file_savefat(get_dm(vfs).fat, vfs);
  return 1;
}
void mkfile(char *name, vfs_t *vfs) {
  char s[12];
  int i, j;
  struct FAT_FILEINFO *finfo = Get_dictaddr(name, vfs);

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
  for (int i = 0; i != get_dm(NowTask()->nfs).FatMaxTerms; i++) {
    if (!get_dm(vfs).fat[i]) {
      finfo->clustno_low = i & 0xffff;
      finfo->clustno_high = i >> 16;
      int end = clustno_end(get_dm(vfs).type);
      get_dm(vfs).fat[i] = end;
      get_dm(vfs).FatClustnoFlags[i] = true;
      break;
    }
  }
  finfo->reserve = 0;
  finfo->update_time = get_fat_time(get_hour_hex(), get_min_hex());
  finfo->update_date =
      get_fat_date(get_year(), get_mon_hex(), get_day_of_month());
  finfo->size = 0;
  file_saveinfo(Get_dictaddr(path, vfs), vfs);
  file_savefat(get_dm(vfs).fat, vfs);
  return;
}
int changedict(char *dictname, vfs_t *vfs) {
  // cd命令的依赖函数
  strtoupper(dictname);
  struct TASK *task = NowTask();

  if (strcmp(dictname, "/") == 0) {
    while (vfs->path->ctl->all != 0) {
      page_kfree(FindForCount(vfs->path->ctl->all, vfs->path)->val, 255);
      DeleteVal(vfs->path->ctl->all, vfs->path);
    }
    get_now_dir(vfs) = get_dm(vfs).root_directory;
    return 1;
  }
  struct FAT_FILEINFO *finfo =
      dict_search(dictname, get_now_dir(vfs), get_dm(vfs).RootMaxFiles);
  //找文件夹
  if (finfo == 0) {
    //没找到
    return 0;
  }
  if (get_clustno(finfo->clustno_high, finfo->clustno_low) == 0) {
    //根目录
    while (vfs->path->ctl->all != 0) {
      page_kfree(FindForCount(vfs->path->ctl->all, vfs->path)->val, 255);
      DeleteVal(vfs->path->ctl->all, vfs->path);
    }
    get_now_dir(vfs) = get_dm(vfs).root_directory;
    return 1;
  }
  //..进行特殊的处理
  //.不进行处理
  //其他按照下面的方式处理
  if (strcmp(dictname, "..") != 0 && strcmp(dictname, ".") != 0) {
    char *dict = page_kmalloc(255);
    strcpy(dict, dictname);
    AddVal(dict, vfs->path);
  }

  if (strcmp(dictname, "..") == 0) {
    page_kfree(FindForCount(vfs->path->ctl->all, vfs->path)->val, 255);
    DeleteVal(vfs->path->ctl->all, vfs->path);
  }
  for (int count = 1;
       FindForCount(count, get_dm(vfs).directory_clustno_list) != NULL;
       count++) {
    struct List *list = FindForCount(count, get_dm(vfs).directory_clustno_list);
    if (list->val == get_clustno(finfo->clustno_high, finfo->clustno_low)) {
      list = FindForCount(count, get_dm(vfs).directory_list);
      get_now_dir(vfs) = (struct FAT_FILEINFO *)list->val;
      break;
    }
  }
  //  task->change_dict_times++;
  return 1;
}
int rename(char *src_name, char *dst_name, vfs_t *vfs) {
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
  struct FAT_FILEINFO *finfo = Get_File_Address(src_name, vfs);
  if (finfo == 0 || finfo->type == 0x01 || finfo->type == 0x04) {
    return 0;
  }
  memset((void *)finfo->name, ' ', 11);
  for (i = 0; i != strlen(name); i++) {
    finfo->name[i] = name[i];
  }
  for (i = 0; i != strlen(ext); i++) {
    finfo->ext[i] = ext[i];
  }
  file_saveinfo(Get_dictaddr(src_name, vfs), vfs);
  return 1;
}
int format(char drive) {
  // A,B盘——软盘
  // C盘——IDE/SATA硬盘主分区
  // D,E,F...盘——IDE/USB/SATA存储介质/分区/虚拟磁盘
  FILE *fp = fopen("/boot.bin", "r"); // or \boot.bin
  if (fp == 0) {
    return 0;
  }
  void *read_in = page_malloc(fp->fileSize);
  fread(read_in, fp->fileSize, 1, fp);
  if (!(drive - 'A')) {
    write_floppy_for_ths(0, 0, 1, read_in, 1);
    unsigned int *fat = (unsigned int *)page_malloc(9 * 512);
    fat[0] = 0x00fffff0;
    write_floppy_for_ths(0, 0, 2, (unsigned char *)fat, 9);
    write_floppy_for_ths(0, 0, 11, (unsigned char *)fat, 9);
    page_free((void *)fat, 9 * 512);
    void *null_sec = page_malloc(512);
    for (int i = 0; i < 224 * 32 / 512; i++) {
      int t, h, s;
      block2hts(19 + i, &t, &h, &s);
      write_floppy_for_ths(t, h, s, null_sec, 1);
    }
    page_free(null_sec, 512);
  } else if (drive != 'B') {
    if (!DiskReady(drive)) {
      return 0;
    }
    if (disk_Size(drive) <= 2097152) { // 2MB及以下 fat12
      *(unsigned char *)(&((unsigned char *)read_in)[BPB_SecPerClus]) = 1;
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_RootEntCnt]) = 224;
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) =
          disk_Size(drive) / 512;
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec32]) = 0;
      unsigned short fatsz = (disk_Size(drive) - 1) / 512 * 3 / 2 / 512 + 1;
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_FATSz16]) = fatsz;
      *(unsigned char *)(&((unsigned char *)read_in)[BS_DrvNum]) =
          drive - 'C' + 0x80;
      memcpy((void *)&((unsigned char *)read_in)[BS_FileSysType],
             (void *)"FAT12   ", 8);
      Disk_Write(0, 1, (unsigned short *)read_in, drive);
      unsigned int *fat = (unsigned int *)page_malloc(fatsz * 512);
      fat[0] = 0x00fffff0;
      Disk_Write(1, fatsz, (unsigned short *)fat, drive);
      Disk_Write(1 + fatsz, fatsz, (unsigned short *)fat, drive);
      page_free((void *)fat, fatsz * 512);
      void *null_sec = page_malloc(512);
      clean((char *)null_sec, 512);
      for (int i = 0; i != 224 * 32 / 512; i++) {
        Disk_Write(1 + fatsz * 2 + i, 1, (unsigned short *)null_sec, drive);
      }
      page_free(null_sec, 512);
      // page_free((void*)info, 256 * sizeof(short));
    } else if (disk_Size(drive) > 2097152 &&
               disk_Size(drive) <= 134217728) { // 2MB~128MB fat16
      *(unsigned char *)(&((unsigned char *)read_in)[BPB_SecPerClus]) =
          disk_Size(drive) / 65536 / 512;
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_RootEntCnt]) =
          14 * (disk_Size(drive) / 65536) / 32;
      if (disk_Size(drive) / 512 > 65535) {
        *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) = 0;
      } else {
        *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) =
            disk_Size(drive) / 512;
      }
      *(unsigned int *)(&((unsigned char *)read_in)[BPB_TotSec32]) =
          disk_Size(drive) / 512;
      unsigned short fatsz =
          (disk_Size(drive) - 1) / (disk_Size(drive) / 65536) * 2 / 512 + 1;
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_FATSz16]) = fatsz;
      *(unsigned char *)(&((unsigned char *)read_in)[BS_DrvNum]) =
          drive - 'C' + 0x80;
      memcpy((void *)&((unsigned char *)read_in)[BS_FileSysType],
             (void *)"FAT16   ", 8);
      Disk_Write(0, 1, (unsigned short *)read_in, drive);
      unsigned short *fat = (unsigned short *)page_malloc(fatsz * 512);
      fat[0] = 0xfff0;
      fat[1] = 0xffff;
      Disk_Write(1, fatsz, (void *)fat, drive);
      Disk_Write(1 + fatsz, fatsz, (void *)fat, drive);
      page_free((void *)fat, fatsz * 512);
      void *null_sec = page_malloc(512);
      clean((char *)null_sec, 512);
      for (int i = 0; i < 14 * (disk_Size(drive) / 65536) / 512; i++) {
        Disk_Write(1 + fatsz * 2 + i, 1, (unsigned short *)null_sec, drive);
      }
      page_free(null_sec, 512);
    }
  }
  page_free(read_in, fp->fileSize);
  fclose(fp);
  return 1;
}
int attrib(char *filename, ftype type, struct vfs_t *vfs) {
  struct FAT_FILEINFO *finfo = Get_File_Address(filename, vfs);
  if (finfo == 0) {
    return 0;
  }
  if (type == FLE)
    finfo->type = 0x20;
  else if (type == RDO)
    finfo->type = 0x01;
  else if (type == HID)
    finfo->type = 0x02;
  else if (type == SYS)
    finfo->type = 0x04;
  else
    return 0;
  file_saveinfo(Get_dictaddr(filename, vfs), vfs);
  return 1;
}
void fat_InitFS(struct vfs_t *vfs, uint8_t disk_number) {
  vfs->cache = malloc(sizeof(fat_cache));
  void *boot_sector = page_malloc(512);
  Disk_Read(0, 1, boot_sector, disk_number);

  if (memcmp(boot_sector + BS_FileSysType, "FAT12   ", 8) == 0) {
    get_dm(vfs).type = 12;
  } else if (memcmp(boot_sector + BS_FileSysType, "FAT16   ", 8) == 0) {
    get_dm(vfs).type = 16;
  } else if (memcmp(boot_sector + BS_FileSysType + BPB_Fat32ExtByts, "FAT32   ",
                    8) == 0) {
    get_dm(vfs).type = 32;
  }
  get_dm(vfs).SectorBytes = *(unsigned short *)(boot_sector + BPB_BytsPerSec);
  get_dm(vfs).RootMaxFiles = *(unsigned short *)(boot_sector + BPB_RootEntCnt);
  get_dm(vfs).ClustnoBytes = get_dm(vfs).SectorBytes *
                             *(unsigned char *)(boot_sector + BPB_SecPerClus);
  get_dm(vfs).Fat1Address = *(unsigned short *)(boot_sector + BPB_RsvdSecCnt) *
                            get_dm(vfs).SectorBytes;
  if (get_dm(vfs).type != 32) {
    get_dm(vfs).RootDictAddress =
        (*(unsigned char *)(boot_sector + BPB_NumFATs) *
             *(unsigned short *)(boot_sector + BPB_FATSz16) +
         *(unsigned short *)(boot_sector + BPB_RsvdSecCnt)) *
        get_dm(vfs).SectorBytes;
    get_dm(vfs).FileDataAddress =
        get_dm(vfs).RootDictAddress + get_dm(vfs).RootMaxFiles * 32;
    if (*(unsigned short *)(boot_sector + BPB_TotSec16) != 0) {
      get_dm(vfs).imgTotalSize =
          *(unsigned short *)(boot_sector + BPB_TotSec16) *
          get_dm(vfs).SectorBytes;
    } else {
      get_dm(vfs).imgTotalSize = *(unsigned int *)(boot_sector + BPB_TotSec32) *
                                 get_dm(vfs).SectorBytes;
    }
    get_dm(vfs).Fat2Address = get_dm(vfs).Fat1Address +
                              *(unsigned short *)(boot_sector + BPB_FATSz16) *
                                  get_dm(vfs).SectorBytes;
  } else {
    get_dm(vfs).FileDataAddress =
        (*(unsigned char *)(boot_sector + BPB_NumFATs) *
             *(unsigned int *)(boot_sector + BPB_FATSz32) +
         *(unsigned short *)(boot_sector + BPB_RsvdSecCnt)) *
        get_dm(vfs).SectorBytes;
    get_dm(vfs).RootDictAddress =
        get_dm(vfs).FileDataAddress +
        (*(unsigned int *)(boot_sector + BPB_RootClus) - 2) *
            get_dm(vfs).ClustnoBytes;
    get_dm(vfs).imgTotalSize =
        *(unsigned int *)(boot_sector + BPB_TotSec32) * get_dm(vfs).SectorBytes;
    get_dm(vfs).Fat2Address =
        get_dm(vfs).Fat1Address +
        *(unsigned int *)(boot_sector + BPB_FATSz32) * get_dm(vfs).SectorBytes;
    get_dm(vfs).RootMaxFiles = get_dm(vfs).ClustnoBytes / 32;
  }
  get_dm(vfs).FatMaxTerms =
      (get_dm(vfs).Fat2Address - get_dm(vfs).Fat1Address) * 8 /
      get_dm(vfs).type;

  printk("dm(type:FAT%d) init done.\n", get_dm(vfs).type);
  printk("dm.ClustnoBytes:%d\n", get_dm(vfs).ClustnoBytes);
  printk("dm.RootMaxFiles:%d\n", get_dm(vfs).RootMaxFiles);
  printk("dm.RootDictAddress:%08x\n", get_dm(vfs).RootDictAddress);
  printk("dm.FileDataAddress:%08x\n", get_dm(vfs).FileDataAddress);
  printk("dm.imgTotalSize:%d\n", get_dm(vfs).imgTotalSize);
  printk("dm.SectorBytes:%d\n", get_dm(vfs).SectorBytes);
  printk("dm.Fat1Address:%08x\n", get_dm(vfs).Fat1Address);
  printk("dm.Fat2Address:%08x\n", get_dm(vfs).Fat2Address);
  printk("dm.FatMaxTerms:%d\n", get_dm(vfs).FatMaxTerms);

  uint32_t sec = (get_dm(vfs).RootDictAddress + get_dm(vfs).RootMaxFiles * 32) /
                 get_dm(vfs).SectorBytes;
  get_dm(vfs).ADR_DISKIMG = (unsigned int)malloc(get_dm(vfs).RootDictAddress +
                                                 get_dm(vfs).RootMaxFiles * 32);

  Disk_Read(0, sec, (void *)get_dm(vfs).ADR_DISKIMG, disk_number);

  get_dm(vfs).fat = malloc(get_dm(vfs).FatMaxTerms * sizeof(int));
  get_dm(vfs).FatClustnoFlags = malloc(get_dm(vfs).FatMaxTerms * sizeof(char));
  read_fat((unsigned char *)(get_dm(vfs).ADR_DISKIMG +
                             (unsigned int)get_dm(vfs).Fat1Address),
           get_dm(vfs).fat, get_dm(vfs).FatClustnoFlags,
           get_dm(vfs).FatMaxTerms, get_dm(vfs).type);
  get_dm(vfs).root_directory =
      (struct FAT_FILEINFO *)malloc(get_dm(vfs).RootMaxFiles * 32);
  memcpy((void *)get_dm(vfs).root_directory,
         (void *)get_dm(vfs).ADR_DISKIMG + get_dm(vfs).RootDictAddress,
         get_dm(vfs).RootMaxFiles * 32);
  get_dm(vfs).directory_list = (struct LIST *)NewList();
  get_dm(vfs).directory_clustno_list = (struct LIST *)NewList();
  struct FAT_FILEINFO *finfo = get_dm(vfs).root_directory;

  for (int i = 0; i != get_dm(vfs).RootMaxFiles; i++) {
    if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
      AddVal(finfo[i].clustno_low,
             (struct List *)get_dm(vfs).directory_clustno_list);
      void *directory_alloc = malloc(get_dm(vfs).ClustnoBytes);
      uint32_t sec1 = (get_dm(vfs).FileDataAddress +
                       (finfo[i].clustno_low - 2) * get_dm(vfs).ClustnoBytes) /
                      get_dm(vfs).SectorBytes;
      Disk_Read(sec1, get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
                directory_alloc, disk_number);
      AddVal((int)directory_alloc, (struct List *)get_dm(vfs).directory_list);
    }
    if (finfo[i].name[0] == 0) {
      break;
    }
  }

  for (int i = 1;
       FindForCount(i, (struct List *)get_dm(vfs).directory_list) != NULL;
       i++) {
    struct List *list =
        FindForCount(i, (struct List *)get_dm(vfs).directory_list);
    finfo = (struct FAT_FILEINFO *)list->val;
    for (int j = 0; j != get_dm(vfs).ClustnoBytes / 32; j++) {
      if (finfo[j].type == 0x10 && finfo[j].name[0] != 0xe5 &&
          strncmp(".", (char *)finfo[j].name, 1) != 0 &&
          strncmp("..", (char *)finfo[j].name, 2) != 0) {
        AddVal(finfo[j].clustno_low,
               (struct List *)get_dm(vfs).directory_clustno_list);
        void *directory_alloc = page_malloc(get_dm(vfs).ClustnoBytes);
        uint32_t sec1 =
            (get_dm(vfs).FileDataAddress +
             (finfo[j].clustno_low - 2) * get_dm(vfs).ClustnoBytes) /
            get_dm(vfs).SectorBytes;
        Disk_Read(sec1, get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
                  directory_alloc, disk_number);
        AddVal((int)directory_alloc, (struct List *)get_dm(vfs).directory_list);
      }
      if (finfo[j].name[0] == 0) {
        break;
      }
    }
  }
  page_free(boot_sector, 512);
}
void Fat_CopyCache(struct vfs_t *dest, struct vfs_t *src) {
  dest->cache = malloc(sizeof(fat_cache));
  memcpy(dest->cache, src->cache, sizeof(fat_cache));
}
bool Fat_cd(struct vfs_t *vfs, char *dictName) {
  return changedict(dictName, vfs);
}
bool Fat_ReadFile(struct vfs_t *vfs, char *path, char *buffer) {
  // printf("Fat12 Read ---- %s\n",path);
  struct FAT_FILEINFO *finfo;
  finfo = Get_File_Address(path, vfs);
  //  printk("finfo = %08x\n", finfo);
  if (finfo == 0) {
    return 0;
  } else {
    file_loadfile(get_clustno(finfo->clustno_high, finfo->clustno_low),
                  finfo->size, buffer, get_dm(vfs).fat, vfs);
    return 1;
  }
}
bool Fat_WriteFile(struct vfs_t *vfs, char *path, char *buffer, int size) {
  struct TASK *task = NowTask();
  struct FAT_FILEINFO *finfo = Get_File_Address(path, vfs);
  // printf("finfo = %08x\n",finfo);
  file_savefile(get_clustno(finfo->clustno_high, finfo->clustno_low), size,
                buffer, get_dm(vfs).fat, get_dm(vfs).FatClustnoFlags, vfs);
  // printf("file save ok\n");
  finfo->size = size;
  file_saveinfo(Get_dictaddr(path, vfs), vfs);
}
List *Fat_ListFile(struct vfs_t *vfs, char *dictpath) {
  struct FAT_FILEINFO *finfo = Get_dictaddr(dictpath, vfs);
  List *result = NewList();
  char s[30];
  for (int i = 0; i != 30; i++) {
    s[i] = 0;
  }
  for (int i = 0; i < get_dm(vfs).RootMaxFiles; i++) {
    if (finfo[i].name[0] == 0x00) {
      break;
    }
    if (finfo[i].name[0] != 0xe5) {
      if ((finfo[i].type & 0x18) == 0 || finfo[i].type == 0x10) {
        for (int j = 0; j < 8; j++) {
          s[j] = finfo[i].name[j];
        }
        s[8] = '.';
        if (finfo[i].ext[0] == ' ') {
          s[8] = 0;
        } else {
          s[9] = finfo[i].ext[0];
          s[10] = finfo[i].ext[1];
          s[11] = finfo[i].ext[2];
        }
        if (s[0] != '+') {
          vfs_file *d = malloc(sizeof(vfs_file));
          if (finfo[i].type == 0x10) {
            d->type = DIR;
          } else if (finfo[i].type == 0x20) {
            d->type = FLE;
          } else if (finfo[i].type == 0x01) {
            d->type = RDO;
          } else if (finfo[i].type == 0x02) {
            d->type = HID;
          } else if (finfo[i].type == 0x04) {
            d->type = SYS;
          }
          d->year = (finfo[i].update_date & 65024) >> 9;
          d->year += 1980;
          d->month = (finfo[i].update_date & 480) >> 5;
          d->day = finfo[i].update_date & 31;
          d->hour = (finfo[i].update_time & 63488) >> 11;
          d->minute = (finfo[i].update_time & 2016) >> 5;
          d->size = finfo[i].size;
          int q = 0;
          for (int k = 0; k < 12 && s[k] != 0; ++k) {
            if (s[k] != ' ') {
              d->name[q++] = s[k];
            }
          }
          d->name[q] = 0;
          // printk("d->name = %s\n", d->name);
          AddVal(d, result);
        }
      }
    }
  }
  return result;
}
bool Fat_RenameFile(struct vfs_t *vfs, char *filename, char *filename_of_new) {
  return rename(filename, filename_of_new, vfs);
}
bool Fat_CreateFile(struct vfs_t *vfs, char *filename) {
  mkfile(filename, vfs);
  return true;
}
void Fat_DeleteFs(struct vfs_t *vfs) {
  free((void *)get_dm(vfs).ADR_DISKIMG);
  free(get_dm(vfs).fat);
  free(get_dm(vfs).FatClustnoFlags);
  free(get_dm(vfs).root_directory);
  DeleteList(get_dm(vfs).directory_clustno_list);
  DeleteList(get_dm(vfs).directory_list);
}
bool Fat_Check(uint8_t disk_number) {
  uint8_t *boot_sec = malloc(512);
  Disk_Read(0, 1, boot_sec, disk_number);
  if (memcmp(boot_sec + BS_FileSysType, "FAT12   ", 8) == 0 ||
      memcmp(boot_sec + BS_FileSysType, "FAT16   ", 8) == 0 ||
      memcmp(boot_sec + BS_FileSysType + BPB_Fat32ExtByts, "FAT32   ", 8) ==
          0) {
    free(boot_sec);
    return true;
  }
  free(boot_sec);
  return false;
}
bool Fat_DelFile(struct vfs_t *vfs, char *path) {
  printk("Fat_DelFile:%s\n", path);
  return del(path, vfs);
}
bool Fat_DelDict(struct vfs_t *vfs, char *path) { return deldir(path, vfs); }
int Fat_FileSize(struct vfs_t *vfs, char *filename) {
  if (Get_File_Address(filename, vfs) == NULL) {
    return -1;
  }
  return Get_File_Address(filename, vfs)->size;
}
bool Fat_Format(uint8_t disk_number) { return format(disk_number); }
bool Fat_CreateDict(struct vfs_t *vfs, char *filename) {
  struct FAT_FILEINFO *finfo = Get_dictaddr(filename, vfs);
  int last_clustno = finfo[0].clustno_low;
  if (finfo == get_dm(vfs).root_directory) {
    last_clustno = 0;
  }
  mkdir(filename, last_clustno, vfs);
}
bool Fat_Attrib(struct vfs_t *vfs, char *filename, ftype type) {
  return attrib(filename, type, vfs);
}
vfs_file *Fat_FileInfo(struct vfs_t *vfs, char *filename) {
  struct FAT_FILEINFO *finfo = Get_File_Address(filename, vfs);
  if (finfo == NULL) {
    return (vfs_file *)NULL;
  }
  vfs_file *result = (vfs_file *)malloc(sizeof(vfs_file));
  char s[30];
  for (int i = 0; i != 30; i++) {
    s[i] = 0;
  }
  for (int i = 0; i < 8; i++) {
    s[i] = finfo->name[i];
  }
  s[8] = '.';
  if (finfo->ext[0] == ' ') {
    s[8] = 0;
  } else {
    s[9] = finfo->ext[0];
    s[10] = finfo->ext[1];
    s[11] = finfo->ext[2];
  }
  int i = 0;
  for (int j = 0; i < 12 && s[i] != 0; i++) {
    if (s[i] != ' ') {
      result->name[j++] = s[i];
    }
  }
  s[i] = '\0';
  if (finfo->type == 0x10) {
    result->type = DIR;
  } else if (finfo->type == 0x20) {
    result->type = FLE;
  } else if (finfo->type == 0x01) {
    result->type = RDO;
  } else if (finfo->type == 0x02) {
    result->type = HID;
  } else if (finfo->type == 0x04) {
    result->type = SYS;
  }
  result->year = (finfo->update_date & 65024) >> 9;
  result->year += 1980;
  result->month = (finfo->update_date & 480) >> 5;
  result->day = finfo->update_date & 31;
  result->hour = (finfo->update_time & 63488) >> 11;
  result->minute = (finfo->update_time & 2016) >> 5;
  result->size = finfo->size;
  return result;
}
void Register_fat_fileSys() {
  vfs_t fs;
  fs.flag = 1;
  fs.cache = NULL;
  strcpy(fs.FSName, "FAT");
  fs.CopyCache = Fat_CopyCache;
  fs.Format = Fat_Format;
  fs.CreateFile = Fat_CreateFile;
  fs.CreateDict = Fat_CreateDict;
  fs.DelDict = Fat_DelDict;
  fs.DelFile = Fat_DelFile;
  fs.ReadFile = Fat_ReadFile;
  fs.WriteFile = Fat_WriteFile;
  fs.DeleteFs = Fat_DeleteFs;
  fs.cd = Fat_cd;
  fs.FileSize = Fat_FileSize;
  fs.Check = Fat_Check;
  fs.ListFile = Fat_ListFile;
  fs.InitFs = fat_InitFS;
  fs.RenameFile = Fat_RenameFile;
  fs.Attrib = Fat_Attrib;
  fs.FileInfo = Fat_FileInfo;
  vfs_register_fs(fs);
}
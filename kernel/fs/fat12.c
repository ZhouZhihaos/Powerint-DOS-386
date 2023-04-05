// fs.c : 文件系统的实现
//目前实现了：fat12文件系统
#include <dos.h>
#include <fs.h>
typedef struct {
  struct DRIVE_MEMBER dm;
  struct FILEINFO* dir;
} fat12_cache;
#define get_dm(vfs) ((fat12_cache*)(vfs->cache))->dm
#define get_now_dir(vfs) ((fat12_cache*)(vfs->cache))->dir
static inline int get_fat12_date(unsigned short year,
                                 unsigned short month,
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
void read_fat(unsigned char* img, int* fat, unsigned char* ff) {
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
void write_fat(unsigned char* img, int* fat) {
  int i, j = 0;
  for (i = 0; i < 3072; i += 2) {
    img[j + 0] = fat[i + 0] & 0xff;
    img[j + 1] = (fat[i + 0] >> 8 | fat[i + 1] << 4) & 0xff;
    img[j + 2] = (fat[i + 1] >> 4) & 0xff;
    j += 3;
  }
  return;
}
void file_loadfile(int clustno, int size, char* buf, int* fat, vfs_t* vfs) {
  if (!size) {
    return;
  }
  void* img = page_malloc(((size - 1) / get_dm(vfs).ClustnoBytes + 1) *
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
  memcpy((void*)buf, img, size);
  page_free(img, ((size - 1) / get_dm(vfs).SectorBytes + 1) *
                     get_dm(vfs).SectorBytes);
 // printk("done 2\n");
  return;
}
FILE* f_t;
void file_savefile(int clustno,
                   int size,
                   char* buf,
                   int* fat,
                   unsigned char* ff,
                   vfs_t* vfs) {
  uint32_t clustall = 0;
  int tmp = clustno;
  while (fat[clustno] !=
         0xfff) {  // 计算文件占多少Fat项 Fat项 = 大小 / 簇大小 + 1
    clustno = fat[clustno];
    clustall++;
  }
  int old_clustno = clustno;
  clustno = tmp;
  int alloc_size;
  if (size >
      (clustall + 1) *
          get_dm(vfs)
              .ClustnoBytes) {  // 新大小 > (旧大小 / 簇大小 + 1) * 簇大小
    // 请求内存大小 = (新大小 / 簇大小 + 1) * 簇大小
    alloc_size =
        ((size - 1) / get_dm(vfs).ClustnoBytes + 1) * get_dm(vfs).ClustnoBytes;
    // 分配Fat（这里需要在写盘前分配）
    for (int size1 = size; size1 > ((clustall + 1) * get_dm(vfs).ClustnoBytes);
         size1 -= get_dm(vfs).ClustnoBytes) {
      for (int i = 0; i != 3072; i++) {
        if (!ff[i]) {
          fat[old_clustno] = i;
          old_clustno = i;
          ff[i] = true;
          break;
        }
      }
    }
    fat[old_clustno] = 0xfff;  // 结尾Fat = 0xfff
    ff[old_clustno] = true;
  } else if (size <=
             (clustall + 1) *
                 get_dm(vfs).ClustnoBytes) {  // 新大小 <= (旧大小 / 簇大小
                                              // + 1) * 簇大小
    // 请求内存大小 = (旧大小 / 簇大小 + 1) * 簇大小
    alloc_size = (clustall + 1) * get_dm(vfs).ClustnoBytes;
    // 这里不分配Fat的原因是要清空更改后多余的数据
  }
  void* img = page_malloc(alloc_size);
  clean((char*)img, alloc_size);
  memcpy(img, (void*)buf, size);  // 把要写入的数据复制到新请求的内存地址
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
          get_dm(vfs).ClustnoBytes) {  // 新大小 < (旧大小 / 簇大小) * 簇大小
    // 分配Fat（中间情况没必要分配）
    int i;
    for (int size1 = clustall * get_dm(vfs).ClustnoBytes, i = old_clustno;
         size1 > size; size1 -= get_dm(vfs).ClustnoBytes) {
      fat[i] = 0;
      ff[i] = false;
      for (int j = 0; j != 3072; j++)
        if (fat[j] == i)
          i = j;
    }
    old_clustno = i;
    fat[old_clustno] = 0xfff;  // 结尾Fat = 0xfff
    ff[old_clustno] = true;
  }
  file_savefat(fat, vfs->disk_number);
}
void file_saveinfo(struct FILEINFO* directory, vfs_t* vfs) {
  if (directory == get_dm(vfs).root_directory) {
    Disk_Write(get_dm(vfs).RootDictAddress / get_dm(vfs).SectorBytes,
               14 * get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
               (void*)directory, vfs->disk_number);
  } else {
    for (int i = 1; FindForCount(i, get_dm(vfs).directory_list) != NULL; i++) {
      struct List* list = FindForCount(i, get_dm(vfs).directory_list);
      if (list->val == directory) {
        list = FindForCount(i, get_dm(vfs).directory_clustno_list);
        Disk_Write((get_dm(vfs).FileDataAddress +
                    (list->val - 2) * get_dm(vfs).ClustnoBytes) /
                       get_dm(vfs).SectorBytes,
                   get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
                   (void*)directory, vfs->disk_number);
        break;
      }
    }
  }
}
void file_savefat(int* fat, vfs_t* vfs) {
  write_fat(get_dm(vfs).ADR_DISKIMG + get_dm(vfs).Fat1Address, fat);
  write_fat(get_dm(vfs).ADR_DISKIMG + get_dm(vfs).Fat2Address, fat);
  Disk_Write(1, 9, get_dm(vfs).ADR_DISKIMG + get_dm(vfs).Fat2Address,
             vfs->disk_number);
  Disk_Write(10, 9, get_dm(vfs).ADR_DISKIMG + get_dm(vfs).Fat2Address,
             vfs->disk_number);
}
struct FILEINFO* file_search(char* name, struct FILEINFO* finfo, int max) {
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
struct FILEINFO* dict_search(char* name, struct FILEINFO* finfo, int max) {
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
struct FILEINFO* Get_File_Address(char* path1, vfs_t* vfs) {
  // TODO: Modifly it
  struct FILEINFO* bmpDict = get_now_dir(vfs);
  int drive_number = vfs->disk_number;
 // printk("Get_File_Address :%s\n", path1);
  char* path = (char*)page_malloc(strlen(path1) + 1);
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
  char* temp_name = (char*)page_malloc(128);
  struct FILEINFO* finfo;
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
      if (finfo->clustno != 0) {
        for (int count = 1;
             FindForCount(count, get_dm(vfs).directory_clustno_list) != NULL;
             count++) {
          struct List* list =
              FindForCount(count, get_dm(vfs).directory_clustno_list);
          if (list->val == finfo->clustno) {
            list = FindForCount(count, get_dm(vfs).directory_list);
            bmpDict = (struct FILEINFO*)list->val;
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
struct FILEINFO* Get_dictaddr(char* path1, vfs_t* vfs) {
  // TODO: Modifly it
  struct FILEINFO* bmpDict = get_now_dir(vfs);
  int drive_number = vfs->disk_number;
  char* path = (char*)page_malloc(strlen(path1) + 1);
  strcpy(path, path1);
  strtoupper(path);
  if (strncmp("/", path, 11) == 0) {
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
  char* temp_name = (char*)page_malloc(128);
  struct FILEINFO* finfo;
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
      if (finfo->clustno != 0) {
        for (int count = 1;
             FindForCount(count, get_dm(vfs).directory_clustno_list) != NULL;
             count++) {
          struct List* list =
              FindForCount(count, get_dm(vfs).directory_clustno_list);
          if (list->val == finfo->clustno) {
            list = FindForCount(count, get_dm(vfs).directory_list);
            // printf("change %08x to", bmpDict);
            bmpDict = (struct FILEINFO*)list->val;
            // printf("%08x\n", bmpDict);

            break;
          } else {
            // printf("list->val = %08x finfo->clustno = %08x %08x\n",
            // list->val,
            //   finfo->clustno,
            //   get_dm(vfs).directory_clustno_list);
          }
        }
        // printf(
        //     "Not Found %08x,and "
        //     "FindForCount(count,get_dm(vfs).directory_"
        //     "clustno_list)=%08x %08x\n",
        //     finfo->clustno,
        //     FindForCount(
        //         1,
        //         get_dm(vfs).directory_clustno_list),get_dm(vfs).directory_clustno_list);
      } else {
        // printf("finfo->clustno == 0\n");
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
struct List* Get_wildcard_File_Address(char* path) {
  // struct TASK* task = NowTask();
  // struct FILEINFO* finfo = Get_dictaddr(path);
  // struct List* list = NewList();
  // strtoupper(path);
  // int tmp = 0;
  // for (int i = 0; i != strlen(path); i++) {
  //   if (path[i] == '\\' || path[i] == '/') {
  //     tmp = i + 1;
  //   }
  // }
  // path += tmp;
  // char s[12];
  // for (int i = 0; i != 11; i++) {
  //   s[i] = ' ';
  // }
  // for (int i = 0, j = 0; i != 11; i++, j++) {
  //   if (path[j] == '.') {
  //     i = 7;
  //   } else {
  //     s[i] = path[j];
  //   }
  // }
  // bool name = false, ext = false;
  // char* p = path;
  // if (p[0] == '*') {
  //   name = true;
  // }
  // for (; *p != '.'; p++)
  //   ;
  // p++;
  // if (p[0] == '*') {
  //   ext = true;
  // }
  // if (!name && !ext) {
  //   bool back = true;
  //   for (int i = 0; i != 12; i++) {
  //     if (s[i] == '?') {
  //       back = false;
  //       break;
  //     }
  //   }
  //   if (back) {
  //     return (struct List*)NULL;
  //   }
  // }
  // for (int i = 0; i != get_dm(vfs).RootMaxFiles; i++) {
  //   bool add = true;
  //   if (!name) {
  //     for (int j = 0; j != 8; j++) {
  //       if (finfo[i].name[j] != s[j] && s[j] != '?') {
  //         add = false;
  //         break;
  //       }
  //     }
  //   }
  //   if (!ext) {
  //     for (int j = 8; j != 11; j++) {
  //       if (finfo[i].ext[j - 8] != s[j] && s[j] != '?') {
  //         add = false;
  //         break;
  //       }
  //     }
  //   }
  //   if (add) {
  //     AddVal((int)(finfo + i), list);
  //   }
  //   if (finfo[i].name[0] == '\0') {
  //     return list;
  //   }
  // }
}
void mkdir(char* dictname, int last_clust, vfs_t* vfs) {
  /*
          dictname:目录名
          last_clust:上一级目录的簇号
  */
  mkfile(dictname, vfs);
  struct FILEINFO* finfo = Get_File_Address(dictname, vfs);
  FILE* fp = fopen(dictname, "wb");
  // 三个目录项（模板）
  struct FILEINFO dictmodel1;  // .目录项，指向自己
  struct FILEINFO dictmodel2;  // ..目录项，指向上一级目录
  struct FILEINFO null;        //空目录项（为mkfile函数提供指引）
  memcpy(null.name, "NULL       ", 11);
  null.type = 0x20;  // 文件的type属性是0x20
  //将size date time这类属性全部设置为0
  null.size = 0;
  null.date = get_fat12_date(get_year(), get_mon_hex(), get_day_of_month());
  null.time = get_fat12_time(get_hour_hex(), get_min_hex());
  for (int i = 0; i != 3072; i++) {
    if (!get_dm(vfs).fat[i]) {
      null.clustno = i;
      get_dm(vfs).fat[i] = 0xfff;
      get_dm(vfs).FatClustnoFlags[i] = true;
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
  dictmodel1.clustno = finfo->clustno;  // 指向自己
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
  unsigned char* ptr = (unsigned char*)&dictmodel1;
  fseek(fp, 0, 0);
  for (int i = 0; i != 32; i++) {
    fputc(ptr[i], fp);
  }
  ptr = (unsigned char*)&dictmodel2;
  fseek(fp, 32, 0);
  for (int i = 0; i != 32; i++) {
    fputc(ptr[i], fp);
  }
  ptr = (unsigned char*)&null;
  fseek(fp, 64, 0);
  for (int i = 0; i != 32; i++) {
    fputc(ptr[i], fp);
  }
  fclose(fp);
  finfo->type = 0x10;  // 是目录（文件夹的type属性是0x10）
  finfo->size = 0;
  int drive_number = vfs->disk_number;
  AddVal(finfo->clustno, get_dm(vfs).directory_clustno_list);
  // printf("Add = %08x %02x %08x\n", finfo->clustno, vfs->disk_number,
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
    file_saveinfo(get_dm(vfs).root_directory, vfs->disk_number);
  } else {
    for (int i = 1; FindForCount(i, get_dm(vfs).directory_clustno_list) != NULL;
         i++) {
      struct List* list = FindForCount(i, get_dm(vfs).directory_clustno_list);
      if (list->val == last_clust) {
        list = FindForCount(i, get_dm(vfs).directory_list);
        struct FILEINFO* d_finfo = (struct FILEINFO*)list->val;
        file_saveinfo(d_finfo, vfs->disk_number);
      }
    }
  }
  file_savefat(get_dm(vfs).fat, vfs->disk_number);

  void* directory_alloc = page_malloc(get_dm(vfs).ClustnoBytes);
  Disk_Read((get_dm(vfs).FileDataAddress +
             (finfo->clustno - 2) * get_dm(vfs).ClustnoBytes) /
                get_dm(vfs).SectorBytes,
            get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes, directory_alloc,
            vfs->disk_number);
  AddVal(directory_alloc, get_dm(vfs).directory_list);
  return;
}

struct FILEINFO* clust_sech(int clustno, struct FILEINFO* finfo, int max) {
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
int del(char* cmdline, vfs_t* vfs) {
  //删除某个文件
  char* name = cmdline;
  int i;
  struct FILEINFO* finfo;
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
  char *buf = malloc(finfo->size);
  memset(buf,0,finfo->size);
  Fat12_WriteFile(vfs,name,buf,finfo->size);
  finfo->name[0] = 0xe5;
  get_dm(vfs).fat[finfo->clustno] = 0;
  get_dm(vfs).FatClustnoFlags[finfo->clustno] = false;
  file_saveinfo(Get_dictaddr(name, vfs), vfs);
  file_savefat(get_dm(vfs).fat, vfs);
  return 1;
}
int deldir(char* path, vfs_t* vfs) {
  struct FILEINFO* finfo = Get_dictaddr(path, vfs);
  if (finfo == Get_dictaddr(".", vfs)) {
    return 0;
  }
  struct FILEINFO* f = get_now_dir(vfs);
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
  struct FILEINFO* root_finfo;
  if (finfo[1].clustno == 0) {
    root_finfo = get_dm(vfs).root_directory;
  } else {
    for (int i = 1; FindForCount(i, get_dm(vfs).directory_clustno_list) != NULL;
         i++) {
      if (FindForCount(i, get_dm(vfs).directory_clustno_list)->val ==
          finfo[1].clustno) {
        root_finfo =
            (struct FILEINFO*)FindForCount(i, get_dm(vfs).directory_list)->val;
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
  get_dm(vfs).fat[finfo->clustno] = 0;
  get_dm(vfs).FatClustnoFlags[finfo->clustno] = false;
  file_saveinfo(Get_dictaddr(path, vfs), vfs);
  file_savefat(get_dm(vfs).fat, vfs);
  return 1;
}
void mkfile(char* name, vfs_t* vfs) {
  char s[12];
  int i, j;
  struct FILEINFO* finfo = Get_dictaddr(name, vfs);

  char* path = name;
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
    if (!get_dm(vfs).fat[i]) {
      finfo->clustno = i;
      get_dm(vfs).fat[i] = 0xfff;
      get_dm(vfs).FatClustnoFlags[i] = true;
      break;
    }
  }
  for (i = 0; i != 10; i++) {
    finfo->reserve[i] = 0;
  }
  finfo->time = get_fat12_time(get_hour_hex(), get_min_hex());
  finfo->date = get_fat12_date(get_year(), get_mon_hex(), get_day_of_month());
  finfo->size = 0;
  file_saveinfo(Get_dictaddr(path, vfs), vfs);
  file_savefat(get_dm(vfs).fat, vfs);
  return;
}
int changedict(char* dictname, vfs_t* vfs) {
  // cd命令的依赖函数
  strtoupper(dictname);
  struct TASK* task = NowTask();

  if (strcmp(dictname, "/") == 0) {
    while (vfs->path->ctl->all != 0) {
      page_kfree(FindForCount(vfs->path->ctl->all, vfs->path)->val, 255);
      DeleteVal(vfs->path->ctl->all, vfs->path);
    }
    get_now_dir(vfs) = get_dm(vfs).root_directory;
    return 1;
  }
  struct FILEINFO* finfo =
      dict_search(dictname, get_now_dir(vfs), get_dm(vfs).RootMaxFiles);
  //找文件夹
  if (finfo == 0) {
    //没找到
    return 0;
  }
  if (finfo->clustno == 0) {
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
    char* dict = page_kmalloc(255);
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
    struct List* list = FindForCount(count, get_dm(vfs).directory_clustno_list);
    if (list->val == finfo->clustno) {
      list = FindForCount(count, get_dm(vfs).directory_list);
      get_now_dir(vfs) = (struct FILEINFO*)list->val;
      break;
    }
  }
  //  task->change_dict_times++;
  return 1;
}
int rename(char* src_name, char* dst_name, vfs_t* vfs) {
  strtoupper(src_name);
  strtoupper(dst_name);
  struct TASK* task = NowTask();
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
  struct FILEINFO* finfo = Get_File_Address(src_name, vfs);
  if (finfo == 0 || finfo->type == 0x01 || finfo->type == 0x04) {
    return 0;
  }
  memset((void*)finfo->name, ' ', 11);
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
  FILE* fp = fopen("/boot.bin", "r");  // or \boot.bin
  if (fp == 0) {
    return 0;
  }
  void* read_in = page_malloc(fp->fileSize);
  fread(read_in, fp->fileSize, 1, fp);
  if (!(drive - 'A')) {
    // printf("3K FloppyDisk: %d bytes\n", 2880 * 512);
    // printf("INT 13H DriveNumber: 0\n");
    // printf("RootDictFiles: 224\n");
    // printf("drive_ctl.drives[%d].ClustnoBytes: 512 "
    //        "bytes\n",
    //        NowTask()->vfs->disk_number);
    *(unsigned char*)(&((unsigned char*)read_in)[BPB_SecPerClus]) = 1;
    *(unsigned short*)(&((unsigned char*)read_in)[BPB_RootEntCnt]) = 224;
    *(unsigned short*)(&((unsigned char*)read_in)[BPB_TotSec16]) = 2880;
    *(unsigned int*)(&((unsigned char*)read_in)[BPB_TotSec32]) = 2880;
    *(unsigned char*)(&((unsigned char*)read_in)[BS_DrvNum]) = 0;
    write_floppy_for_ths(0, 0, 1, read_in, 1);
    unsigned int* fat = (unsigned int*)page_malloc(9 * 512);
    fat[0] = 0x00fffff0;
    write_floppy_for_ths(0, 0, 2, (unsigned char*)fat, 9);
    write_floppy_for_ths(0, 0, 11, (unsigned char*)fat, 9);
    page_free((void*)fat, 9 * 512);
    void* null_sec = page_malloc(512);
    for (int i = 0; i < 224 * 32 / 512; i++) {
      write_floppy_for_ths(0, 0, 20 + i, null_sec, 1);
    }
    page_free(null_sec, 512);
  } else if (drive != 'B') {
    // struct IDEHardDiskInfomationBlock* info = drivers_idehdd_info();
    // printk("drive=%c %d\n", drive, have_vdisk(drive));
    if (DiskReady(drive)) {
      // printf("Couldn't find Disk.\n");
      return 0;
    }

    // printf("Disk: %d bytes\n", disk_Size(drive));
    // printf("RootDictFiles: %d\n",
    //        14 * (((disk_Size(drive) / 4096) / 512 + 1) * 512) / 32);
    // printf("ClustnoBytes: %d bytes\n",
    //        ((disk_Size(drive) / 4096) / 512 + 1) * 512);
    *(unsigned char*)(&((unsigned char*)read_in)[BPB_SecPerClus]) =
        ((disk_Size(drive) / 4096) / 512 + 1);
    *(unsigned short*)(&((unsigned char*)read_in)[BPB_RootEntCnt]) =
        14 * (((disk_Size(drive) / 4096) / 512 + 1) * 512) / 32;
    // printk("Sectors:%d\n", ide_devices[drive - 'C'].Size /power
    if (disk_Size(drive) / 512 > 65535) {
      *(unsigned short*)(&((unsigned char*)read_in)[BPB_TotSec16]) = 0;
    } else {
      *(unsigned short*)(&((unsigned char*)read_in)[BPB_TotSec16]) =
          disk_Size(drive) / 512;
    }
    *(unsigned int*)(&((unsigned char*)read_in)[BPB_TotSec32]) =
        disk_Size(drive) / 512;
    *(unsigned char*)(&((unsigned char*)read_in)[BS_DrvNum]) =
        drive - 'C' + 0x80;
    Disk_Write(0, 1, (unsigned short*)read_in, drive);
    unsigned int* fat = (unsigned int*)page_malloc(9 * 512);
    fat[0] = 0x00fffff0;
    Disk_Write(1, 9, (unsigned short*)fat, drive);
    Disk_Write(10, 9, (unsigned short*)fat, drive);
    page_free((void*)fat, 9 * 512);
    void* null_sec = page_malloc(512);
    clean((char*)null_sec, 512);
    for (int i = 0;
         i < 14 * (((disk_Size(drive) / 4096) / 512 + 1) * 512) / 32 * 32 / 512;
         i++) {
      Disk_Write(19, 1, (unsigned short*)null_sec, drive);
    }
    page_free(null_sec, 512);
    // page_free((void*)info, 256 * sizeof(short));
  }
  page_free(read_in, fp->fileSize);
  fclose(fp);
  return 1;
}
int attrib(char* filename, char type, vfs_t* vfs) {
  struct FILEINFO* finfo = Get_File_Address(filename, vfs);
  int drive_number = vfs->disk_number;
  if (finfo == 0) {
    return -1;
  }
  finfo->type = type;
  file_saveinfo(Get_dictaddr(filename, vfs), vfs);
}
void fat12_InitFS(struct vfs_t* vfs, uint8_t disk_number) {
  vfs->cache = malloc(sizeof(fat12_cache));
  if (1) {
    void* boot_sector = page_malloc(512);
    Disk_Read(0, 1, boot_sector, disk_number);

    if (!*(unsigned char*)(boot_sector)) {
      return;
    }
    get_dm(vfs).SectorBytes = *(unsigned short*)(boot_sector + BPB_BytsPerSec);
    get_dm(vfs).RootMaxFiles = *(unsigned short*)(boot_sector + BPB_RootEntCnt);
    get_dm(vfs).ClustnoBytes = get_dm(vfs).SectorBytes *
                               *(unsigned char*)(boot_sector + BPB_SecPerClus);
    get_dm(vfs).RootDictAddress =
        (*(unsigned char*)(boot_sector + BPB_NumFATs) *
             *(unsigned short*)(boot_sector + BPB_FATSz16) +
         *(unsigned short*)(boot_sector + BPB_RsvdSecCnt)) *
        get_dm(vfs).SectorBytes;
    get_dm(vfs).FileDataAddress =
        get_dm(vfs).RootDictAddress + get_dm(vfs).RootMaxFiles * 32;
    if (*(unsigned short*)(boot_sector + BPB_TotSec16) != 0) {
      get_dm(vfs).imgTotalSize =
          *(unsigned short*)(boot_sector + BPB_TotSec16) *
          get_dm(vfs).SectorBytes;
    } else {
      get_dm(vfs).imgTotalSize = *(unsigned int*)(boot_sector + BPB_TotSec32) *
                                 get_dm(vfs).SectorBytes;
    }
    get_dm(vfs).Fat1Address = *(unsigned short*)(boot_sector + BPB_RsvdSecCnt) *
                              get_dm(vfs).SectorBytes;
    get_dm(vfs).Fat2Address =
        get_dm(vfs).Fat1Address +
        *(unsigned short*)(boot_sector + BPB_FATSz16) * get_dm(vfs).SectorBytes;
    uint32_t sec = get_dm(vfs).FileDataAddress / get_dm(vfs).SectorBytes;
    get_dm(vfs).ADR_DISKIMG = (unsigned int)malloc(get_dm(vfs).FileDataAddress);

    Disk_Read(0, sec, (void*)get_dm(vfs).ADR_DISKIMG, disk_number);

    get_dm(vfs).fat = malloc(3072 * sizeof(int));
    get_dm(vfs).FatClustnoFlags = malloc(3072 * sizeof(char));
    read_fat((unsigned char*)(get_dm(vfs).ADR_DISKIMG +
                              (unsigned int)get_dm(vfs).Fat1Address),
             get_dm(vfs).fat, get_dm(vfs).FatClustnoFlags);
    get_dm(vfs).root_directory =
        (struct FILEINFO*)malloc(get_dm(vfs).RootMaxFiles * 32);
    memcpy((void*)get_dm(vfs).root_directory,
           (void*)get_dm(vfs).ADR_DISKIMG + get_dm(vfs).RootDictAddress,
           get_dm(vfs).RootMaxFiles * 32);
    get_dm(vfs).directory_list = (struct LIST*)NewList();
    get_dm(vfs).directory_clustno_list = (struct LIST*)NewList();
    struct FILEINFO* finfo = get_dm(vfs).root_directory;

    for (int i = 0; i != get_dm(vfs).RootMaxFiles; i++) {
      if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
        AddVal(finfo[i].clustno,
               (struct List*)get_dm(vfs).directory_clustno_list);
        void* directory_alloc = malloc(get_dm(vfs).ClustnoBytes);
        uint32_t sec1 = (get_dm(vfs).FileDataAddress +
                         (finfo[i].clustno - 2) * get_dm(vfs).ClustnoBytes) /
                        get_dm(vfs).SectorBytes;
        Disk_Read(sec1, get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
                  directory_alloc, disk_number);
        AddVal((int)directory_alloc, (struct List*)get_dm(vfs).directory_list);
      }
      if (finfo[i].name[0] == 0) {
        break;
      }
    }

    for (int i = 1;
         FindForCount(i, (struct List*)get_dm(vfs).directory_list) != NULL;
         i++) {
      struct List* list =
          FindForCount(i, (struct List*)get_dm(vfs).directory_list);
      finfo = (struct FILEINFO*)list->val;
      for (int j = 0; j != get_dm(vfs).ClustnoBytes / 32; j++) {
        if (finfo[j].type == 0x10 && finfo[j].name[0] != 0xe5 &&
            strncmp(".", (char*)finfo[j].name, 1) != 0 &&
            strncmp("..", (char*)finfo[j].name, 2) != 0) {
          AddVal(finfo[j].clustno,
                 (struct List*)get_dm(vfs).directory_clustno_list);
          void* directory_alloc = page_malloc(get_dm(vfs).ClustnoBytes);
          uint32_t sec1 = (get_dm(vfs).FileDataAddress +
                           (finfo[j].clustno - 2) * get_dm(vfs).ClustnoBytes) /
                          get_dm(vfs).SectorBytes;
          Disk_Read(sec1, get_dm(vfs).ClustnoBytes / get_dm(vfs).SectorBytes,
                    directory_alloc, disk_number);
          AddVal((int)directory_alloc,
                 (struct List*)get_dm(vfs).directory_list);
        }
        if (finfo[j].name[0] == 0) {
          break;
        }
      }
    }
    page_free(boot_sector, 512);
  }
}
void Fat12_CopyCache(struct vfs_t* dest, struct vfs_t* src) {
  dest->cache = malloc(sizeof(fat12_cache));
  memcpy(dest->cache, src->cache, sizeof(fat12_cache));
}
bool Fat12_cd(struct vfs_t* vfs, char* dictName) {
  return changedict(dictName, vfs);
}
bool Fat12_ReadFile(struct vfs_t* vfs, char* path, char* buffer) {
  // printf("Fat12 Read ---- %s\n",path);
  struct FILEINFO* finfo;
  finfo = Get_File_Address(path, vfs);
//  printk("finfo = %08x\n", finfo);
  if (finfo == 0) {
    return 0;
  } else {
    int drive_number = vfs->disk_number;
    file_loadfile(finfo->clustno, finfo->size, buffer, get_dm(vfs).fat, vfs);
    return 1;
  }
}
bool Fat12_WriteFile(struct vfs_t* vfs, char* path, char* buffer, int size) {
  struct TASK* task = NowTask();
  struct FILEINFO* finfo = Get_File_Address(path, vfs);
  // printf("finfo = %08x\n",finfo);
  file_savefile(finfo->clustno, size, buffer, get_dm(vfs).fat,
                get_dm(vfs).FatClustnoFlags, vfs);
  // printf("file save ok\n");
  finfo->size = size;
  file_saveinfo(Get_dictaddr(path, vfs), vfs);
}
List* Fat12_ListFile(struct vfs_t* vfs, char* dictpath) {
  struct FILEINFO* finfo = get_now_dir(vfs);
  List* result = NewList();
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
        if (finfo[i].type == 0x02) {
          continue;
        }
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
          vfs_dict* d = malloc(sizeof(vfs_dict));
          if (finfo[i].type == 0x10) {
            d->type = DIR;
          } else {
            d->type = FLE;
          }
          int i = 0;
          for (int k = 0; k < 12 && s[k] != 0; ++k) {
            if (s[k] != ' ') {
              d->name[i++] = s[k];
            }
          }
          d->name[i] = 0;
         // printk("d->name = %s\n", d->name);
          AddVal(d, result);
        }
      }
    }
  }
  return result;
}
bool Fat12_RenameFile(struct vfs_t* vfs,
                      char* filename,
                      char* filename_of_new) {
  return rename(filename, filename_of_new, vfs);
}
bool Fat12_CreateFile(struct vfs_t* vfs, char* filename) {
  mkfile(filename, vfs);
  return true;
}
void Fat12_DeleteFs(struct vfs_t* vfs) {
  free((void*)get_dm(vfs).ADR_DISKIMG);
  free(get_dm(vfs).fat);
  free(get_dm(vfs).FatClustnoFlags);
  free(get_dm(vfs).root_directory);
  DeleteList(get_dm(vfs).directory_clustno_list);
  DeleteList(get_dm(vfs).directory_list);
}
bool Fat12_Check(uint8_t disk_number) {
  uint8_t* boot_sec = malloc(512);
  Disk_Read(0, 1, boot_sec, disk_number);
  if (memcmp(boot_sec + BS_FileSysType, "FAT12   ", 8) == 0) {
    free(boot_sec);
    return true;
  }
  free(boot_sec);
  return false;
}
bool Fat12_DelFile(struct vfs_t* vfs, char* path) {
  printk("Fat12_DelFile:%s\n", path);
  return del(path, vfs);
}
bool Fat12_DelDict(struct vfs_t* vfs, char* path) {
  return deldir(path, vfs);
}
int Fat12_FileSize(struct vfs_t* vfs, char* filename) {
  if (Get_File_Address(filename, vfs) == NULL) {
    return -1;
  }
  return Get_File_Address(filename, vfs)->size;
}
bool Fat12_Format(uint8_t disk_number) {
  return format(disk_number);
}
bool Fat12_CreateDict(struct vfs_t* vfs, char* filename) {
  struct FILEINFO* finfo = Get_dictaddr(filename, vfs);
  int last_clustno = finfo[0].clustno;
  if (finfo == get_dm(vfs).root_directory) {
    last_clustno = 0;
  }
  mkdir(filename, last_clustno, vfs);
}
void Register_fat12_fileSys() {
  vfs_t fs;
  fs.flag = 1;
  fs.cache = NULL;
  strcpy(fs.FSName, "FAT12");
  fs.CopyCache = Fat12_CopyCache;
  fs.Format = Fat12_Format;
  fs.CreateFile = Fat12_CreateFile;
  fs.CreateDict = Fat12_CreateDict;
  fs.DelDict = Fat12_DelDict;
  fs.DelFile = Fat12_DelFile;
  fs.ReadFile = Fat12_ReadFile;
  fs.WriteFile = Fat12_WriteFile;
  fs.DeleteFs = Fat12_DeleteFs;
  fs.cd = Fat12_cd;
  fs.FileSize = Fat12_FileSize;
  fs.Check = Fat12_Check;
  fs.ListFile = Fat12_ListFile;
  fs.InitFs = fat12_InitFS;
  fs.RenameFile = Fat12_RenameFile;
  vfs_register_fs(fs);
}
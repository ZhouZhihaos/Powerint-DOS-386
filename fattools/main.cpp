// fattools
// Copyright (C) min0911_ & zhouzhihao 2022
#include <fstream>
#include <iostream>
#include <string.h>
#define BS_jmpBoot 0
#define BS_OEMName 3
#define BPB_BytsPerSec 11
#define BPB_SecPerClus 13
#define BPB_RsvdSecCnt 14
#define BPB_NumFATs 16
#define BPB_RootEntCnt 17
#define BPB_TotSec16 19
#define BPB_Media 21
#define BPB_FATSz16 22
#define BPB_SecPerTrk 24
#define BPB_NumHeads 26
#define BPB_HiddSec 28
#define BPB_TotSec32 32
#define BPB_FATSz32 36
#define BPB_ExtFlags 40
#define BPB_FSVer 42
#define BPB_RootClus 44
#define BPB_FSInfo 48
#define BPB_BkBootSec 50
#define BPB_Reserved 52
#define BPB_Fat32ExtByts 28
#define BS_DrvNum 36
#define BS_Reserved1 37
#define BS_BootSig 38
#define BS_VolD 39
#define BS_VolLab 43
#define BS_FileSysType 54
#define print printf
#define get_clustno(high, low) (high << 16) | (low & 0xffff)
#define clustno_end(type) 0xfffffff & ((((1 << (type - 1)) - 1) << 1) + 1)
struct FAT_FILEINFO {
  unsigned char name[8], ext[3], type;
  char reserve;
  unsigned char create_time_tenth;
  unsigned short create_time, create_date, access_date, clustno_high;
  unsigned short update_time, update_date, clustno_low;
  unsigned int size;
};
typedef enum { FLE, DIR, RDO, HID, SYS } ftype;
typedef struct MYFILE {
  unsigned int mode;
  unsigned int fileSize;
  unsigned char *buffer;
  unsigned int bufferSize;
  unsigned int p;
  char *name;
} MYFILE;
struct ListCtl {
  struct List *start;
  struct List *end;
  int all;
};
struct List {
  struct ListCtl *ctl;
  struct List *prev;
  uintptr_t val;
  struct List *next;
};
typedef struct {
  char name[255];
  ftype type;
  unsigned int size;
  unsigned short year, month, day;
  unsigned short hour, minute;
} vfs_file;
typedef struct List List;
using namespace std;
static struct FAT_FILEINFO *root_directory;
static struct List *directory_list;
static struct List *directory_clustno_list;
static int *fat;
static int FatMaxTerms;
static unsigned int ClustnoBytes;
static unsigned short RootMaxFiles;
static unsigned int RootDictAddress;
static unsigned int FileDataAddress;
static unsigned int imgTotalSize;
static unsigned short SectorBytes;
static unsigned int Fat1Address, Fat2Address;
static unsigned char *FatClustnoFlags;
struct FAT_FILEINFO *dir;
static int type;
static string ___file;
#define ADR_DISKIMG &___file[0]
string path;
void AddVal(uintptr_t val, struct List *Obj);
struct List *FindForCount(int count, struct List *Obj);
void DeleteVal(int count, struct List *Obj);
void Change(int count, struct List *Obj, uintptr_t val);
int GetLastCount(struct List *Obj);
void DeleteList(struct List *Obj);
void clean(char *s, int n);
void strtoupper(char *str);
int Copy_in(char *path, char *path1);
void Copy_out(char *path, char *path1);
void Disk_Read(unsigned int lba, unsigned int number, void *buffer);
void Disk_Write(unsigned int lba, unsigned int number, void *buffer);
void read_fat(unsigned char *img, int *fat, unsigned char *ff, int max,
              int type);
void file_loadfile(int clustno, int size, char *buf, int *fat);
void file_savefile(int clustno, int size, char *buf, int *fat,
                   unsigned char *ff);
void file_saveinfo(struct FAT_FILEINFO *directory);
void file_savefat(int *fat, int clustno, int length);
struct FAT_FILEINFO *Get_File_Address(char *path1);
void mkdir(char *dictname, int last_clust);
int del(char *cmdline);
int deldir(char *path);
void mkfile(char *name);
int changedict(char *dictname);
int rename(char *src_name, char *dst_name);
int attrib(char *filename, ftype type);
void fat_InitFS();
bool Fat_WriteFile(char *path, char *buffer, int size);
bool Fat_ReadFile(char *path, char *buffer);
bool Fat_CreateFile(char *filename);
int Fat_FileSize(char *filename);
void command_run(string commands);
void *_malloc(size_t size) {
  void *result = malloc(size);
  clean((char *)result, size);
  return result;
}
void AddVal(uintptr_t val, struct List *Obj) {
  while (Obj->next != NULL)
    Obj = Obj->next;
  Obj = Obj->ctl->end;
  struct List *new_ = (struct List *)_malloc(sizeof(struct List));
  Obj->next = new_;
  Obj->ctl->end = new_;
  new_->prev = Obj;
  new_->ctl = Obj->ctl;
  new_->next = (List *)NULL;
  new_->val = val;
  new_->ctl->all++;
}
struct List *FindForCount(int count, struct List *Obj) {
  int count_last = GetLastCount(Obj);
  struct List *p = Obj, *q = Obj->ctl->end;
  if (count > count_last)
    return (List *)NULL;
  for (int i = 0, j = count_last;; i++, j--) {
    if (i == count) {
      return p;
    } else if (j == count) {
      return q;
    }
    p = p->next;
    q = q->prev;
  }
}
void DeleteVal(int count, struct List *Obj) {
  struct List *Will_Free = FindForCount(count, Obj);
  if (Will_Free == NULL) {
    // Not found!
    return;
  }
  if (count == 0) {
    return;
  }
  if (Will_Free->next == (List *)NULL) {
    // 是尾节点
    struct List *prev = FindForCount(count - 1, Obj);
    prev->next = (List *)NULL;
    prev->ctl->end = prev;
  } else {
    struct List *prev = FindForCount(count - 1, Obj);
    struct List *next = FindForCount(count + 1, Obj);
    prev->next = next;
    next->prev = prev;
  }
  free(Will_Free);
  Obj->ctl->all--;
}
struct List *NewList() {
  struct List *Obj = (struct List *)_malloc(sizeof(struct List));
  struct ListCtl *ctl = (struct ListCtl *)_malloc(sizeof(struct ListCtl));
  Obj->ctl = ctl;
  Obj->ctl->start = Obj;
  Obj->ctl->end = Obj;
  Obj->val = 0x123456; // 头结点数据不可用
  Obj->prev = (List *)NULL;
  Obj->next = (List *)NULL;
  Obj->ctl->all = 0;
  return Obj;
}
void Change(int count, struct List *Obj, uintptr_t val) {
  struct List *Will_Change = FindForCount(count + 1, Obj);
  if (Will_Change != NULL) {
    Will_Change->val = val;
  } else {
    AddVal(val, Obj);
  }
}
// 获取尾节点的count
int GetLastCount(struct List *Obj) { return Obj->ctl->all; }
void DeleteList(struct List *Obj) {
  Obj = Obj->ctl->start;
  free(Obj->ctl);
  for (; Obj != (struct List *)NULL;) {
    struct List *tmp = Obj;
    Obj = Obj->next;
    free(tmp);
  }
  return;
}
void clean(char *s, int n) {
  for (int i = 0; i < n; i++)
    s[i] = 0;
}
void strtoupper(char *str) {
  while (*str != '\0') {
    if (*str >= 'a' && *str <= 'z') {
      *str -= 32;
    }
    str++;
  }
}
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
int Copy_in(char *path, char *path1) {
  unsigned char *path1_file_buffer;
  if (Fat_FileSize(path) == -1) {
    printf("file not found\n");
    return -1;
  }
  printf("-----------------------------\n");
  Fat_CreateFile(path1);

  path1_file_buffer = (unsigned char *)_malloc(Fat_FileSize(path) + 1);
  int sz = Fat_FileSize(path);
  Fat_ReadFile(path, (char *)path1_file_buffer);
  Fat_WriteFile(path1, (char *)path1_file_buffer, sz);
  free(path1_file_buffer);
  return 0;
}
void Copy_out(char *path, char *path1) {
  fstream in;
  in.open(path, ios::in | ios::binary);
  if (!in.is_open()) {
    return;
  }
  in.seekg(0, ios::end);
  int size = in.tellg();
  in.seekg(0, ios::beg);
  char *ptr = new char[size];
  in.read(ptr, size);
  in.close();
  if (Fat_FileSize(path1) == -1) {
    mkfile(path1); //创建文件
  }
  Fat_WriteFile(path1, ptr, size);
  // 拷贝文件属性
  struct FAT_FILEINFO *finfo1 = Get_File_Address(path1);
  time_t timep;
  struct tm *p;
  time(&timep);
  p = gmtime(&timep);
  finfo1->update_date =
      get_fat_date(p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
  finfo1->update_time = get_fat_time(p->tm_hour, p->tm_min);
  return;
}
void Disk_Read(unsigned int lba, unsigned int number, void *buffer) {
  memcpy(buffer, ADR_DISKIMG + lba * 512, number * 512);
}
void Disk_Write(unsigned int lba, unsigned int number, void *buffer) {
  memcpy(ADR_DISKIMG + lba * 512, buffer, number * 512);
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
void file_loadfile(int clustno, int size, char *buf, int *fat) {
  if (!size) {
    return;
  }
  void *img = _malloc(((size - 1) / ClustnoBytes + 1) * ClustnoBytes);
  for (int i = 0; i != (size - 1) / ClustnoBytes + 1; i++) {
    uint32_t sec =
        (FileDataAddress + (clustno - 2) * ClustnoBytes) / SectorBytes;
    Disk_Read(sec, ClustnoBytes / SectorBytes, img + i * ClustnoBytes);
    clustno = fat[clustno];
  }
  // printf("done 1 %d\n", size);
  memcpy((void *)buf, img, size);
  free(img);
  // printf("done 2\n");
  return;
}
void file_savefile(int clustno, int size, char *buf, int *fat,
                   unsigned char *ff) {
  uint32_t clustall = 0;
  int tmp = clustno;
  int end = clustno_end(type);
  while (fat[clustno] != end) { // 计算文件占多少Fat项 Fat项 = 大小 / 簇大小 + 1
    clustno = fat[clustno];
    clustall++;
  }
  int old_clustno = clustno;
  clustno = tmp;
  int alloc_size;
  if (size > (clustall + 1) *
                 ClustnoBytes) { // 新大小 > (旧大小 / 簇大小 + 1) * 簇大小
    // 请求内存大小 = (新大小 / 簇大小 + 1) * 簇大小
    alloc_size = ((size - 1) / ClustnoBytes + 1) * ClustnoBytes;
    // 分配Fat（这里需要在写盘前分配）
    for (int size1 = size; size1 > ((clustall + 1) * ClustnoBytes);
         size1 -= ClustnoBytes) {
      for (int i = 0; i != FatMaxTerms; i++) {
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
             (clustall + 1) * ClustnoBytes) { // 新大小 <= (旧大小 / 簇大小
                                              // + 1) * 簇大小
    // 请求内存大小 = (旧大小 / 簇大小 + 1) * 簇大小
    alloc_size = (clustall + 1) * ClustnoBytes;
    // 这里不分配Fat的原因是要清空更改后多余的数据
  }
  void *img = _malloc(alloc_size);
  clean((char *)img, alloc_size);
  memcpy(img, buf, size); // 把要写入的数据复制到新请求的内存地址
  for (int i = 0; i != (alloc_size / ClustnoBytes); i++) {
    uint32_t sec =
        (FileDataAddress + (clustno - 2) * ClustnoBytes) / SectorBytes;
    Disk_Write(sec, ClustnoBytes / SectorBytes, img + i * ClustnoBytes);
    clustno = fat[clustno];
  }
  free(img);
  if (size < clustall * ClustnoBytes) { // 新大小 < (旧大小 / 簇大小) * 簇大小
    // 分配Fat（中间情况没必要分配）
    int i = old_clustno;
    for (int size1 = clustall * ClustnoBytes; size1 > size;
         size1 -= ClustnoBytes) {
      fat[i] = 0;
      ff[i] = false;
      for (int j = 0; j != FatMaxTerms; j++) {
        if (fat[j] == i) {
          i = j;
        }
      }
    }
    old_clustno = i;
    fat[old_clustno] = end;
    ff[old_clustno] = true;
  }
  file_savefat(fat, 0, FatMaxTerms);
}
void file_saveinfo(struct FAT_FILEINFO *directory) {
  if (directory == root_directory) {
    Disk_Write(RootDictAddress / SectorBytes, RootMaxFiles * 32 / SectorBytes,
               (void *)directory);
  } else {
    for (int i = 1; FindForCount(i, directory_list) != NULL; i++) {
      struct List *list = FindForCount(i, directory_list);
      if ((struct FAT_FILEINFO *)list->val == directory) {
        list = FindForCount(i, directory_clustno_list);
        Disk_Write((FileDataAddress + (list->val - 2) * ClustnoBytes) /
                       SectorBytes,
                   ClustnoBytes / SectorBytes, (void *)directory);
        break;
      }
    }
  }
}
void file_savefat(int *fat, int clustno, int length) {
  unsigned char *img = (unsigned char *)ADR_DISKIMG + Fat1Address;
  unsigned char *img1 = (unsigned char *)ADR_DISKIMG + Fat2Address;
  int size, sec;
  if (type == 12) {
    if (clustno % 2 != 0) {
      clustno--;
      length++;
    }
    for (int i = 0; i <= (length / 3 + 1) * 2; i += 2) {
      img[(clustno + i) * 3 / 2 + 0] = fat[clustno + i + 0] & 0xff;
      img[(clustno + i) * 3 / 2 + 1] =
          (fat[(clustno + i) + 0] >> 8 | fat[clustno + i + 1] << 4) & 0xff;
      img[(clustno + i) * 3 / 2 + 2] = (fat[clustno + i + 1] >> 4) & 0xff;
      img1[(clustno + i) * 3 / 2 + 0] = fat[clustno + i + 0] & 0xff;
      img1[(clustno + i) * 3 / 2 + 1] =
          (fat[(clustno + i) + 0] >> 8 | fat[clustno + i + 1] << 4) & 0xff;
      img1[(clustno + i) * 3 / 2 + 2] = (fat[clustno + i + 1] >> 4) & 0xff;
    }
    size = length * 3 / 2;
    sec = clustno * 3 / 2;
  } else if (type == 16) {
    for (int i = 0; i <= length; i++) {
      img[(clustno + i) * 2 + 0] = fat[clustno + i] & 0xff;
      img[(clustno + i) * 2 + 1] = (fat[clustno + i] >> 8) & 0xff;
      img1[(clustno + i) * 2 + 0] = fat[clustno + i] & 0xff;
      img1[(clustno + i) * 2 + 1] = (fat[clustno + i] >> 8) & 0xff;
    }
    size = length * 2 - 1;
    sec = clustno * 2;
  } else if (type == 32) {
    for (int i = 0; i <= length; i++) {
      img[(clustno + i) * 4 + 0] = fat[clustno + i] & 0xff;
      img[(clustno + i) * 4 + 1] = (fat[clustno + i] >> 8) & 0xff;
      img[(clustno + i) * 4 + 2] = (fat[clustno + i] >> 16) & 0xff;
      img[(clustno + i) * 4 + 3] = fat[clustno + i] >> 24;
      img1[(clustno + i) * 4 + 0] = fat[clustno + i] & 0xff;
      img1[(clustno + i) * 4 + 1] = (fat[clustno + i] >> 8) & 0xff;
      img1[(clustno + i) * 4 + 2] = (fat[clustno + i] >> 16) & 0xff;
      img1[(clustno + i) * 4 + 3] = fat[clustno + i] >> 24;
    }
    size = length * 4 - 1;
    sec = clustno * 4;
  }
  Disk_Write((Fat1Address + sec) / SectorBytes, size / SectorBytes + 1,
             ADR_DISKIMG + Fat1Address);
  Disk_Write((Fat2Address + sec) / SectorBytes, size / SectorBytes + 1,
             ADR_DISKIMG + Fat2Address);
  /*write_fat( ADR_DISKIMG +  Fat1Address, fat,
             FatMaxTerms,  type);
  write_fat( ADR_DISKIMG +  Fat2Address, fat,
             FatMaxTerms,  type);
  Disk_Write( Fat1Address /  SectorBytes,
             ( Fat2Address -  Fat1Address) /
                  SectorBytes,
              ADR_DISKIMG +  Fat2Address);
  Disk_Write( Fat2Address /  SectorBytes,
             ( Fat2Address -  Fat1Address) /
                  SectorBytes,
              ADR_DISKIMG +  Fat2Address);*/
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
struct FAT_FILEINFO *Get_File_Address(char *path1) {
  // TODO: Modifly it
  struct FAT_FILEINFO *bmpDict = dir;
  // printf("Get_File_Address :%s\n", path1);
  char *path = (char *)_malloc(strlen(path1) + 1);
  char *bmp = path;
  strcpy(path, path1);
  strtoupper(path);
  if (strncmp("/", path, 1) == 0) {
    path += 1;
    bmpDict = root_directory;
  }
  // printf("strlen(path) = %d\n",strlen(path));
  if (path[0] == '\\' || path[0] == '/') {
    //跳过反斜杠和正斜杠
    for (int i = 0; i < strlen(path); i++) {
      if (path[i] != '\\' && path[i] != '/') {
        path += i;
        break;
      }
    }
  }
  // printf("strlen(path) = %d\n",strlen(path));
  char *temp_name = (char *)_malloc(128);
  // printf("%08x %08x\n", path, temp_name);
  struct FAT_FILEINFO *finfo;
  int i = 0;
  while (1) {
    int j;
    // printf("strlen(path) = %d\n",strlen(path));
    for (j = 0; i < strlen(path); i++, j++) {
      //    printf("i = %d %c %d\n",i,path[i],strlen(path));
      if (path[i] == '\\' || path[i] == '/') {
        temp_name[j] = '\0';
        // printf("Got It:%s,ALL:%s\n", temp_name, path);
        i++;
        break;
      }
      temp_name[j] = path[i];
      // sleep(100);
    }
    finfo = dict_search(temp_name, bmpDict, RootMaxFiles);
    if (finfo == 0) {
      if (path[i] != '\0') {
        free(temp_name);
        free(bmp);
        return 0;
      }
      finfo = file_search(temp_name, bmpDict, RootMaxFiles);
      if (finfo == 0) {
        free(temp_name);
        free(bmp);
        return 0;
      } else {
        goto END;
      }
    } else {
      // printf("dict_search:%s ", temp_name);
      if (get_clustno(finfo->clustno_high, finfo->clustno_low) != 0) {
        for (int count = 1; FindForCount(count, directory_clustno_list) != NULL;
             count++) {
          struct List *list = FindForCount(count, directory_clustno_list);
          if (get_clustno(finfo->clustno_high, finfo->clustno_low) ==
              list->val) {
            list = FindForCount(count, directory_list);
            bmpDict = (struct FAT_FILEINFO *)list->val;
            // printf("finfo:%08x\n", bmpDict);
            break;
          }
        }
      } else {
        // printf("finfo:%08x\n",
        //  root_directory);
        bmpDict = root_directory;
      }
      clean(temp_name, 128);
    }
  }
END:
  // printf("file_search:%s finfo:%08x\n", temp_name, finfo);
  free((void *)temp_name);
  free((void *)bmp);
  return finfo;
}
struct FAT_FILEINFO *Get_dictaddr(char *path1) {
  // TODO: Modifly it
  struct FAT_FILEINFO *bmpDict = dir;
  char *path = (char *)_malloc(strlen(path1) + 1);
  char *bmp = path;
  strcpy(path, path1);
  strtoupper(path);
  if (strncmp("/", path, 1) == 0) {
    path += 1;
    bmpDict = root_directory;
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
  char *temp_name = (char *)_malloc(128);
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
    finfo = dict_search(temp_name, bmpDict, RootMaxFiles);
    if (finfo == 0) {
      goto END;
    } else {
      if (get_clustno(finfo->clustno_high, finfo->clustno_low) != 0) {
        for (int count = 1; FindForCount(count, directory_clustno_list) != NULL;
             count++) {
          struct List *list = FindForCount(count, directory_clustno_list);
          // printf("count:%d val:%d\n", count, list->val);
          if (get_clustno(finfo->clustno_high, finfo->clustno_low) ==
              list->val) {
            list = FindForCount(count, directory_list);
            // printf("clustno:%d change %08x to %d %08x\n",
            //        get_clustno(finfo->clustno_high, finfo->clustno_low),
            //        bmpDict, count, list->val);
            bmpDict = (struct FAT_FILEINFO *)list->val;

            break;
          } else {
            // printf("list->val = %08x finfo->clustno_low = %08x %08x\n",
            // list->val,
            //   finfo->clustno_low,
            //    directory_clustno_list);
          }
        }
        // printf(
        //     "Not Found %08x,and "
        //     "FindForCount(count, directory_"
        //     "clustno_list)=%08x %08x\n",
        //     finfo->clustno_low,
        //     FindForCount(
        //         1,
        //          directory_clustno_list), directory_clustno_list);
      } else {
        // printf("finfo->clustno_low == 0\n");
        bmpDict = root_directory;
      }
      clean(temp_name, 128);
      if (path[i] == '\0') {
        goto END;
      }
    }
  }
END:
  free(temp_name);
  free(bmp);
  // printf("bmpDict=%08x root=%08x\n", bmpDict,
  //        root_directory);
  return bmpDict;
}
void mkdir(char *dictname, int last_clust) {
  /*
          dictname:目录名
          last_clust:上一级目录的簇号
  */
  mkfile(dictname);
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
  time_t timep;
  struct tm *p;
  time(&timep);
  p = gmtime(&timep);
  model[2].update_date =
      get_fat_date(p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
  model[2].update_time = get_fat_time(p->tm_hour, p->tm_min);
  for (int i = 0; i != FatMaxTerms; i++) {
    if (!fat[i]) {
      model[2].clustno_low = i;
      int end = clustno_end(type);
      fat[i] = end;
      FatClustnoFlags[i] = true;
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
  struct FAT_FILEINFO *finfo1 = Get_File_Address(dictname);
  model[0].type = 0x10;
  model[0].clustno_low = finfo1->clustno_low; // 指向自己
  model[0].clustno_high = finfo1->clustno_high;
  model[0].size = 0;
  model[0].update_date =
      get_fat_date(p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
  model[0].update_time = get_fat_time(p->tm_hour, p->tm_min);
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
      get_fat_date(p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
  model[1].update_time = get_fat_time(p->tm_hour, p->tm_min);
  model[1].type = 0x10;
  Fat_WriteFile(dictname, (char *)&model[0], 32 * 3);
  struct FAT_FILEINFO *finfo = Get_File_Address(dictname);
  finfo->type = 0x10; // 是目录（文件夹的type属性是0x10）
  finfo->size = 0;
  AddVal(get_clustno(finfo->clustno_high, finfo->clustno_low),
         directory_clustno_list);
  // for (int count = 1;
  //      FindForCount(count,
  //                    directory_clustno_list) !=
  //      NULL;
  //      count++) {
  //   printf("%02x ",
  //          FindForCount(count,
  //                        directory_clustno_list)
  //              ->val);
  // }
  if (last_clust == 0) {
    file_saveinfo(root_directory);
  } else {
    for (int i = 1; FindForCount(i, directory_clustno_list) != NULL; i++) {
      struct List *list = FindForCount(i, directory_clustno_list);
      if (list->val == last_clust) {
        list = FindForCount(i, directory_list);
        struct FAT_FILEINFO *d_finfo = (struct FAT_FILEINFO *)list->val;
        file_saveinfo(d_finfo);
      }
    }
  }
  file_savefat(fat, get_clustno(model[2].clustno_high, model[2].clustno_low),
               1);
  void *directory_alloc = _malloc(ClustnoBytes);
  Disk_Read((FileDataAddress +
             (get_clustno(finfo->clustno_high, finfo->clustno_low) - 2) *
                 ClustnoBytes) /
                SectorBytes,
            ClustnoBytes / SectorBytes, directory_alloc);
  AddVal((uintptr_t)directory_alloc, directory_list);
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
int del(char *cmdline) {
  //删除某个文件
  char *name = cmdline;
  int i;
  struct FAT_FILEINFO *finfo;
  // for (i = 0; i < strlen(cmdline); i++) {
  //   name[i] = cmdline[i + 4];
  // }
  finfo = Get_File_Address(name);
  if (finfo == 0 || finfo->type == 0x01 || finfo->type == 0x04) {
    return 0;
  }
  // MYFILE* fp = my_fopen(name, "wb");
  // for (i = 0; i != fp->size; i++) {
  //   my_fputc(0, fp);
  // }
  // fp->size = 0;
  // my_fclose(fp);
  // char* buf = _malloc(finfo->size);
  // memset(buf, 0, finfo->size);
  // Fat_WriteFile(name, buf, finfo->size);
  file_savefile(get_clustno(finfo->clustno_high, finfo->clustno_low), 0, NULL,
                fat, FatClustnoFlags);
  finfo->name[0] = 0xe5;
  fat[get_clustno(finfo->clustno_high, finfo->clustno_low)] = 0;
  FatClustnoFlags[get_clustno(finfo->clustno_high, finfo->clustno_low)] = false;
  file_saveinfo(Get_dictaddr(name));
  file_savefat(fat, get_clustno(finfo->clustno_high, finfo->clustno_low), 1);
  return 1;
}
int deldir(char *path) {
  struct FAT_FILEINFO *finfo = Get_dictaddr(path);
  if (finfo == Get_dictaddr(".")) {
    return 0;
  }
  struct FAT_FILEINFO *f = dir;
  dir = finfo;
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
      // printf("(IN)DEL MYFILE:%s\n", s);
      if (del(s) == -1) {
        return 0;
      }
    }
  }
  dir = f;
  // printf("(IN)DEL SELF\n");
  struct FAT_FILEINFO *root_finfo;
  if (finfo[1].clustno_low == 0) {
    root_finfo = root_directory;
  } else {
    for (int i = 1; FindForCount(i, directory_clustno_list) != NULL; i++) {
      if (FindForCount(i, directory_clustno_list)->val ==
          finfo[1].clustno_low) {
        root_finfo =
            (struct FAT_FILEINFO *)FindForCount(i, directory_list)->val;
        // printf("FIND ROOT %08x\n", root_finfo);
      }
    }
  }
  for (int i = 0; root_finfo[i].name[0] != '\0'; i++) {
    // printf("ROOT MYFILE:%s\n", root_finfo[i].name);
    if (root_finfo[i].clustno_low == finfo[0].clustno_low) {
      root_finfo[i].name[0] = 0xe5;
      break;
    }
  }
  fat[get_clustno(finfo->clustno_high, finfo->clustno_low)] = 0;
  FatClustnoFlags[get_clustno(finfo->clustno_high, finfo->clustno_low)] = false;
  file_saveinfo(Get_dictaddr(path));
  file_savefat(fat, get_clustno(finfo->clustno_high, finfo->clustno_low), 1);
  return 1;
}
void mkfile(char *name) {
  char s[12];
  int i, j;
  struct FAT_FILEINFO *finfo = Get_dictaddr(name);

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
  for (int i = 0; i != FatMaxTerms; i++) {
    if (!fat[i]) {
      finfo->clustno_low = i & 0xffff;
      finfo->clustno_high = i >> 16;
      int end = clustno_end(type);
      fat[i] = end;
      FatClustnoFlags[i] = true;
      break;
    }
  }
  finfo->reserve = 0;
  time_t timep;
  struct tm *p;
  time(&timep);
  p = gmtime(&timep);
  finfo->update_date =
      get_fat_date(p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
  finfo->update_time = get_fat_time(p->tm_hour, p->tm_min);
  finfo->size = 0;
  file_saveinfo(Get_dictaddr(path));
  file_savefat(fat, get_clustno(finfo->clustno_high, finfo->clustno_low), 1);
  return;
}
int changedict(char *dictname) {
  // cd命令的依赖函数
  strtoupper(dictname);

  if (strcmp(dictname, "/") == 0) {
    dir = root_directory;
    return 1;
  }
  struct FAT_FILEINFO *finfo = dict_search(dictname, dir, RootMaxFiles);
  //找文件夹
  if (finfo == 0) {
    //没找到
    return 0;
  }
  if (get_clustno(finfo->clustno_high, finfo->clustno_low) == 0) {
    //根目录
    dir = root_directory;
    return 1;
  }
  //..进行特殊的处理
  //.不进行处理
  //其他按照下面的方式处理
  for (int count = 1; FindForCount(count, directory_clustno_list) != NULL;
       count++) {
    struct List *list = FindForCount(count, directory_clustno_list);
    if (get_clustno(finfo->clustno_high, finfo->clustno_low) == list->val) {
      list = FindForCount(count, directory_list);
      dir = (struct FAT_FILEINFO *)list->val;
      break;
    }
  }
  return 1;
}
int rename(char *src_name, char *dst_name) {
  strtoupper(src_name);
  strtoupper(dst_name);
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
  struct FAT_FILEINFO *finfo = Get_File_Address(src_name);
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
  file_saveinfo(Get_dictaddr(src_name));
  return 1;
}
int attrib(char *filename, ftype type) {
  struct FAT_FILEINFO *finfo = Get_File_Address(filename);
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
  file_saveinfo(Get_dictaddr(filename));
  return 1;
}
int format(char *mbr_path, int fat_type) {
  fstream in;
  in.open(path, ios::in | ios::binary);
  if (!in.is_open()) {
    return 0;
  }
  in.seekg(0, ios::end);
  int size = in.tellg();
  in.seekg(0, ios::beg);
  char *ptr = new char[size];
  in.read(ptr, size);
  in.close();
  void *read_in = ptr;
  if (fat_type == 12 &&
      (memcmp(read_in + BS_FileSysType, "FAT12   ", 8) == 0 ||
       memcmp(read_in + BS_FileSysType, "FAT16   ", 8) == 0)) {
    *(unsigned char *)(&((unsigned char *)read_in)[BPB_SecPerClus]) = 1;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_RootEntCnt]) = 224;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) =
        imgTotalSize / 512;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec32]) = 0;
    unsigned short fatsz = (imgTotalSize - 1) / 512 * 3 / 2 / 512 + 1;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_FATSz16]) = fatsz;
    *(unsigned char *)(&((unsigned char *)read_in)[BS_DrvNum]) = 0;
    memcpy((void *)&((unsigned char *)read_in)[BS_FileSysType],
           (void *)"FAT12   ", 8);
    *(unsigned int *)(&((unsigned char *)read_in)[BS_VolD]) = 114514;
    memcpy((void *)&((unsigned char *)read_in)[BS_VolLab],
           (void *)"POWERINTDOS", 11);
    Disk_Write(0, 1, (unsigned short *)read_in);
    unsigned int *fat = (unsigned int *)_malloc(fatsz * 512);
    fat[0] = 0x00fffff0;
    Disk_Write(1, fatsz, (unsigned short *)fat);
    Disk_Write(1 + fatsz, fatsz, (unsigned short *)fat);
    free((void *)fat);
    void *null_sec = _malloc(512);
    clean((char *)null_sec, 512);
    for (int i = 0; i != 224 * 32 / 512; i++) {
      Disk_Write(1 + fatsz * 2 + i, 1, (unsigned short *)null_sec);
    }
    free(null_sec);
    // free((void*)info);
  } else if (fat_type == 16 &&
             (memcmp(read_in + BS_FileSysType, "FAT12   ", 8) == 0 ||
              memcmp(read_in + BS_FileSysType, "FAT16   ", 8) == 0)) {
    unsigned int clustno_size = ((imgTotalSize - 1) / 65536 + 512) / 512 * 512;
    *(unsigned char *)(&((unsigned char *)read_in)[BPB_SecPerClus]) =
        clustno_size / 512;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_RootEntCnt]) =
        14 * clustno_size / 32;
    if (imgTotalSize / 512 > 65535) {
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) = 0;
    } else {
      *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) =
          imgTotalSize / 512;
    }
    *(unsigned int *)(&((unsigned char *)read_in)[BPB_TotSec32]) =
        imgTotalSize / 512;
    unsigned short fatsz = (imgTotalSize - 1) / clustno_size * 2 / 512 + 1;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_FATSz16]) = fatsz;
    *(unsigned char *)(&((unsigned char *)read_in)[BS_DrvNum]) = 0;
    memcpy((void *)&((unsigned char *)read_in)[BS_FileSysType],
           (void *)"FAT16   ", 8);
    *(unsigned int *)(&((unsigned char *)read_in)[BS_VolD]) = rand();
    memcpy((void *)&((unsigned char *)read_in)[BS_VolLab],
           (void *)"POWERINTDOS", 11);
    Disk_Write(0, 1, (unsigned short *)read_in);
    unsigned short *fat = (unsigned short *)_malloc(fatsz * 512);
    fat[0] = 0xfff0;
    fat[1] = 0xffff;
    Disk_Write(1, fatsz, (void *)fat);
    Disk_Write(1 + fatsz, fatsz, (void *)fat);
    free((void *)fat);
    void *null_sec = _malloc(512);
    clean((char *)null_sec, 512);
    for (int i = 0; i < 14 * clustno_size / 512; i++) {
      Disk_Write(1 + fatsz * 2 + i, 1, (unsigned short *)null_sec);
    }
    free(null_sec);
  } else if (fat_type == 32 &&
             memcmp(read_in + BS_FileSysType + BPB_Fat32ExtByts, "FAT32   ",
                    8) == 0) {
    unsigned int clustno_size = (imgTotalSize - 1) / 268435456 * 512 + 512;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_RsvdSecCnt]) = 1;
    *(unsigned char *)(&((unsigned char *)read_in)[BPB_SecPerClus]) =
        clustno_size / 512;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_RootEntCnt]) = 0;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_TotSec16]) = 0;
    *(unsigned int *)(&((unsigned char *)read_in)[BPB_TotSec32]) =
        imgTotalSize / 512;
    unsigned int fatsz = (imgTotalSize - 1) / clustno_size * 4 / 512 + 1;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_FATSz16]) = 0;
    *(unsigned int *)(&((unsigned char *)read_in)[BPB_FATSz32]) = fatsz;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_ExtFlags]) = 0;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_FSVer]) = 0;
    *(unsigned int *)(&((unsigned char *)read_in)[BPB_RootClus]) = 2;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_FSInfo]) = 0;
    *(unsigned short *)(&((unsigned char *)read_in)[BPB_BkBootSec]) = 0;
    *(unsigned long long *)(&((unsigned char *)read_in)[BPB_Reserved]) = 0;
    *(unsigned char *)(&(
        (unsigned char *)read_in)[BS_DrvNum + BPB_Fat32ExtByts]) = 0;
    *(unsigned char *)(&(
        (unsigned char *)read_in)[BS_Reserved1 + BPB_Fat32ExtByts]) = 0;
    *(unsigned char *)(&(
        (unsigned char *)read_in)[BS_BootSig + BPB_Fat32ExtByts]) = 0x29;
    memcpy(
        (void *)&((unsigned char *)read_in)[BS_FileSysType + BPB_Fat32ExtByts],
        (void *)"FAT32   ", 8);
    *(unsigned int *)(&((unsigned char *)read_in)[BS_VolD + BPB_Fat32ExtByts]) =
        rand();
    memcpy((void *)&((unsigned char *)read_in)[BS_VolLab + BPB_Fat32ExtByts],
           (void *)"POWERINTDOS", 11);
    Disk_Write(0, 1, (unsigned short *)read_in);
    unsigned int *fat = (unsigned int *)_malloc(fatsz * 512);
    fat[0] = 0xffffff0;
    fat[1] = 0xfffffff;
    fat[2] = 0xfffffff;
    Disk_Write(1, fatsz, (void *)fat);
    Disk_Write(1 + fatsz, fatsz, (void *)fat);
    free((void *)fat);
    void *null_sec = _malloc(512);
    clean((char *)null_sec, 512);
    Disk_Write(1 + fatsz * 2, 1, (unsigned short *)null_sec);
    free(null_sec);
  }
  fat_InitFS();
  return 1;
}
void fat_InitFS() {
  void *boot_sector = _malloc(512);
  Disk_Read(0, 1, boot_sector);

  if (memcmp(boot_sector + BS_FileSysType, "FAT12   ", 8) == 0) {
    type = 12;
  } else if (memcmp(boot_sector + BS_FileSysType, "FAT16   ", 8) == 0) {
    type = 16;
  } else if (memcmp(boot_sector + BS_FileSysType + BPB_Fat32ExtByts, "FAT32   ",
                    8) == 0) {
    type = 32;
  }
  SectorBytes = *(unsigned short *)(boot_sector + BPB_BytsPerSec);
  RootMaxFiles = *(unsigned short *)(boot_sector + BPB_RootEntCnt);
  ClustnoBytes = SectorBytes * *(unsigned char *)(boot_sector + BPB_SecPerClus);
  Fat1Address = *(unsigned short *)(boot_sector + BPB_RsvdSecCnt) * SectorBytes;
  if (type != 32) {
    RootDictAddress = (*(unsigned char *)(boot_sector + BPB_NumFATs) *
                           *(unsigned short *)(boot_sector + BPB_FATSz16) +
                       *(unsigned short *)(boot_sector + BPB_RsvdSecCnt)) *
                      SectorBytes;
    FileDataAddress = RootDictAddress + RootMaxFiles * 32;
    if (*(unsigned short *)(boot_sector + BPB_TotSec16) != 0) {
      imgTotalSize =
          *(unsigned short *)(boot_sector + BPB_TotSec16) * SectorBytes;
    } else {
      imgTotalSize =
          *(unsigned int *)(boot_sector + BPB_TotSec32) * SectorBytes;
    }
    Fat2Address = Fat1Address +
                  *(unsigned short *)(boot_sector + BPB_FATSz16) * SectorBytes;
  } else {
    FileDataAddress = (*(unsigned char *)(boot_sector + BPB_NumFATs) *
                           *(unsigned int *)(boot_sector + BPB_FATSz32) +
                       *(unsigned short *)(boot_sector + BPB_RsvdSecCnt)) *
                      SectorBytes;
    RootDictAddress =
        FileDataAddress +
        (*(unsigned int *)(boot_sector + BPB_RootClus) - 2) * ClustnoBytes;
    imgTotalSize = *(unsigned int *)(boot_sector + BPB_TotSec32) * SectorBytes;
    Fat2Address = Fat1Address +
                  *(unsigned int *)(boot_sector + BPB_FATSz32) * SectorBytes;
    RootMaxFiles = ClustnoBytes / 32;
  }
  FatMaxTerms = (Fat2Address - Fat1Address) * 8 / type;

  // printf("type:FAT%d init done.\n", type);
  // printf("ClustnoBytes:%d\n", ClustnoBytes);
  // printf("RootMaxFiles:%d\n", RootMaxFiles);
  // printf("RootDictAddress:%08x\n", RootDictAddress);
  // printf("FileDataAddress:%08x\n", FileDataAddress);
  // printf("imgTotalSize:%d\n", imgTotalSize);
  // printf("SectorBytes:%d\n", SectorBytes);
  // printf("Fat1Address:%08x\n", Fat1Address);
  // printf("Fat2Address:%08x\n", Fat2Address);
  // printf("FatMaxTerms:%d\n", FatMaxTerms);

  fat = (int *)_malloc(FatMaxTerms * sizeof(int));
  FatClustnoFlags = (unsigned char *)_malloc(FatMaxTerms * sizeof(char));
  read_fat((unsigned char *)(ADR_DISKIMG + (unsigned int)Fat1Address), fat,
           FatClustnoFlags, FatMaxTerms, type);
  root_directory = (struct FAT_FILEINFO *)_malloc(RootMaxFiles * 32);
  memcpy((void *)root_directory, (void *)ADR_DISKIMG + RootDictAddress,
         RootMaxFiles * 32);
  directory_list = (struct List *)NewList();
  directory_clustno_list = (struct List *)NewList();
  struct FAT_FILEINFO *finfo = root_directory;
  dir = root_directory;

  for (int i = 0; i != RootMaxFiles; i++) {
    if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
      AddVal(get_clustno(finfo[i].clustno_high, finfo[i].clustno_low),
             (struct List *)directory_clustno_list);
      void *directory_alloc = _malloc(ClustnoBytes);
      uint32_t sec1 =
          (FileDataAddress +
           (get_clustno(finfo[i].clustno_high, finfo[i].clustno_low) - 2) *
               ClustnoBytes) /
          SectorBytes;
      Disk_Read(sec1, ClustnoBytes / SectorBytes, directory_alloc);
      AddVal((uintptr_t)directory_alloc, (struct List *)directory_list);
    }
    if (finfo[i].name[0] == 0) {
      break;
    }
  }

  for (int i = 1; FindForCount(i, (struct List *)directory_list) != NULL; i++) {
    struct List *list = FindForCount(i, (struct List *)directory_list);
    finfo = (struct FAT_FILEINFO *)list->val;
    for (int j = 0; j != ClustnoBytes / 32; j++) {
      if (finfo[j].type == 0x10 && finfo[j].name[0] != 0xe5 &&
          strncmp(".", (char *)finfo[j].name, 1) != 0 &&
          strncmp("..", (char *)finfo[j].name, 2) != 0) {
        AddVal(get_clustno(finfo[j].clustno_high, finfo[j].clustno_low),
               (struct List *)directory_clustno_list);
        void *directory_alloc = _malloc(ClustnoBytes);
        uint32_t sec1 =
            (FileDataAddress +
             (get_clustno(finfo[j].clustno_high, finfo[j].clustno_low) - 2) *
                 ClustnoBytes) /
            SectorBytes;
        Disk_Read(sec1, ClustnoBytes / SectorBytes, directory_alloc);
        AddVal((uintptr_t)directory_alloc, (struct List *)directory_list);
      }
      if (finfo[j].name[0] == 0) {
        break;
      }
    }
  }
  free(boot_sector);
}
bool Fat_cd(char *dictName) { return changedict(dictName); }
bool Fat_ReadFile(char *path, char *buffer) {
  // printf("Fat12 Read ---- %s\n",path);
  struct FAT_FILEINFO *finfo;
  finfo = Get_File_Address(path);
  //  printk("finfo = %08x\n", finfo);
  if (finfo == 0) {
    return 0;
  } else {
    file_loadfile(get_clustno(finfo->clustno_high, finfo->clustno_low),
                  finfo->size, buffer, fat);
    return 1;
  }
}
bool Fat_WriteFile(char *path, char *buffer, int size) {
  struct FAT_FILEINFO *finfo = Get_File_Address(path);
  // printf("finfo = %08x\n",finfo);
  file_savefile(get_clustno(finfo->clustno_high, finfo->clustno_low), size,
                buffer, fat, FatClustnoFlags);
  // printf("file save ok\n");
  finfo->size = size;
  file_saveinfo(Get_dictaddr(path));
}
List *Fat_ListFile(char *dictpath) {
  struct FAT_FILEINFO *finfo = Get_dictaddr(dictpath);
  List *result = NewList();
  char s[30];
  for (int i = 0; i != 30; i++) {
    s[i] = 0;
  }
  for (int i = 0; i < RootMaxFiles; i++) {
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
          vfs_file *d = (vfs_file *)_malloc(sizeof(vfs_file));
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
          AddVal((uintptr_t)d, result);
        }
      }
    }
  }
  return result;
}
bool Fat_RenameFile(char *filename, char *filename_of_new) {
  return rename(filename, filename_of_new);
}
bool Fat_CreateFile(char *filename) {
  mkfile(filename);
  return true;
}
bool Fat_Check() {
  uint8_t *boot_sec = (uint8_t *)_malloc(512);
  Disk_Read(0, 1, boot_sec);
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
bool Fat_DelFile(char *path) { return del(path); }
bool Fat_DelDict(char *path) { return deldir(path); }
int Fat_FileSize(char *filename) {
  if (Get_File_Address(filename) == NULL) {
    return -1;
  }
  return Get_File_Address(filename)->size;
}
bool Fat_CreateDict(char *filename) {
  struct FAT_FILEINFO *finfo = Get_dictaddr(filename);
  int last_clustno = finfo[0].clustno_low;
  if (finfo == root_directory) {
    last_clustno = 0;
  }
  mkdir(filename, last_clustno);
}
bool Fat_Attrib(char *filename, ftype type) { return attrib(filename, type); }
vfs_file *Fat_FileInfo(char *filename) {
  struct FAT_FILEINFO *finfo = Get_File_Address(filename);
  if (finfo == NULL) {
    return (vfs_file *)NULL;
  }
  vfs_file *result = (vfs_file *)_malloc(sizeof(vfs_file));
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
void saveImage(void) {
  fstream out;
  out.open(path, ios::out | ios::binary);
  out << ___file;
}
void shell() {
  while (1) {
    printf(":>");
    string _cmd = "";
    getline(cin, _cmd);
    command_run(_cmd);
    printf("\n");
  }
}
void cmd_dir(char **args) {
  vfs_file *file = Fat_FileInfo(args[0]);
  if (file != NULL) {
    printf("%s  %d  %04d-%02d-%02d %02d:%02d  ", file->name, file->size,
           file->year, file->month, file->day, file->hour, file->minute);
    if (file->type == FLE) {
      printf("FILE");
    } else if (file->type == RDO) {
      printf("READ-ONLY");
    } else if (file->type == HID) {
      printf("HIDE");
    } else if (file->type == SYS) {
      printf("SYSTEM-FILE");
    }
    printf("\n");
    free(file);
  } else {
    List *list_of_file = Fat_ListFile(args[0]);
    for (int i = 1; FindForCount(i, list_of_file) != NULL; i++) {
      vfs_file *d = (vfs_file *)FindForCount(i, list_of_file)->val;
      if (d->type == DIR) {
        // SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
        printf("%s ", d->name);
        // free(d->name);
        // SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
      } else if (d->type == RDO) {
        // SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
        printf("%s ", d->name);
        // SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
      } else if (d->type == HID) {
        // SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
        printf("%s ", d->name);
        // SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
      } else if (d->type == SYS) {
        // SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
        printf("%s ", d->name);
        // SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
      } else {
        printf("%s ", d->name);
        // free(d->name);
      }
      free(d);
    }
    DeleteList(list_of_file);
    printf("\n");
  }
  return;
}
void command_run(string commands) {
  if (commands == "cls") {
    system("cls");
  } else if (strncmp(commands.c_str(), "dir", 3) == 0) {
    char *args[1];
    args[0] = (char *)_malloc(256);
    strcpy(args[0], &commands.c_str()[4]);
    cmd_dir(args);
  } else if (strncmp(commands.c_str(), "cd ", 3) == 0) {
    char *ptr = (char *)commands.c_str();
    if (Fat_cd((char *)commands.c_str() + 3) == 0) {
      printf("Invalid directory.\n\n");
    }
  } else if (strncmp(commands.c_str(), "mkfile ", 7) == 0) {
    char *ptr = (char *)commands.c_str();
    Fat_CreateFile(ptr + 7);
  } else if (strncmp(commands.c_str(), "cmdedit ", 8) == 0) {
    char *ptr = (char *)commands.c_str();
    char path[100];
    ptr += 8;
    int i;
    for (i = 0; *ptr != ' '; ptr++, i++)
      path[i] = *ptr;
    path[i] = 0;
    ptr++;
    Fat_WriteFile(path, ptr, strlen(ptr));
  } else if (strncmp(commands.c_str(), "type ", 5) == 0) {
    int size = Fat_FileSize((char *)commands.c_str() + 5);
    if (size == -1) {
      printf("File not find.\n");
    } else {
      char *buffer = (char *)malloc(size);
      Fat_ReadFile((char *)commands.c_str() + 5, buffer);
      for (int i = 0; i != size; i++) {
        printf("%c", buffer[i]);
      }
    }
  } else if (strncmp(commands.c_str(), "attrib ", 7) == 0) {
    char *ptr = (char *)commands.c_str();
    char path[100];
    ptr += 7;
    int i;
    for (i = 0; *ptr != ' '; ptr++, i++)
      path[i] = *ptr;
    path[i] = 0;
    ptr++;
    if (Fat_FileSize(path) == -1) {
      printf("File not find.\n");
      return;
    }
    if (strcmp("READ-ONLY", ptr) == 0) {
      Fat_Attrib(path, RDO);
    } else if (strcmp("HIDE", ptr) == 0) {
      Fat_Attrib(path, HID);
    } else if (strcmp("SYSTEM-FILE", ptr) == 0) {
      Fat_Attrib(path, SYS);
    } else if (strcmp("FILE", ptr) == 0) {
      Fat_Attrib(path, FLE);
    } else {
      printf("Undefined type.\n\n");
    }
  } else if (strncmp(commands.c_str(), "rename ", 7) == 0) {
    char *ptr = (char *)commands.c_str();
    char path[100];
    ptr += 7;
    int i;
    for (i = 0; *ptr != ' '; ptr++, i++)
      path[i] = *ptr;
    path[i] = 0;
    ptr++;
    if (Fat_FileSize(path) == -1) {
      printf("File not find.\n");
      return;
    }
    Fat_RenameFile(path, ptr);
  } else if (strncmp(commands.c_str(), "mkdir ", 6) == 0) {
    Fat_CreateDict((char *)commands.c_str() + 6);
  } else if (strncmp(commands.c_str(), "copy_in ", 8) == 0) {
    char *ptr = (char *)commands.c_str();
    char path_1[100];
    char path_2[100];
    ptr += 8;
    int i;
    for (i = 0; *ptr != ' '; ptr++, i++)
      path_1[i] = *ptr;
    path_1[i] = 0;
    ptr++;
    for (i = 0; *ptr != 0; ptr++, i++)
      path_2[i] = *ptr;
    path_2[i] = 0;
    Copy_in(path_1, path_2);
  } else if (strncmp(commands.c_str(), "copy_out ", 9) == 0) {
    char *ptr = (char *)commands.c_str();
    char path_1[100];
    char path_2[100];
    ptr += 9;
    int i;
    for (i = 0; *ptr != ' '; ptr++, i++)
      path_1[i] = *ptr;
    path_1[i] = 0;
    ptr++;
    for (i = 0; *ptr != 0; ptr++, i++)
      path_2[i] = *ptr;
    path_2[i] = 0;
    Copy_out(path_1, path_2);
  } else if (strncmp(commands.c_str(), "format ", 7) == 0) {
    char mbr_path[100];
    int fat_type;
    sscanf(commands.c_str(), "format %s %d", mbr_path, &fat_type);
    format(mbr_path, fat_type);
  } else if (commands == "save") {
    saveImage();
  } else {
    cout << "Bad Command!\n";
  }
}
int main(int argc, char const *argv[]) {
  bool Path_Ok = false;
  path = "";
  fstream file;
  if (argc == 1) {
    goto _Start;
  } else {
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-cons") == 0) {
        goto _Start; // Console
      } else if (strcmp(argv[i], "-file") == 0) {
        path = argv[++i];
        Path_Ok = true;
        file.open(path, ios::in | ios::binary);
        if (!file) {
          cout << "Please input file address:" << path << endl;
          return -1;
        }
        file.seekg(0, ios::end);
        ___file.resize(file.tellg());
        file.seekg(0, ios::beg);
        file.read(&___file[0], ___file.size());
        if (!Fat_Check()) {
          cout << "Init FAT file system failed" << endl;
          return -1;
        }
        fat_InitFS();
      } else if (strcmp(argv[i], "-mkdir") == 0) {
        if (Path_Ok) {
          char buf[128];
          sprintf(buf, "mkdir %s", argv[++i]);
          command_run(buf);
        }

      } else if (strcmp(argv[i], "-mkfile") == 0) {
        if (Path_Ok) {
          char buf[128];
          sprintf(buf, "mkfile %s", argv[++i]);
          command_run(buf);
        } else {
          return 1;
        }

      } else if (strcmp(argv[i], "-copy") == 0) {
        if (Path_Ok) {
          char buf[128];
          sprintf(buf, "copy_out %s %s", argv[i+1], argv[i+2]);
		  i += 2;
          command_run(buf);
        } else {
          return 1;
        }
      
      } else if (strcmp(argv[i], "-format") == 0) {
        char buf[128];
        sprintf(buf, "format %s %s", argv[i+1], argv[i+2]);
		i += 2;
        command_run(buf);
      
      } else {
        cout << "wrong arg--->" << argv[i] << endl;
      }
    }
  }
  if (Path_Ok) {
    saveImage();
  }
  return 0;
_Start:
  if (!Path_Ok) {
    cout << "Please input file address:";
    getline(cin, path);
    file.open(path, ios::in | ios::binary);
    if (!file) {
      cout << "Wrong file address!" << endl;
      return 0;
    }
  }
  file.seekg(0, ios::end);
  ___file.resize(file.tellg());
  file.seekg(0, ios::beg);
  file.read(&___file[0], ___file.size());
  if (!Fat_Check()) {
    cout << "Init FAT file system failed" << endl;
    return -1;
  }
  fat_InitFS();
  shell();
  return 0;
}

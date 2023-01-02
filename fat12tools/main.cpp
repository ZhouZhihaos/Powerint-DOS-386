// fat12tools
// Copyright (C) min0911_ & zhouzhihao 2022
#include <fstream>
#include <io.h>
#include <iostream>
#include <windows.h>
#define RootDictAddress 0x2600
#define FileDataAddress 0x4200
#define ClustnoBytes 512
#define SectorBytes 512
#define RootMaxFiles 224
#define imgTotalSize 2880 * 512
#define Fat1Address 0x200
#define Fat2Address 0x1400
#define print printf
using namespace std;
string _file;
char *ptr_file;
string ___file;
int dictaddr = RootDictAddress;
string View_path = "";
int change_dict_times = 0;
void strtoupper(char *str);
void command_run(string commands);
struct FILEINFO {
  unsigned char name[8], ext[3], type;
  char reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
};
struct MYFILE {
  unsigned char *buf; // 文件缓冲区
  int size;           // 文件大小
  int p;              // 当前读写位置
  int realloc;        // 内存区域大小
  char *path;         // 文件名
};
int *fat;
unsigned char *FatClustnoFlags;
#define ADR_DISKIMG &___file[0]
string path;
void read_fat(unsigned char *img, int *fat, unsigned char *ff);
void write_fat(unsigned char *img, int *fat);
void file_loadfile(int clustno, int size, char *buf, int *fat);
void file_savefile(int clustno, int size, char *buf, int *fat,
                   unsigned char *ff);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);
struct FILEINFO *dict_search(char *name, struct FILEINFO *finfo, int max);
struct FILEINFO *Get_File_Address(char *path1);
int Get_dictaddr(char *path1);
void mkdir(char *dictname, int last_clust);
void Copy(char *path, char *path1);
struct FILEINFO *clust_sech(int clustno, struct FILEINFO *finfo, int max);
void del(char *cmdline);
void mkfile(char *name);
void changedict(char *dictname);
struct MYFILE *myfopen(char *path, char *mode);
int myfputc(int c, struct MYFILE *fp);
int myfgetc(struct MYFILE *fp);
char *myfgets(char *s, int size, struct MYFILE *fp);
int myfseek(struct MYFILE *fp, int offset, int whence);
int myfclose(struct MYFILE *fp);
int myfread(void *buf, int size, int count, struct MYFILE *fp);
int fsz(char *filename);
void print_date(unsigned short _date, unsigned short _time) {
  unsigned short year = _date & 65024;
  year = year >> 9;
  unsigned short month = _date & 480;
  month = month >> 5;
  unsigned short day = _date & 31;

  unsigned short hour = _time & 63488;
  hour = hour >> 11;
  unsigned short minute = _time & 2016;
  minute = minute >> 5;
  printf("%02u-%02u-%02u %02u:%02u", (year + 1980), month, day, hour, minute);
}
void clean(char *s, int n) {
  for (int i = 0; i < n; i++)
    s[i] = 0;
}
struct MYFILE *myfopen(char *path, char *mode) {
  struct FILEINFO *finfo;
  finfo = Get_File_Address(path);
  if (finfo == 0) {
    return 0;
  } else {
    struct MYFILE *fp = (struct MYFILE *)malloc(sizeof(struct MYFILE));
    if (finfo->size != 0) {
      fp->buf = (unsigned char *)malloc(((finfo->size - 1) / 4096 + 1) * 4096);
      fp->realloc = ((finfo->size - 1) / 4096 + 1) * 4096;
      clean((char *)fp->buf, fp->realloc);
    } else {
      fp->buf = (unsigned char *)malloc(4096);
      fp->realloc = 4096;
      clean((char *)fp->buf, fp->realloc);
    }
    file_loadfile(finfo->clustno, finfo->size, (char *)fp->buf, fat);
    fp->size = finfo->size;
    fp->p = 0; //指向文件的开头
    fp->path = path;
    return fp;
  }
}
// myfputc
int myfputc(int c, struct MYFILE *fp) {
  if (fp->p >= fp->realloc) {
    void *p = malloc(fp->realloc + 4096);
    memcpy(p, (void *)fp->buf, fp->realloc);
    free((void *)fp->buf);
    fp->buf = (unsigned char *)p;
    fp->realloc += 4096;
  }
  if (fp->p >= fp->size) {
    fp->size = fp->p + 1;
  }
  fp->buf[fp->p++] = c;
  return 0;
}
// myfgetc
int myfgetc(struct MYFILE *fp) {
  if (fp->p >= fp->size) {
    return EOF;
  } else {
    return fp->buf[fp->p++];
  }
}
// myfgets
char *myfgets(char *s, int size, struct MYFILE *fp) {
  int i;
  for (i = 0; i < size - 1; i++) {
    int c = myfgetc(fp);
    if (c == EOF) {
      if (i == 0) {
        return 0;
      } else {
        break;
      }
    }
    s[i] = c;
    if (c == '\n') {
      break;
    }
  }
  s[i] = '\0';
  return s;
}
// myfseek
int myfseek(struct MYFILE *fp, int offset, int whence) {
  if (whence == 0) {
    fp->p = offset;
  } else if (whence == 1) {
    fp->p += offset;
  } else if (whence == 2) {
    fp->p = fp->size + offset;
  } else {
    return -1;
  }
  return 0;
}
int myfclose(struct MYFILE *fp) {
  extern int change_dict_times;
  struct FILEINFO *finfo = Get_File_Address(fp->path);
  file_savefile(finfo->clustno, fp->size, (char *)fp->buf, fat,
                FatClustnoFlags);
  int Bmp = dictaddr;
  dictaddr = Get_dictaddr(fp->path);
  while (dictaddr != RootDictAddress) {
    if (dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr),
                    RootMaxFiles) != 0) {
      struct FILEINFO *finfo_this_dict_clust = dict_search(
          ".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), RootMaxFiles);
      struct FILEINFO *finfo_this_dict = clust_sech(
          finfo_this_dict_clust->clustno,
          (struct FILEINFO *)(ADR_DISKIMG + Get_dictaddr("../")), RootMaxFiles);
      finfo_this_dict->size += (fp->size - finfo->size);
    }
    changedict("..");
  }
  dictaddr = Bmp;
  finfo->size = fp->size; // 大小更改
  free((void *)fp->buf);
  free((void *)fp);
  return 0;
}
// myfread
int myfread(void *buf, int size, int count, struct MYFILE *fp) {
  int i;
  for (i = 0; i < size * count; i++) {
    if (fp->p >= fp->size) {
      break;
    }
    ((unsigned char *)buf)[i] = fp->buf[fp->p++];
    // printk("%02x ", ((unsigned char*)buf)[i]);
  }
  return i;
}
int fsz(char *filename) {
  struct FILEINFO *finfo = Get_File_Address(filename);
  if (finfo == 0) {
    return -1;
  } else {
    return finfo->size;
  }
}
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
  clean((char *)ff, 3072 * sizeof(char));
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
void file_loadfile(int clustno, int size, char *buf, int *fat) {
  int i;
  char *img = ADR_DISKIMG + (FileDataAddress - ClustnoBytes * 2);
  for (;;) {
    if (size <= ClustnoBytes) {
      for (i = 0; i < size; i++) {
        buf[i] = img[clustno * ClustnoBytes + i];
      }
      break;
    }
    for (i = 0; i < ClustnoBytes; i++) {
      buf[i] = img[clustno * ClustnoBytes + i];
    }
    size -= ClustnoBytes;
    buf += ClustnoBytes;
    clustno = fat[clustno];
  }
  return;
}
void file_savefile(int clustno, int size, char *buf, int *fat,
                   unsigned char *ff) {
  uint32_t clustall = 0;
  while (fat[clustno + clustall] != 0xfff) {
    clustall++;
  }
  int old_clustno = clustno + clustall;
  if (size > (clustall + 1) * ClustnoBytes) {
    for (int size1 = size; size1 > ((clustall + 1) * ClustnoBytes);
         size1 -= ClustnoBytes) {
      for (int i = 0; i != 3072; i++) {
        if (!ff[i]) {
          memcpy((void *)&fat[old_clustno + 1], (void *)&fat[old_clustno],
                 (3072 - (old_clustno + 1)) *
                     sizeof(int));
          fat[old_clustno++] = i;
          ff[i] = true;
          break;
        }
      }
    }
  } else if (size < clustall * ClustnoBytes) {
    int i;
    for (int size1 = clustall * ClustnoBytes, i = clustno + clustall;
         size1 > size; size1 -= ClustnoBytes, i--) {
      fat[i] = 0;
      ff[i] = false;
    }
    old_clustno = i;
  }
  fat[old_clustno] = 0xfff;
  ff[old_clustno] = true;
  char *img = ADR_DISKIMG + (FileDataAddress - ClustnoBytes * 2);
  for (;;) {
    if (size <= ClustnoBytes) {
      for (int i = 0; i < size; i++) {
        img[clustno * ClustnoBytes + i] = buf[i];
      }
      break;
    }
    for (int i = 0; i < ClustnoBytes; i++) {
      img[clustno * ClustnoBytes + i] = buf[i];
    }
    size -= ClustnoBytes;
    buf += ClustnoBytes;
    clustno = fat[clustno];
  }
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
      return finfo + i; /*找到文件*/
    }
  next:
    i++;
  }
  return 0; /*没有找到*/
}
struct FILEINFO *Get_File_Address(char *path1) {
  int bmpDict = dictaddr;
  char *path2 = (char *)malloc(strlen(path1) + 1);
  char *addr = path2;
  strcpy(path2, path1);
  strtoupper(path2);
  if (strncmp("A:\\", path2, 3) == 0 || strncmp("A:/", path2, 3) == 0) {
    path2 += 3;
    bmpDict = RootDictAddress;
  }
  if (path2[0] == '\\' || path2[0] == '/') {
    //跳过反斜杠和正斜杠
    for (int i = 0; i < strlen(path2); i++) {
      if (path2[i] != '\\' && path2[i] != '/') {
        path2 += i;
        break;
      }
    }
  }
  char *temp_name = (char *)malloc(128);
  clean(temp_name, 128);
  struct FILEINFO *finfo;
  int i = 0;
  while (1) {
    for (int j = 0; i < strlen(path2); i++, j++) {
      // printf("path2[%d] = %c\n", i, path2[i]);
      if (path2[i] == '\\' || path2[i] == '/') {
        temp_name[j] = '\0';
        i++;
        break;
      }
      temp_name[j] = path2[i];
    }
    // printf("Got It:%s,ALL:%s\n", temp_name, path2);
    finfo = dict_search(temp_name, (struct FILEINFO *)(ADR_DISKIMG + bmpDict),
                        RootMaxFiles);
    if (finfo == 0) {
      if (path2[i] != '\0') {
        // printf("last file:%s\n", temp_name);
        return 0;
      }
      finfo = file_search(temp_name, (struct FILEINFO *)(ADR_DISKIMG + bmpDict),
                          RootMaxFiles);
      if (finfo == 0) {
        // printf("Invalid file:%s\n", temp_name);
        return 0;
      } else {
        goto END;
      }
    } else {
      // printf("dict_search:%s\n", temp_name);
      if (finfo->clustno != 0) {
        bmpDict = (finfo->clustno * 512 + (FileDataAddress - 2 * 512));
        // printf("dictaddr = %08x, i = %d\n", bmpDict, i);
      } else {
        // print("finfo->clustno == 0\n");
        bmpDict = RootDictAddress;
      }
      clean(temp_name, 128);
    }
  }
END:
  free(temp_name);
  free(addr);
  return finfo;
}
struct FILEINFO *clust_sech(int clustno, struct FILEINFO *finfo, int max) {
  int i, j;
  j = 0;
  for (i = 0; i < max; i++) {
    if (finfo[i].clustno == clustno) {
      return finfo + i;
    }
  }
  return 0; /*没有找到*/
}
void strtoupper(char *str) {
  while (*str != '\0') {
    if (*str >= 'a' && *str <= 'z') {
      *str -= 32;
    }
    str++;
  }
}
void changedict(char *dictname) {
  struct FILEINFO *finfo = dict_search(
      dictname, (struct FILEINFO *)(ADR_DISKIMG + dictaddr), RootMaxFiles);
  if (finfo == (struct FILEINFO *)(0)) {
    printf("Invalid directory.\n");
    return;
  }
  if (finfo->clustno == 0) {
    strcpy((char *)&View_path[0], "");
    change_dict_times = 0;
    dictaddr = RootDictAddress; // 锟斤拷目录
    return;
  }
  if (strcmp(dictname, "..") != 0 && strcmp(dictname, ".") != 0) {
    if (change_dict_times == 0) {
      strcat((char *)&View_path[0], dictname);
    } else {
      strcat((char *)&View_path[0], "\\");
      strcat((char *)&View_path[0], dictname);
    }
  }

  if (strcmp(dictname, "..") == 0) {
    int i;
    // print("OK\n");
    for (i = strlen((char *)&View_path[0]) - 1; i >= 0; i--) {
      if (View_path[i] == '\\') {
        View_path[i] = '\0';
        break;
      }
    }
    change_dict_times -= 2; //锟饺会还要++锟斤拷锟斤拷-=2
  }
  dictaddr = (finfo->clustno * 512 + (FileDataAddress - 2 * 512));
  change_dict_times++;
  return;
}
int Get_dictaddr(char *path1) {
  int bmpDict = dictaddr;
  char *path2 = (char *)malloc(strlen(path1) + 1);
  char *addr = path2;
  strcpy(path2, path1);
  strtoupper(path2);
  if (strncmp("A:\\", path2, 3) == 0 || strncmp("A:/", path2, 3) == 0) {
    path2 += 3;
    bmpDict = RootDictAddress;
  }
  if (path2[0] == '\\' || path2[0] == '/') {
    //跳过反斜杠和正斜杠
    for (int i = 0; i < strlen(path2); i++) {
      if (path2[i] != '\\' && path2[i] != '/') {
        path2 += i;
        break;
      }
    }
  }
  char *temp_name = (char *)malloc(128);
  clean(temp_name, 128);
  struct FILEINFO *finfo;
  int i = 0;
  while (1) {
    int j;
    for (j = 0; i < strlen(path2); i++, j++) {
      if (path2[i] == '\\' || path2[i] == '/') {
        temp_name[j] = '\0';
        // printf("Got It:%s,ALL:%s\n", temp_name, path);
        i++;
        break;
      }
      temp_name[j] = path2[i];
    }
    finfo = dict_search(temp_name, (struct FILEINFO *)(ADR_DISKIMG + bmpDict),
                        RootMaxFiles);
    if (finfo == 0) {
      free(temp_name);
      free(addr);
      return bmpDict;
    } else {
      // printf("dict_search:%s\n", temp_name);
      if (finfo->clustno != 0) {
        bmpDict = (finfo->clustno * 512 + (FileDataAddress - 2 * 512));
      } else {
        bmpDict = RootDictAddress;
      }
      clean(temp_name, 128);
      if (path2[i] == '\0') {
        goto END;
      }
    }
  }
END:
  free(temp_name);
  free(addr);
  return bmpDict;
}
// ../kernel/img/Powerint_DOS_386.img
void mkdir(char *dictname, int last_clust) {
  /*
          dictname:目录名
          last_clust:上一级目录的簇号
  */
  mkfile(dictname);
  struct MYFILE *fp = myfopen(dictname, "wb");
  // 三个目录项（模板）
  struct FILEINFO *finfo = Get_File_Address(dictname);
  struct FILEINFO dictmodel1; // .目录项，指向自己
  struct FILEINFO dictmodel2; // ..目录项，指向上一级目录
  struct FILEINFO null;       //空目录项（为mkfile函数提供指引）
  memcpy(null.name, "NULL       ", 11);
  null.type = 0x20; // 文件的type属性是0x20
  //将size date time这类属性全部设置为0
  null.size = 0;
  time_t timep;
  struct tm *p;
  time(&timep);
  p = gmtime(&timep);
  null.date = get_fat12_date(p->tm_year, p->tm_mon, p->tm_mday);
  null.time = get_fat12_time(p->tm_hour, p->tm_min);
  for (int i = 0; i != 3072; i++) {
    if (!fat[i]) {
      null.clustno = i;
      fat[i] = 0xfff;
      FatClustnoFlags[i] = true;
      break;
    }
  }
  dictmodel1.name[0] = '.';
  for (int i = 1; i != 8; i++)
    dictmodel1.name[i] = ' ';
  for (int i = 0; i != 3; i++)
    dictmodel1.ext[i] = ' ';
  dictmodel1.type = 0x10;
  dictmodel1.clustno = finfo->clustno; // 指向自己
  dictmodel1.size = 0;
  dictmodel1.date = null.date;
  dictmodel1.time = null.time;
  dictmodel2.name[0] = '.';
  dictmodel2.name[1] = '.';
  for (int i = 2; i != 8; i++)
    dictmodel2.name[i] = ' ';
  for (int i = 0; i != 3; i++)
    dictmodel2.ext[i] = ' ';
  dictmodel2.clustno = last_clust;
  dictmodel2.size = 0;
  dictmodel2.date = null.date;
  dictmodel2.time = null.time;
  dictmodel2.type = 0x10;
  unsigned char *ptr = (unsigned char *)&dictmodel1;
  myfseek(fp, 0, 0);
  for (int i = 0; i != 32; i++) {
    myfputc(ptr[i], fp);
  }
  ptr = (unsigned char *)&dictmodel2;
  myfseek(fp, 32, 0);
  for (int i = 0; i != 32; i++) {
    myfputc(ptr[i], fp);
  }
  ptr = (unsigned char *)&null;
  myfseek(fp, 64, 0);
  for (int i = 0; i != 32; i++) {
    myfputc(ptr[i], fp);
  }
  myfclose(fp);
  finfo->type = 0x10; // 是目录（文件夹的type属性是0x10）
  finfo->size = 0;
  return;
}
void Copy(char *path, char *path1) {
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
  mkfile(path1); //创建文件
  struct FILEINFO *finfo1 = Get_File_Address(path1);
  struct MYFILE *fp1 = myfopen(path1, "wb");
  // 拷贝文件内容
  for (int i = 0; i != size; i++) {
    myfputc(ptr[i], fp1);
  }
  myfclose(fp1);
  // 拷贝文件属性
  time_t timep;
  struct tm *p;
  time(&timep);
  p = gmtime(&timep);
  finfo1->date = get_fat12_date(p->tm_year, p->tm_mon, p->tm_mday);
  finfo1->time = get_fat12_time(p->tm_hour, p->tm_min);
  return;
}
void del(char *cmdline) {
  //删除某个文件
  char *name;
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
  struct MYFILE *fp = myfopen(name, "wb");
  for (i = 0; i != fp->size; i++) {
    myfputc(0, fp);
  }
  fp->size = 0;
  myfclose(fp);
  finfo->name[0] = 0xe5;
  fat[finfo->clustno] = 0;
  FatClustnoFlags[finfo->clustno] = false;
  /*i = 0;
  while (fat[finfo->clustno + i] != 0xfff) {
    fat[finfo->clustno + i] = 0;
    FatClustnoFlags[finfo->clustno + i] = false;
    i++;
  }
  fat[finfo->clustno + i] = 0;
  FatClustnoFlags[finfo->clustno + i] = false;*/
  return;
}
void mkfile(char *name) {
  char s[12];
  int i, j;
  int dict = Get_dictaddr(name);
  struct FILEINFO *finfo =
      (struct FILEINFO *)(ADR_DISKIMG + Get_dictaddr(name));

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
    if (!fat[i]) {
      finfo->clustno = i;
      fat[i] = 0xfff;
      // printf("mkfile:%s dict:%08x fat[%d] = 0xfff\n", name, dict, i);
      FatClustnoFlags[i] = true;
      break;
    }
  }
  for (i = 0; i != 10; i++) {
    finfo->reserve[i] = 0;
  }
  time_t timep;
  struct tm *p;
  time(&timep);
  p = gmtime(&timep);
  finfo->time = get_fat12_time(p->tm_hour, p->tm_min);
  finfo->date = get_fat12_date(p->tm_year, p->tm_mon, p->tm_mday);
  finfo->size = 0;
  return;
}
void cmd_dir() {
  struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + dictaddr);
  int i, j, k, line = 0;
  char s[30];
  for (i = 0; i != 30; i++) {
    s[i] = 0;
  }
  printf("FILENAME   EXT    LENGTH       TYPE   DATE\n");
  for (i = 0; i < RootMaxFiles; i++, line++) {
    if (finfo[i].name[0] == 0x00) {
      break;
    }
    if (finfo[i].name[0] != 0xe5) {
      if ((finfo[i].type & 0x18) == 0 || finfo[i].type == 0x10) {

        for (j = 0; j < 8; j++) {
          s[j] = finfo[i].name[j];
        }
        s[9] = finfo[i].ext[0];
        s[10] = finfo[i].ext[1];
        s[11] = finfo[i].ext[2];

        if (s[0] != '+') {
          for (k = 0; k < 12; ++k) {
            if (k == 9) {
              printf("   ");
            }
            if (s[k] == '\n') {
              printf("   ");
            } else {

              printf("%c", s[k]);
            }
          }
          printf("    ");
          printf("%d", finfo[i].size);
          // gotoxy(31, get_y());
          if ((finfo[i].type & 0x18) == 0) {
            printf("<FILE> ");
          }
          if (finfo[i].type == 0x10) {
            printf("<DIR>  ");
          }
          print_date(finfo[i].date, finfo[i].time);
          printf("\n");
        }
      }
    }
  }
  printf("\n");
  //&s = 0;
  return;
}
void saveImage(void) {
  fstream out;
  out.open(path, ios::out | ios::binary);
  out << ___file;
}
void shell() {
  while (1) {
    printf("%s:>", (char *)&View_path[0]);
    string _cmd = "";
    getline(cin, _cmd);
    command_run(_cmd);
    printf("\n");
  }
}
void command_run(string commands) {
  if (commands == "cls") {
    system("cls");
  } else if (commands == "dir") {
    cmd_dir();
  } else if (strncmp(commands.c_str(), "cd ", 3) == 0) {

    char *ptr = (char *)commands.c_str();
    changedict(ptr + 3);
  } else if (strncmp(commands.c_str(), "mkfile ", 7) == 0) {
    char *ptr = (char *)commands.c_str();
    mkfile(ptr + 7);

  } else if (strncmp(commands.c_str(), "mkdir ", 6) == 0) {
    char *ptr = (char *)commands.c_str();
    if (change_dict_times == 0) {
      mkdir(ptr + 6, 0);
    } else {
      struct FILEINFO *finfo = dict_search(
          ".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), RootMaxFiles);
      mkdir(ptr + 6, finfo->clustno);
    }

  } else if (strncmp(commands.c_str(), "copy ", 5) == 0) {
    char *ptr = (char *)commands.c_str();
    char path_1[100];
    char path_2[100];
    ptr += 5;
    int i;
    for (i = 0; *ptr != ' '; ptr++, i++)
      path_1[i] = *ptr;
    path_1[i] = 0;
    ptr++;
    for (i = 0; *ptr != 0; ptr++, i++)
      path_2[i] = *ptr;
    path_2[i] = 0;

    Copy(path_1, path_2);

  } else if (commands == "save") {
    write_fat((unsigned char *)(ADR_DISKIMG + Fat1Address), fat);
    write_fat((unsigned char *)(ADR_DISKIMG + Fat2Address), fat);
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
        fat = (int *)malloc(3072 * sizeof(int));
        FatClustnoFlags =
            (unsigned char *)malloc(3072 * sizeof(char));
        read_fat((unsigned char *)(ADR_DISKIMG + Fat1Address), fat,
                 FatClustnoFlags);
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
          sprintf(buf, "copy %s %s", argv[++i], argv[++i]);
          command_run(buf);
        } else {
          return 1;
        }
      } else {
        cout << "wrong arg--->" << argv[i] << endl;
      }
    }
  }
  if (Path_Ok) {
    write_fat((unsigned char *)(ADR_DISKIMG + Fat1Address), fat);
    write_fat((unsigned char *)(ADR_DISKIMG + Fat2Address), fat);
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
  fat = (int *)malloc(3072 * sizeof(int));
  FatClustnoFlags =
      (unsigned char *)malloc(3072 * sizeof(char));
  read_fat((unsigned char *)(ADR_DISKIMG + Fat1Address), fat, FatClustnoFlags);
  shell();

  return 0;
}

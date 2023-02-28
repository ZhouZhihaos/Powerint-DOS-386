#include <dosldr.h>
FILE *fopen(char *path, char *mode) {
  struct FILEINFO *finfo;
  finfo = Get_File_Address(path);
  if (finfo == 0) {
    return 0;
  } else {
    FILE *fp = (FILE *)page_malloc(sizeof(FILE));
    int drive_number;
    if (strncmp(path + 1, ":\\", 2) == 0 || strncmp(path + 1, ":/", 2) == 0) {
      drive_number = *path - 0x41;
    } else {
      drive_number = NowTask()->drive_number;
    }
    if (finfo->size != 0) {
      fp->buf = (char *)page_kmalloc(finfo->size);
      fp->realloc = ((finfo->size - 1) / 4096 + 1) * 4096;
    } else {
      fp->buf = (char *)page_malloc_one();
      fp->realloc = 4096;
    }
    file_loadfile(finfo->clustno, finfo->size, fp->buf,
                  drive_ctl.drives[drive_number].fat, drive_number);
    fp->size = finfo->size;
    fp->p = 0; //指向文件的开头
    fp->path = (char *)malloc(strlen(path));
    strcpy(fp->path, path);
    return fp;
  }
}
// fputc
int fputc(int c, FILE *fp) {
  if (fp->p >= fp->realloc) {
    void *p = page_kmalloc(fp->realloc + 4096);
    memcpy(p, (void *)fp->buf, fp->realloc);
    page_kfree((void *)fp->buf, fp->realloc);
    fp->buf = (char *)p;
    fp->realloc += 4096;
  }
  if (fp->p >= fp->size) {
    fp->size = fp->p + 1;
  }
  fp->buf[fp->p++] = c;
  return 0;
}
// fgetc
int fgetc(FILE *fp) {
  if (fp->p >= fp->size) {
    return EOF;
  } else {
    return fp->buf[fp->p++];
  }
}
// fgets
char *fgets(char *s, int size, FILE *fp) {
  int i;
  for (i = 0; i < size - 1; i++) {
    int c = fgetc(fp);
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
// fseek
int fseek(FILE *fp, int offset, int whence) {
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
int fclose(FILE *fp) {
  struct TASK *task = NowTask();
  struct FILEINFO *finfo = Get_File_Address(fp->path);
  int drive_number;
  if (strncmp(fp->path + 1, ":\\", 2) == 0 ||
      strncmp(fp->path + 1, ":/", 2) == 0) {
    drive_number = *fp->path - 0x41;
  } else {
    drive_number = task->drive_number;
  }
  extern FILE *f_t;
  f_t = fp;
  file_savefile(finfo->clustno, fp->size, fp->buf,
                drive_ctl.drives[drive_number].fat,
                drive_ctl.drives[drive_number].FatClustnoFlags, drive_number);
  struct FILEINFO *Bmp = task->directory;
  int Bmp2 = task->change_dict_times;
  task->directory = Get_dictaddr(fp->path);
  char *BMP_PATH =
      (char *)malloc(drive_ctl.drives[drive_number].RootMaxFiles * 13 + 1);
  for (int i = 0; i < drive_ctl.drives[drive_number].RootMaxFiles * 13; i++) {
    BMP_PATH[i] = 0;
  }
  strcpy(BMP_PATH, task->path);
  while (task->directory != drive_ctl.drives[drive_number].root_directory) {
    if (dict_search(".", task->directory,
                    drive_ctl.drives[drive_number].RootMaxFiles) != 0) {
      struct FILEINFO *finfo_this_dict_clust = dict_search(
          ".", task->directory, drive_ctl.drives[drive_number].RootMaxFiles);
      struct FILEINFO *finfo_this_dict =
          clust_sech(finfo_this_dict_clust->clustno, Get_dictaddr("../"),
                     drive_ctl.drives[drive_number].RootMaxFiles);
      finfo_this_dict->size += (fp->size - finfo->size);
      file_saveinfo(Get_dictaddr("../"), drive_number);
    }
    int tmp = task->drive_number;
    task->drive_number = drive_number;
    changedict("..");
    task->drive_number = tmp;
  }
  strcpy(task->path, BMP_PATH);
  free(BMP_PATH);
  task->directory = Bmp;
  task->change_dict_times = Bmp2;
  finfo->size = fp->size; // 大小更改
  file_saveinfo(Get_dictaddr(fp->path), drive_number);
  free((void *)fp->path);
  page_kfree((void *)fp->buf, fp->realloc);
  page_free((void *)fp, sizeof(FILE));
  return 0;
}
// fread
int fread(void *buf, int size, int count, FILE *fp) {
  int i;
  for (i = 0; i < size * count; i++) {
    if (fp->p >= fp->size) {
      break;
    }
    ((unsigned char *)buf)[i] = fp->buf[fp->p++];
  }
  return i;
}
void EDIT_FILE(char *name, char *dest, int length, int offset) {
  FILE *fp = fopen(name, "wb");
  if (fp == 0) {
    //没有找到文件，创建一个，然后再编辑
    mkfile(name);
    EDIT_FILE(name, dest, length, offset);
    return;
  }
  fseek(fp, offset, 0);
  for (int i = 0; i != length; i++) {
    fputc(dest[i], fp);
  }
  fclose(fp);
  return;
}
void longName28dot3(char *result, char *src_in) {
  char *src = (char *)page_malloc(strlen(src_in) + 1);
  strcpy(src, src_in);
  strtoupper(src);
  char result1[8 + 1] = {0};
  int nameSize = 0;
  for (int i = 0; i < strlen(src); i++) {
    if (src[i] == '.') {
      break;
    }
    nameSize++;
  }
  int extSize = strlen(src) - nameSize + 1;
  strncpy(result1, src, 6);
  char ext[4] = {0};
  strncpy(ext, src + nameSize + 1, 3);
  for (int i = 1; i < 10; i++) {
    sprintf(result, "%s~%d.%s\0", result1, i, ext);
    if (fsz(result) == -1) {
      page_free((int)src, strlen(src) + 1);
      return;
    }
  }
  page_free((int)src, strlen(src) + 1);
}
int fsz(char *filename) {
  struct FILEINFO *finfo = Get_File_Address(filename);
  if (finfo == 0) {
    return -1;
  } else {
    return finfo->size;
  }
}
// ftell
int ftell(FILE *fp) { return fp->p; }
// rewind
void rewind(FILE *fp) { fp->p = 0; }

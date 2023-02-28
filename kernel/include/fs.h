#ifndef _FS_H
#define _FS_H
#include <define.h>
// fat12.c
void read_fat(unsigned char *img, int *fat, unsigned char *ff);
void write_fat(unsigned char *img, int *fat);
void file_loadfile(int clustno, int size, char *buf, int *fat, int drive_number);
void file_savefile(int clustno, int size, char *buf, int *fat, unsigned char *ff, int drive_number);
void file_saveinfo(struct FILEINFO *directorys, int drive_number);
void file_savefat(int *fat, int drive_number);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);
struct FILEINFO *dict_search(char *name, struct FILEINFO *finfo, int max);
struct FILEINFO *Get_File_Address(char *path1);
struct FILEINFO *Get_dictaddr(char *path1);
void mkdir(char *dictname, int last_clust);
void Copy(char *path, char *path1);
struct FILEINFO *clust_sech(int clustno, struct FILEINFO *finfo, int max);
void del(char *cmdline);
void mkfile(char *name);
void changedict(char *dictname);
void rename(char *src_name, char *dst_name);
// file.c
FILE *fopen(char *path, char *mode);
int fputc(int c, FILE *fp);
int fgetc(FILE *fp);
char *fgets(char *s, int size, FILE *fp);
int fseek(FILE *fp, int offset, int whence);
int fclose(FILE *fp);
int fread(void *buf, int size, int count, FILE *fp);
void EDIT_FILE(char *name, char *dest, int length, int offset);
void longName28dot3(char *result, char *src_in);
int fsz(char *filename);
int ftell(FILE *fp);
void rewind(FILE *fp);
// bmp.c
void bmpview(char *filename);
void BMPVIEW32(char *path, unsigned char *vram, int xsize);
// jpeg.c
int info_JPEG(struct DLL_STRPICENV *env, int *info, int size,
              unsigned char *fp);
int decode0_JPEG(struct DLL_STRPICENV *env, int size, unsigned char *fp0,
                 int b_type, unsigned char *buf, int skip);
void jpgview32(char *path, unsigned char *vram, int xsize);
// pra.c
// Powerint raw image viewer
void MakePraFile(char *PraFileName, uint8_t *imgBuffer, uint32_t xsize,
                 uint32_t ysize);
void PraShell();
void pra_view_32(unsigned char *path, unsigned char *vram, int scr_xsize);
void jpgview32(char *path, unsigned char *vram, int xsize);
// path.c
struct FILEINFO *Path_Find_File(char *fileName, char *PATH_ADDR);
void Path_Find_FileName(char *Result, char *fileName, char *PATH_ADDR);
#endif

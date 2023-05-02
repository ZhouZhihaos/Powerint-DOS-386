#ifndef _FS_H
#define _FS_H
#include <define.h>
// fat12.c
void Register_fat_fileSys();
// file.c
FILE* fopen(char* path, char* mode);
int fputc(int c, FILE* fp);
int fgetc(FILE* fp);
char* fgets(char* s, int size, FILE* fp);
int fseek(FILE* fp, int offset, int whence);
int fclose(FILE* fp);
long ftell(FILE *stream);
unsigned int fread(void* buffer,
                   unsigned int size,
                   unsigned int count,
                   FILE* stream);
unsigned int fwrite(const void *ptr, unsigned int size, unsigned int nmemb,
                    FILE *stream);
int fputs(const char *str, FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
int feof(FILE *stream);
int getc(FILE *stream);
int ferror(FILE *stream);
int fsz(char *filename);
void EDIT_FILE(char* name, char* dest, int length, int offset);
int Copy(char* path, char* path1);
// bmp.c
void bmpview(char* filename);
void BMPVIEW32(char* path, unsigned char* vram, int xsize);
// jpeg.c
int info_JPEG(struct DLL_STRPICENV* env,
              int* info,
              int size,
              unsigned char* fp);
int decode0_JPEG(struct DLL_STRPICENV* env,
                 int size,
                 unsigned char* fp0,
                 int b_type,
                 unsigned char* buf,
                 int skip);
void jpgview32(char* path, unsigned char* vram, int xsize);
// pra.c
// Powerint raw image viewer
void MakePraFile(char* PraFileName,
                 uint8_t* imgBuffer,
                 uint32_t xsize,
                 uint32_t ysize);
void PraShell();
void pra_view_32(unsigned char* path, unsigned char* vram, int scr_xsize);
void jpgview32(char* path, unsigned char* vram, int xsize);
// path.c
bool Path_Find_File(char* fileName, char* PATH_ADDR);
void Path_Find_FileName(char* Result, char* fileName, char* PATH_ADDR);
// vfs.c
List* vfs_listfile(char* dictpath);
bool vfs_readfile(char* path, char* buffer);
bool vfs_writefile(char* path, char* buffer, int size);
uint32_t vfs_filesize(char* filename);
bool vfs_register_fs(vfs_t vfs);
bool vfs_renamefile(char* filename, char* filename_of_new);
bool vfs_change_path(char* dictName);
bool vfs_deldir(char* dictname);
bool vfs_delfile(char* filename);
bool vfs_createfile(char* filename);
bool vfs_createdict(char* filename);
void vfs_getPath(char* buffer);
bool vfs_change_disk(uint8_t drive);
bool vfs_mount_disk(uint8_t disk_number, uint8_t drive);
uint32_t vfs_filesize(char* filename);
bool vfs_readfile(char* path, char* buffer);
bool vfs_change_disk_for_task(uint8_t drive, struct TASK* task);
bool vfs_format(uint8_t disk_number, char* FSName);
bool vfs_check_mount(uint8_t drive);
bool vfs_unmount_disk(uint8_t drive);
bool vfs_attrib(char* filename, ftype type);
vfs_file *vfs_fileinfo(char *filename);
#endif

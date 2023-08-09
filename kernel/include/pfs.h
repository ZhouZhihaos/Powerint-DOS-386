#ifndef __PFS_H__
#define __PFS_H__
#include <dos.h>
#include <ctypes.h>

#pragma pack(1)
typedef struct {
  uint8_t jmp[2];
  uint8_t sign[4];             // "PFS\xff"
  uint32_t sec_bitmap_start;   // 位图起始扇区号
  uint32_t resd_sector_start;  // 都为0就没有保留
  uint32_t resd_sector_end;
  uint32_t first_sector_of_bitmap;
  uint32_t root_dict_block;
  char volid[16];  // "POWERINTDOS\x00"
} pfs_mbr;
typedef struct {
  uint8_t r : 1;
  uint8_t w : 1;
  uint8_t e : 1;
  uint8_t resd : 5;
} pfs_attr;
typedef struct {
  char
      type;  // 0：未使用 1:文件 2.文件夹 3.长文件名（长文件名使用另外一个结构）
  char name[14];  // 第十四个字节如果是0xff
                  // 那么说明没读完，要继续往下读（通过next）
  uint32_t
      dat;  // 如果是文件，那么就是data_block的编号，如果是文件夹，那么就是该文件夹的目录区的编号
  uint32_t time;  // unix时间戳
  uint32_t size;
  pfs_attr attr;  // 权限
  uint32_t
      next;  // 下一个与此有关的inode在该目录区的第几项（文件名），诺此处为0，则表示没有，继续按顺序往下读
} pfs_inode;  // 32字节
typedef struct {
  char type;      // it is always 3
  char name[27];  // 如果第26个字节是0xff 那么就继续往下读
  uint32_t next;  // 0 or other
} pfs_inode_of_long_file_name;
typedef struct {
  uint8_t data[508];
  uint32_t next;
} pfs_data_block;  // 512b(one sec)
typedef struct {
  pfs_inode inodes[15];  // 480b
  uint32_t next;
  char resd[28];
} pfs_dict_block;  // 512b
#pragma pack()
typedef struct pfs pfs_t;
typedef struct pfs {
  List* file_list;
  List* bitmap;
  List* bitmap_buffer;
  uint32_t resd_sec_start;
  uint32_t resd_sec_end;
  uint32_t first_sec_of_bitmap;
  uint32_t sec_bitmap_start;
  uint32_t root_dict_block;
  void (*read_block)(pfs_t* pfs, uint32_t lba, uint32_t numbers, void* buff);
  void (*write_block)(pfs_t* pfs, uint32_t lba, uint32_t numbers, void* buff);
  uint8_t disk_number;
  uint32_t current_dict_block;
  int64_t current_bitmap_block;
  uint8_t *bitmap_buff;
  List * prev_dict_block;
} pfs_t;
typedef struct {
  char* name;
  uint32_t size;
  uint32_t block;
  uint32_t time;
  pfs_attr attr;
} pfs_file_list;
#define total_bits_of_one_sec ((512 - 4) * 8)
#define used(bitmap, index) bitmap[index / 8] |= (1 << (index % 8))
#define unused(bitmap, index) bitmap[index / 8] &= ~(1 << (index % 8))
#define bit_get(bitmap, index) (bitmap[index / 8] & (1 << (index % 8)))
#define set_next(bitmap, next) *((uint32_t*)((uintptr_t)bitmap + 508)) = (next)
#define get_next(bitmap) (*((uint32_t*)((uintptr_t)bitmap + 508)))
#define block2sector(block, _pfs) ((block) + ((_pfs)->first_sec_of_bitmap))
uint32_t pfs_create_inode(vfs_t *vfs,uint32_t dict_block);
// void pfs_format(pfs_t p, char* volid);
// void init_pfs(pfs_t p);
// uint32_t pfs_create_inode(uint32_t dict_block);
// void pfs_ls(uint32_t dict_block);
// uint32_t pfs_get_filesize(char* filename, uint32_t dict_block, uint32_t* err);
// void pfs_read_file(char* filename, void* buff, uint32_t dict_block);
// void pfs_create_file(char* filename, uint32_t dict_block);
// void pfs_create_dict(char* name, uint32_t dict_block);
// uint32_t pfs_get_dict_block_by_name(char* name,
//                                     uint32_t dict_block,
//                                     uint32_t* err);
// uint32_t pfs_get_idx_of_inode_by_name(char* name,
//                                       uint32_t dict_block,
//                                       uint32_t* err);
// void pfs_write_file(char* filename,
//                     uint32_t size,
//                     void* buff,
//                     uint32_t dict_block);
// uint32_t pfs_get_dict_block_by_path(char* path,
//                                     char** end,
//                                     uint32_t start_block,
//                                     uint32_t* err);
// void pfs_get_file_index_by_path(char* path,
//                                 uint32_t start_block,
//                                 uint32_t* err,
//                                 uint32_t* idx,
//                                 uint32_t* dict_block);
#endif
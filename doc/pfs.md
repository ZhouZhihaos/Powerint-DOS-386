# PFS --- 仅用以学习实践而设计的文件系统
**Attention**：
1. 在此文档中，目录项也被写作`inode`
2. 在此文档中，`block索引`，如果不做强调，指的不是在磁盘扇区的位置，而是`bitmap`的位置，诺令某个block索引为n，则这个block索引在磁盘中的扇区位置是`n+bitmap的第一个扇区索引所指向的扇区位置`
## 特点
- 慢
- 慢
- 慢
## 结构
### mbr
|偏移（hex)| 名称 | 描述 | 长度 |
|---|---|---|---|
| 0x00 | jmp | 引导扇区开头保留两个字节的跳转指令 | 2 |
| 0x02 | sign | PFS标识 "PFS\xff" | 4 |
| 0x06 | sec_bit_map_start| 存储block位图的扇区从哪里开始 | 4 |
| 0x0A | resd_sector_start| 保留扇区的开始 | 4 |
| 0x0E | resd_sector_end | 保留扇区的结束| 4 |
| 0x12| first_sector_of_bitmap | bitmap的第一个扇区索引所指向的扇区位置，后面就按顺序以此类推 | 4 |
|0x16| root_dict_block | 根目录的block索引（位图索引），始终填为0 | 4 |
|0x1A| volid | 磁盘ID | 16 |

如果使用C语言的结构体表达，那么就是下面这样的
```c
// 注意 前面要加上 #pragma pack(1)
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
```
----
通过上面的`mbr`，我们就可以获取一些重要信息，而`sec_bitmap_start`则就是以保留扇区（`sec_bitmap_start` = `resd_sector_end`）计算的  
那么， **如何计算**？我们用C语言代码表示  
**注意**：`resd_sector_start`和`resd_sector_end`如果都是0，那么说明没有保留扇区，如果没有保留扇区，那么就要令`sec_bitmap_start` = 1
```c
pfs_mbr p;
/*set resd_sector_start and resd_sector_end*/
// ......
if(resd_sector_start == 0 && resd_sector_end == 0) {
  p.sec_bitmap_start = 1;
} else {
  p.sec_bitmap_start = resd_sector_end;
}
p.first_sector_of_bitmap = p.sec_bitmap_start + 1; //在位图后面一个扇区
p.root_dict_block = 0;
```
而`root_dict_block`我们只需要填写为0即可，原因我们将在后面的章节描述
### 目录
PFS的目录在刚开始，只有一个扇区，每个扇区最多只能有15个目录项，诺超出了15，就可以以类似于链表的形式继续往后查找。  
那么，目录项的结构是什么样的呢
| 偏移（hex） | 名称 | 描述 | 长度 |
|---| --- | --- | ----|
| 0x00 | type | 这个inode项的type，type的值有：0：未使用 1:文件 2.文件夹 3.长文件名（长文件名使用另外一个结构） | 1 |
| 0x01| name | 文件名，如果第十四个字节是0x0，说明已经结束，如果第十四个字节是0xff，说明还没结束，需要使用next项继续往下读 | 14 |
| 0x0f| dat | 如果type显示是文件，则这个dat是data_block（另外一个数据结构）的索引，也就是通过这个索引读取出来的数据是data_block，如果是文件夹，则是dict_block(**目录的数据结构，和根目录的数据结构相同，所有目录都是使用此数据结构**) | 4 |
| 0x13 | time | 文件创建时的unix时间戳 | 4 |
| 0x17 | size | 文件大小 | 4|
| 0x1b | attr | 文件权限 | 1|
| 0x1c | next | 如果还有文件名，这里就会被填写，否则填0| 4

用C语言，则像这样表示
```c
// 注意 前面要加上 #pragma pack(1)
typedef struct {
  char type;  // 0：未使用 1:文件 2.文件夹 3.长文件名（长文件名使用另外一个结构）
  char name[14];  // 第十四个字节如果是0xff
                  // 那么说明没读完，要继续往下读（通过next）
  uint32_t dat;  // 如果是文件，那么就是data_block的编号，如果是文件夹，那么就是该文件夹的目录区的编号
  uint32_t time;  // unix时间戳
  uint32_t size;
  pfs_attr attr;  // 权限
  uint32_t next;  // 下一个与此有关的inode在该目录区的第几项（文件名），诺此处为0，则表示没有，继续按顺序往下读
} pfs_inode;  // 32字节
```
而当文件名超出了13字节的限制，就会启动另外一个结构，这两个结构的占用大小相同，换言之，即这个结构，也会占用一个目录项用于存储文件名。  
而这个结构，是这样的
| 偏移（hex） | 名称 | 描述 | 长度 |
|---| --- | --- | ----|
|0x00|type|该项的类型（在这个结构体中，**正常情况**下，永远为3） | 1 |
|0x01|name|文件名（续上之前没读完的文件名），一个文件名完完整整的结束后，最后一个字节后面的那个字节必须是0，因此这个结构虽然使用了27字节，但只能存储26字节的文件名，最后一个字节也需要进行设置，如果一个文件名在此时已经被完完整整的描述完成，那么就设置第27个字节为0x00（如果没有完整填完26个字节，这里也需要设置，而且还要将填写完成的文件名的最后一个字节的位置的后面一个字节设置成0，但是如果正好填完了就不需要这样，只需要设置第27个字节为0），否则填写为0xff，并且分配一个新的目录项，填写至next，然后到新的目录项继续存储文件名，直至结束。| 27 |
|0x1c|next|如果这个文件名结束了，这里填写0，否则是接下来的文件名的目录项索引|4|


在C语言中表示，就像下面这样
```c
// 注意 前面要加上 #pragma pack(1)
typedef struct {
  char type;      // it is always 3
  char name[27];  // 如果第26个字节是0xff 那么就继续往下读
  uint32_t next;  // 0 or other
} pfs_inode_of_long_file_name;
```
或许有人会问：
> 那每个目录项的权限怎么办？  

实现上并没有实现（~~lazy~~），但是结构上给出了权限这个项的预留，这个项的结构，如下表所示
| Bit | 名称 | 描述 | 长度 |
| --- | --- | --- | ---|
|   0   | r | 此文件是否可读| 1|
| 1 | w | 此文件是否可写 | 1 |
| 2| e | 此文件是否可执行| 1 |
|3| resd| 保留位 | 5 |

换作C语言表达，则是如此
```c
// 请在前面加上#pragma pack(1)
typedef struct {
  uint8_t r : 1; // 可读
  uint8_t w : 1; // 可写
  uint8_t e : 1; // 可执行
  uint8_t resd : 5; // 保留，或可用于其他（例如用户组等）
} pfs_attr;
```
那么一个目录中，这么多`目录项`（inodes）该如何存储呢？  
上面我提到，每个扇区只能存储15个inode，所以这个扇区的结构是这样的  
| 偏移（hex） | 名称 | 描述 | 长度 |
|---| --- | --- | ----|
| 0x000 | inodes| 在这个扇区中存储的所有的目录项（以上述结构存储），共十五项| 480 |
| 0x1E0| next | 此处为0说明这个目录项已经结束，如果不为0，说明仍旧还有，需要通过next给出的block索引继续查找，这是一个类似`链表`的结构| 4 |
| 0x1E4 | resd | 保留项 | 28 |

写为C语言就是这样
```c
// 注意，请在前面加上#pragma pack(1)
typedef struct {
  pfs_inode inodes[15];  // 480b
  uint32_t next;
  char resd[28];
} pfs_dict_block;  // 512b
```
### 文件的存储方式
PFS的文件的数据是以数据块的形式存储的，通常来讲，我们会通过位图分配**n**个位置给文件，每个位置的结构如下图所示
| 偏移（hex） | 名称 | 描述 | 长度 |
|---| --- | --- | ----|
| 0x000 | data | 该文件的数据内容 | 508 |
| 0x1FC | next | 如果此处为0，说明文件已经结束，不需要再往下读了，否则则是下一个数据块（续）的block索引 | 4 |

写成C语言就是像下面这样
```c
// 注意，请在前面使用#pragma pack(1)
typedef struct {
  uint8_t data[508];
  uint32_t next;
} pfs_data_block;  // 512b(one sec)
```
### 位图
位图块占用一个扇区，前508的字节是`Bitmap`，而最后四个字节指下一个Bitmap的位置，如果当前的Bitmap用完了，就可以申请一张新的Bitmap，并且将最后四个字节（`next`）设置成新的位图块的位置，并且将新的位图块的位置的`next`为0  
操作位图块的C语言宏是这样表示的
```c
#define total_bits_of_one_sec ((512 - 4) * 8) // 一个位图块可以管理多少扇区
#define used(bitmap, index) bitmap[index / 8] |= (1 << (index % 8)) // 设置某个block为已经使用
#define unused(bitmap, index) bitmap[index / 8] &= ~(1 << (index % 8)) // 设置某个block为未使用
#define bit_get(bitmap, index) (bitmap[index / 8] & (1 << (index % 8))) // 获取某个block的状态（使用(true) 未使用(false) ）
#define set_next(bitmap, next) *((uint32_t*)((uintptr_t)bitmap + 508)) = (next) // 设置该位图块的next项
#define get_next(bitmap) (*((uint32_t*)((uintptr_t)bitmap + 508))) // 获取该位图块的next项
```
# 结束
注：pfs**的实现**同并pdos以`GPL 3.0`开源，如需要使用，请遵守开源协议  
PS: 如果使用的是本文中的代码，则不需要遵守GPL3.0协议，如果对此文档有疑问，请发送邮件(mm3644112@gmail.com)，不要使用issue；若是pfs实现的bug，请使用issue，不要发送邮件。  
This documant written by `min0911_`  

----
`Copyright 2022-2023 min0911_ & zhouzhihao, pfs`
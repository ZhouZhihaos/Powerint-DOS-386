#include <dos.h>
#include <drivers.h>
typedef struct {
  int rw;
  int number;
  int lba;
  unsigned char* buffer;
  unsigned char* ok_flag;
} packet_header;
typedef struct {
  uint8_t res[2];
  uint16_t sectors_of_number;
  uint16_t off;
  uint16_t seg;
  uint64_t lba;
} __attribute__((packed)) packet;
void bios_read_hd_sec(unsigned LBA, unsigned char* buffer) {
  packet p;
  p.res[0] = 16;
  p.res[1] = 0;
  p.lba = LBA;
  p.off = 0;
  p.seg = 0x800;  // 0x8000物理地址 （0x800:0）
  p.sectors_of_number = 1;
  memcpy(0x7e00, &p, sizeof(packet));
  send_ipc_message(2, "a", 1, asynchronous);

  while (*(unsigned char*)(0x7e00) != 0xff)
    ;

  // regs16_t r;
  // r.ax = 0x4200;
  // r.ds = 0x7e0;
  // r.si = 0x0;
  // r.dx = 0x0080;
  // INT(0x13, &r);
}
void bios_write_hd_sec(unsigned LBA, unsigned char* buffer) {
  memcpy(0x8000, buffer, 512);
  packet p;
  p.res[0] = 16;
  p.res[1] = 0;
  p.lba = LBA;
  p.off = 0;
  p.seg = 0x800;  // 0x8000物理地址 （0x800:0）
  p.sectors_of_number = 1;
  memcpy(0x7e00, &p, sizeof(packet));
  send_ipc_message(2, "b", 1, asynchronous);
  while (*(unsigned char*)(0x7e00) != 0xff)
    ;
  // regs16_t r;
  // r.ax = 0x4200;
  // r.ds = 0x7e0;
  // r.si = 0x0;
  // r.dx = 0x0080;
  // INT(0x13, &r);
}
void drivers_idehdd_read(unsigned int LBA,
                         unsigned int number,
                         unsigned short* buffer) {
  // unsigned int offset, i;
  // io_out8(0x1f7, 0x0c);
  // io_out16(0x1f2, number);              /**数量*/
  // io_out8(0x1f3, (LBA & 0xff));         /**LBA地址7~0*/
  // io_out8(0x1f4, ((LBA >> 8) & 0xff));  /**LBA地址15~8*/
  // io_out8(0x1f5, ((LBA >> 16) & 0xff)); /**LBA地址23~16*/
  // io_out8(0x1f6,
  //         (((LBA >> 24) & 0xff) | 0xe0)); /**LBA地址27~24 | LBA模式，主硬盘*/
  // io_out8(0x1f7, 0x20);                   /**读扇区*/
  // /**循环从DATA端口读取数据*/
  // for (i = 0; i != number; i++) {
  //   drivers_idehdd_wait();
  //   for (offset = 0; offset < 256; offset++) {
  //     buffer[(i * 256) + offset] = io_in16(0x1f0); /**从DATA端口中读取数据*/
  //   }
  // }
  // /**开放任务切换*/
  // io_sti();
  ide_read_sectors(0x0, number, LBA, 8, buffer);
  return;
}
void drivers_idehdd_wait1(void) {
  for (; (io_in8(0x1f7) & 0x80) == 0x80;)
    ; /**循环等待*/
}
void drivers_idehdd_wait2(void) {
  for (; (io_in8(0x1f7) & 0x08) != 0x08;)
    ; /**循环等待*/
}
void drivers_idehdd_wait(void) {
  /**等待次数计时*/
  // unsigned long n;
  for (; (io_in8(0x1f7) & 0x88) != 0x08;)
    ; /**循环等待*/
}
void drivers_idehdd_write(unsigned int LBA,
                          unsigned int number,
                          unsigned short* buffer) {
  ide_write_sectors(0x0, number, LBA, 8, buffer);
}
struct IDEHardDiskInfomationBlock* drivers_idehdd_info() {
  io_cli();
  unsigned short* p =
      (unsigned short*)page_malloc(256 * sizeof(unsigned short));
  io_out8(0x1f7, 0xec);
  for (int i = 0; i != 256; i++) {
    p[i] = io_in16(0x1f0); /**从DATA端口中读取数据*/
  }
  io_sti();
  return (struct IDEHardDiskInfomationBlock*)p;
}

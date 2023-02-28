
#include <dosldr.h>
void drivers_idehdd_read(unsigned int LBA, unsigned int number,
                         unsigned short *buffer) {
  /**互斥操作，不允许在读取时有其他任务切换*/
  io_cli();
  unsigned int offset, i;
  io_out8(0x1f7, 0x0c);
  io_out16(0x1f2, number);              /**数量*/
  io_out8(0x1f3, (LBA & 0xff));         /**LBA地址7~0*/
  io_out8(0x1f4, ((LBA >> 8) & 0xff));  /**LBA地址15~8*/
  io_out8(0x1f5, ((LBA >> 16) & 0xff)); /**LBA地址23~16*/
  io_out8(0x1f6,
          (((LBA >> 24) & 0xff) | 0xe0)); /**LBA地址27~24 | LBA模式，主硬盘*/
  io_out8(0x1f7, 0x20);                   /**读扇区*/
  /**循环从DATA端口读取数据*/
  for (i = 0; i != number; i++) {
    drivers_idehdd_wait();
    for (offset = 0; offset < 256; offset++) {
      buffer[(i * 256) + offset] = io_in16(0x1f0); /**从DATA端口中读取数据*/
    }
  }
  /**开放任务切换*/
  io_sti();
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
void drivers_idehdd_write(unsigned int LBA, unsigned int number,
                          unsigned short *buffer) {
  /**互斥操作，不允许在读取时有其他任务切换*/
  io_cli();
  unsigned offset, i, z;
  io_out8(0x1f7, 0x0c);
  io_out16(0x1f2, number);              /**数量*/
  io_out8(0x1f3, (LBA & 0xff));         /**LBA地址7~0*/
  io_out8(0x1f4, ((LBA >> 8) & 0xff));  /**LBA地址15~8*/
  io_out8(0x1f5, ((LBA >> 16) & 0xff)); /**LBA地址23~16*/
  io_out8(0x1f6,
          (((LBA >> 24) & 0xff) + 0xe0)); /**LBA地址27~24 + LBA模式，主硬盘*/
  io_out8(0x1f7, 0x30);                   /**写扇区*/
  /**循环往DATA端口写数据*/
  for (i = 0; i != number; i++) {
    drivers_idehdd_wait2();
    for (offset = 0; offset < 256; offset++) {
      io_out16(0x1f0, buffer[(i * 256) + offset]); /**往DATA端口中写数据*/
    }
    drivers_idehdd_wait1();
  }
  /**开放任务切换*/
  io_sti();
  return;
}
struct IDEHardDiskInfomationBlock *drivers_idehdd_info() {
  io_cli();
  unsigned short *p =
      (unsigned short *)page_malloc(256 * sizeof(unsigned short));
  io_out8(0x1f7, 0xec);
  for (int i = 0; i != 256; i++) {
    p[i] = io_in16(0x1f0); /**从DATA端口中读取数据*/
  }
  io_sti();
  return (struct IDEHardDiskInfomationBlock *)p;
}

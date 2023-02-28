/*
 * sb 16 driver
 * copyright (c) by min0911_
 */
// TODO:还是有bug（WAV播放），初步判定出现在set_dsp上，set_dma应该是没问题
#include <dos.h>
#include <drivers.h>

void asm_sb16_handler(int *esp);
struct WAV16_HEADER {
  char riff[4];
  int size;
  char wave[4];
  char fmt[4];
  int fmt_size;
  short format;
  short channel;
  int sample_rate;
  int byte_per_sec;
} __attribute__((packed));
#define DMA_BASE_ADDR 0xC4
#define DMA_COUNT_PORT 0xC6
#define DMA_MASK_PORT 0xD4
#define DMA_MODE_PORT 0xD6
#define DMA_CLR_PTR_PORT 0xD8
#define DMA_PAGE_PORT 0x8B
#define DMA_START_MASK 0x01
#define DMA_STOP_MASK 0x05
#define DMA_MODE 0x59

struct sb16 {
  uint8_t buffer[2][65536];
  int use_task_id;
  int status;
};
struct sb16 sb16_driver;
void sb16_handler() {
  send_eoi(SB16_IRQ);
  if (sb16_driver.use_task_id == -1) {
    return;
  }
  printk("sb16_handler\n");
  sb16_driver.status = !sb16_driver.status; // 通知更换buffer
}
static bool reset() {
  io_out8(SB16_PORT_RESET, 0x1);
  sleep(1);
  io_out8(SB16_PORT_RESET, 0x0);
  unsigned char d = io_in8(SB16_PORT_READ);
  if (d != 0xaa) {
    WARNING_F("sb16 not found the return value is %02x", d);
    return false;
  }
  return true;
}
uint8_t dsp_read() {
  while (!(io_in8(SB16_PORT_READ_STATUS) & BUF_RDY_VAL))
    ;
  return (uint8_t)io_in8(SB16_PORT_READ);
}
void dsp_write(uint8_t command) {
  while (io_in8(SB16_PORT_WRITE) & BUF_RDY_VAL)
    ;
  io_out8(SB16_PORT_WRITE, command);
}

void set_dma(int buffer_addr, int size, int page) {
  io_out8(DMA_MASK_PORT, 0x05);
  io_out8(DMA_CLR_PTR_PORT, 0x01);
  io_out8(DMA_MODE_PORT, 0x59);
  io_out8(DMA_BASE_ADDR, (buffer_addr & 0xFF));
  io_out8(DMA_BASE_ADDR, (buffer_addr >> 8) & 0xFF);
  // LENGTH
  io_out8(DMA_COUNT_PORT, (size & 0xFF));
  io_out8(DMA_COUNT_PORT, (size >> 8) & 0xFF);
  // PAGE
  io_out8(DMA_PAGE_PORT, page);
  // START
  io_out8(DMA_MASK_PORT, 0x01);
}
void set_dsp(uint16_t sample_rate, uint8_t bcommand, uint8_t bmode,
             uint16_t block_length) {
  dsp_write(COMMAND_DSP_SOSR);
  dsp_write((sample_rate >> 8) & 0xFF);
  dsp_write(sample_rate & 0xff);
  dsp_write(bcommand);
  dsp_write(bmode);
  dsp_write(block_length & 0xff);
  dsp_write((block_length >> 8) & 0xff);
}
int init_sb16() {
  if (!reset()) {
    return;
  }
  if (sb16_driver.use_task_id != -1) {
    WARNING_F("sb16 is busy");
    return;
  }
  sb16_driver.use_task_id = Get_Tid(NowTask());
  sb16_driver.status = 0;
  // set irq
  io_out8(SB16_PORT_MIXER, COMMAND_SET_IRQ);
  io_out8(SB16_PORT_DATA, 0x02); // irq5
  ClearMaskIrq(SB16_IRQ);
  register_intr_handler(SB16_IRQ + 0x20, asm_sb16_handler);
  uint16_t buffer_offset;
  uint16_t buffer_page;
  buffer_offset = (uint32_t)((uint32_t)sb16_driver.buffer >> 1) % 65536;
  buffer_page = (uint32_t)((uint32_t)sb16_driver.buffer >> 16);
  set_dma(buffer_offset, 65536 / 2, buffer_page);
  return (int)sb16_driver.buffer;
}
void wav_set_dsp(struct WAV16_HEADER *wav_header) {
  set_dsp(wav_header->sample_rate, 0xB6, 0x30, (65536 / 4) - 1);
}
void disable_sb16() {
  if (!reset()) {
    return;
  }
  sb16_driver.use_task_id = -1;
}
void wav_player(char *filename) {
  // 我们假设wav文件是有效的
  struct WAV16_HEADER wav_header;
  FILE *fp = fopen(filename, "r");
  uint8_t *data = fp->buf;
  if (fp == NULL) {
    printf("open file error");
    return;
  }
  int fsize = Get_File_Address(filename)->size;
  memcpy(&wav_header, data, sizeof(wav_header));
  uint8_t *start = (uint8_t *)data + 44; // wav文件头的长度是44字节
  int buffer[2];
  buffer[0] = init_sb16();
  buffer[1] = buffer[0] + 65536;
  wav_set_dsp(&wav_header);
  printf("wav header->sample:%d\n", wav_header.sample_rate);
  if (fsize - 44 >= 65536) {
    memcpy(buffer[0], start, 65536);
    start += 65536;
    fsize -= 65536;
  } else {
    memcpy(buffer[0], start, fsize);
    start += fsize;
    fsize = 0;
  }
  int cstatus = 0;
  for (;;) {
    if (cstatus != sb16_driver.status) {
      cstatus = sb16_driver.status;
      printf("Switch buffer Size=%d\n", fsize);
      // 注意文件大小
      if (fsize - 44 >= 65536) {
        memcpy(buffer[0], start, 65536);
        start += 65536;
        fsize -= 65536;
      } else {
        memcpy(buffer[0], start, fsize);
        start += fsize;
        fsize = 0;
        disable_sb16();
        break;
      }
    }
  }
}
void wav_player_test() { wav_player("A:\\other\\coin.wav"); }

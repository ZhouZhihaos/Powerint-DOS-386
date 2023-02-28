// bmp.c ：位图解析
#include <fs.h>
void bmpview(char *filename) {
  int i, j;
  FILE *fp = fopen(filename, "r");
  char *p = fp->buf;
  unsigned short pxsize, pysize, start;
  unsigned int length;
  if (fp == 0) {
    print("Can't find file ");
    print(filename);
    print("\n");
    return;
  }
  if (p[0] != 'B' || p[1] != 'M') {
    print("Isn't BMP photo,format error.\n");
    return;
  }
  pxsize = *(unsigned short *)(p + 0x12);
  pysize = *(unsigned short *)(p + 0x16);
  length = *(unsigned int *)(p + 2);
  start = *(unsigned short *)(p + 0xa);
  if (set_mode(1024, 768, 8) != 0) {
    printf("Can't enable 1024x768x8 VBE mode.\n\n");
    return;
  }
  io_out8(VGA_DAC_WRITE_INDEX, 0);
  for (i = 0; i != 256; i++) {
    io_out8(VGA_DAC_DATA, p[0x36 + i * 4 + 2] / 4);
    io_out8(VGA_DAC_DATA, p[0x36 + i * 4 + 1] / 4);
    io_out8(VGA_DAC_DATA, p[0x36 + i * 4] / 4);
  }
  struct VBEINFO *vinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
  unsigned char *vram = vinfo->vram;
  for (i = 0; i < pysize; i++) {
    for (j = 0; j < pxsize; j++) {
      // Draw_Px(j, i, p[length - (i * pxsize + pxsize - j)]);
      vram[i * 1024 + j] = p[length - (i * pxsize + pxsize - j)];
    }
  }
  fclose(fp);
  return;
}
void BMPVIEW32(char *path, unsigned char *vram, int xsize) {
  FILE *fp = fopen(path, "r");
  unsigned char *buf = fp->buf;
  int i, j;
  int offset;
  int width, height;
  unsigned char r, g, b;
  int x, y;
  width = *(int *)(buf + 18);
  height = *(int *)(buf + 22);
  offset = *(int *)(buf + 10);
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      b = buf[offset + i * width * 3 + j * 3 + 0];
      g = buf[offset + i * width * 3 + j * 3 + 1];
      r = buf[offset + i * width * 3 + j * 3 + 2];
      x = j;
      y = height - 1 - i;
      Draw_Px_32(vram, x, y, r, g, b, xsize);
    }
  }
  fclose(fp);
}
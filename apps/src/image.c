#include <syscall.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_THREAD_LOCALS
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
void convert_ABGR_to_ARGB(uint32_t* bitmap, size_t num_pixels) {
  for (size_t i = 0; i < num_pixels; ++i) {
    uint32_t pixel = bitmap[i];
    uint8_t alpha = (pixel >> 24) & 0xFF;
    uint8_t red = (pixel >> 16) & 0xFF;
    uint8_t green = (pixel >> 8) & 0xFF;
    uint8_t blue = pixel & 0xFF;
    bitmap[i] = (alpha << 24) | (blue << 16) | (green << 8) | red;
  }
}
int main(int argc, char** argv) {
  if (argc == 1) {
    printf("no input file.\n");
    return 1;
  }
  int w, h, bpp;
  stbi_uc* b = stbi_load(argv[1], &w, &h, &bpp, 4);
  if (w > 1024 && h > 768) {
    uint8_t* b1 = malloc(1024 * 768 * 4);
    stbir_resize_uint8(b, w, h, 0, b1, 1024, 768, 0, 4);
    convert_ABGR_to_ARGB(b1, 1024 * 768);
    set_mode(1024, 768);
    VBESetBuffer(0, 0, 1024, 768, b1);
  } else if (w > 1024) {
    uint8_t* b1 = malloc(1024 * h * 4);
    stbir_resize_uint8(b, w, h, 0, b1, 1024, h, 0, 4);
    convert_ABGR_to_ARGB(b1, 1024 * h);
    set_mode(1024, 768);
    VBESetBuffer(0, 0, 1024, h, b1);
  } else if (h > 768) {
    uint8_t* b1 = malloc(w * 768 * 4);
    stbir_resize_uint8(b, w, h, 0, b1, w, 768, 0, 4);
    convert_ABGR_to_ARGB(b1, w * 768);
    set_mode(1024, 768);
    VBESetBuffer(0, 0, w, 768, b1);
  } else {
    convert_ABGR_to_ARGB(b, w * h);
    set_mode(1024, 768);
    VBESetBuffer(0, 0, w, h, b);
  }
  while (getch() != '\n')
    ;
  SwitchToText8025_BIOS();
  return 0;
}
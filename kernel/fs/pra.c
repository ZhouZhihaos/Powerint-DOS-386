#include <fs.h>
void MakePraFile(char *PraFileName, uint8_t *imgBuffer, uint32_t xsize,
                 uint32_t ysize) {
  struct paw_info header;                                        // 图像头
  int PraFileSize = sizeof(struct paw_info) + xsize * ysize * 3; // 图像大小
  mkfile(PraFileName);
  for (int i = 0; i < 12; i++)
    header.reserved[i] = 0xff; // 填充
  memcpy(header.oem, "PRA", 3);
  header.xsize = xsize;
  header.ysize = ysize;
  uint8_t *praFile = (uint8_t *)page_malloc(PraFileSize);
  memcpy(praFile, &header, sizeof(struct paw_info));
  uint8_t *Img_Start = praFile + sizeof(struct paw_info);
  for (int i = 0; i < ysize; i++) {
    for (int j = 0; j < xsize; j++) {
      int index = i * xsize + j;
      int r = imgBuffer[index * 3 + 0];
      int g = imgBuffer[index * 3 + 1];
      int b = imgBuffer[index * 3 + 2];
      // pra文件是倒置的
      Img_Start[(ysize - i - 1) * xsize * 3 + j * 3 + 0] = r;
      Img_Start[(ysize - i - 1) * xsize * 3 + j * 3 + 1] = g;
      Img_Start[(ysize - i - 1) * xsize * 3 + j * 3 + 2] = b;
    }
  }
  EDIT_FILE(PraFileName, praFile, PraFileSize, 0);
  page_free(praFile, PraFileSize);
}
void PraShell() {
  printf("Welcome to PraShell!\n");
  printf("We can help you make a picture.\n");
  printf("In this every one are a artist.\n");
  char *cmd = (char *)page_malloc(128);
  char *prafileName[20];
  uint8_t *imgBuffer;
  int make_file_flag = 0;
  int xsize = 0;
  int ysize = 0;
  while (1) {
    printf(">>> ");
    input(cmd, 128); // define in input.c
    if (strncmp("file ", cmd, 5) == 0) {
      make_file_flag = 1;
      // mkfile(cmd + 5);
      strncpy(prafileName, cmd + 5, 20);
      printf("The Image file xsize:");
      input(cmd, 128);
      xsize = strtol(cmd, NULL, 10);
      printf("The Image file ysize:");
      input(cmd, 128);
      ysize = strtol(cmd, NULL, 10);
      imgBuffer = (uint8_t *)page_malloc(xsize * ysize * 3);
    } else if (strcmp("exit", cmd) == 0) {
      if (make_file_flag) {
        MakePraFile(prafileName, imgBuffer, xsize, ysize);
        return;
      } else {
        return;
      }
    } else if (make_file_flag) {
      if (strncmp("Draw_Pxl ", cmd, 9) == 0) {
        // Draw_Pxl x y r g b
        char str_x[100];
        char str_y[100];
        char str_r[100];
        char str_g[100];
        char str_b[100];

        // 获取x, y, r, g, b
        char *arg_base = cmd + 9;
        if (!arg_base[0]) {
          printf("Please input x, y, r, g, b\n");
          continue;
        }
        // 拷贝x，直到空格
        char *arg_x = str_x;
        while (*arg_base != ' ' && *arg_base != '\0') {
          *arg_x = *arg_base;
          arg_x++;
          arg_base++;
          // printf("A");
        }
        if (!arg_base[0]) {
          printf("Please input x, y, r, g, b\n");
          continue;
        }
        *arg_x = '\0';
        arg_base++;
        // 拷贝y，直到空格
        char *arg_y = str_y;
        while (*arg_base != ' ' && *arg_base != '\0') {
          *arg_y = *arg_base;
          arg_y++;
          arg_base++;
        }
        if (!arg_base[0]) {
          printf("Please input x, y, r, g, b\n");
          continue;
        }
        *arg_y = '\0';
        arg_base++;
        // 拷贝r，直到空格
        char *arg_r = str_r;
        while (*arg_base != ' ' && *arg_base != '\0') {
          *arg_r = *arg_base;
          arg_r++;
          arg_base++;
        }
        if (!arg_base[0]) {
          printf("Please input x, y, r, g, b\n");
          continue;
        }
        *arg_r = '\0';
        arg_base++;
        // 拷贝g，直到空格
        char *arg_g = str_g;
        while (*arg_base != ' ' && *arg_base != '\0') {
          *arg_g = *arg_base;
          arg_g++;
          arg_base++;
        }
        if (!arg_base[0]) {
          printf("Please input x, y, r, g, b\n");
          continue;
        }
        *arg_g = '\0';
        arg_base++;
        // 拷贝b，直到空格
        char *arg_b = str_b;

        while (*arg_base != ' ' && *arg_base != '\0') {
          *arg_b = *arg_base;
          arg_b++;
          arg_base++;
        }
        if (*arg_base != '\0') {
          printf("Please input x, y, r, g, b\n");
          continue;
        }
        *arg_b = '\0';

        Draw_Px_32(imgBuffer, strtol(str_x, NULL, 10), strtol(str_y, NULL, 10),
                   strtol(str_r, NULL, 16), strtol(str_g, NULL, 16),
                   strtol(str_b, NULL, 16), xsize);

      } else if (strncmp("Draw_Box ", cmd, 9) == 0) {
        char str_x1[100];
        char str_y1[100];
        char str_x2[100];
        char str_y2[100];
        char str_r[100];
        char str_g[100];
        char str_b[100];
        // Draw_Box 0 0 0 0 ff ff ff
        if (Get_Argc(cmd) != 7) {
          printf("Please input x1, y1, x2, y2, r, g, b\n");
          continue;
        }
        Get_Arg(str_x1, cmd, 1);
        Get_Arg(str_y1, cmd, 2);
        Get_Arg(str_x2, cmd, 3);
        Get_Arg(str_y2, cmd, 4);
        Get_Arg(str_r, cmd, 5);
        Get_Arg(str_g, cmd, 6);
        Get_Arg(str_b, cmd, 7);
        printf("x1=%s y1=%s x2=%s y2=%s r=%s g=%s b=%s\n", str_x1, str_y1,
               str_x2, str_y2, str_r, str_g, str_b);
        Draw_Box32(imgBuffer, xsize, strtol(str_x1, NULL, 10),
                   strtol(str_y1, NULL, 10), strtol(str_x2, NULL, 10),
                   strtol(str_y2, NULL, 10), strtol(str_r, NULL, 16),
                   strtol(str_g, NULL, 16), strtol(str_b, NULL, 16));
      } else if (strncmp("Draw_Line ", cmd, 10) == 0) {
        {
          char str_x1[100];
          char str_y1[100];
          char str_x2[100];
          char str_y2[100];
          char str_r[100];
          char str_g[100];
          char str_b[100];
          if (Get_Argc(cmd) != 7) {
            printf("Draw_Line x1 y1 x2 y2 r g b\n");
            continue;
          }
          Get_Arg(str_x1, cmd, 1);
          Get_Arg(str_y1, cmd, 2);
          Get_Arg(str_x2, cmd, 3);
          Get_Arg(str_y2, cmd, 4);
          Get_Arg(str_r, cmd, 5);
          Get_Arg(str_g, cmd, 6);
          Get_Arg(str_b, cmd, 7);
          Draw_Line_32(imgBuffer, strtol(str_x1, NULL, 10),
                       strtol(str_y1, NULL, 10), strtol(str_x2, NULL, 10),
                       strtol(str_y2, NULL, 10), strtol(str_r, NULL, 16),
                       strtol(str_g, NULL, 16), strtol(str_b, NULL, 16), xsize);
        }
      } else {
        printf("Unknown command!\n");
      }

    } else {
      printf("Unknown command!\n");
    }
  }
}
void pra_view_32(unsigned char *path, unsigned char *vram, int scr_xsize) {
  // 判断是否是PRA文件
  // 1. 检查前12个字节是否为0xFF
  // 2. 检查OEM是否为PRA
  FILE *fp = fopen(path, "r");
  unsigned char *img = fp->buf;
  for (int i = 0; i < 12; i++) {
    if (img[i] != 0xFF) {
      SwitchToText8025_BIOS();
      Set_Font("tskdrv:\\other\\font.bin");
      printf("format error.\n");
      fclose(fp);
      return;
    }
  }
  if (img[12] != 'P' || img[13] != 'R' || img[14] != 'A') {
    SwitchToText8025_BIOS();
    Set_Font("tskdrv:\\other\\font.bin");
    printf("format error.\n");
    fclose(fp);
    return;
  }
  // 这是一个PRA文件
  struct paw_info *paw_info = (struct paw_info *)img;
  unsigned char *start = img + sizeof(struct paw_info);
  int xsize = paw_info->xsize;
  int ysize = paw_info->ysize;
  int x, y;
  unsigned char r, g, b;
  for (y = 0; y < ysize; y++) {
    for (x = 0; x < xsize; x++) {
      int x1 = x;
      int y1 = ysize - y - 1;
      r = start[(xsize * y + x) * 3 + 0];
      g = start[(xsize * y + x) * 3 + 1];
      b = start[(xsize * y + x) * 3 + 2];
      Draw_Px_32(vram, x1, y1, r, g, b, scr_xsize);
    }
  }
  fclose(fp);
}
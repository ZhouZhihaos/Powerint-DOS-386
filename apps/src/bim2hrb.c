#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
typedef unsigned char UCHAR;
int getnum(const UCHAR* p);
int get32(const UCHAR* p);
void put32(UCHAR* p, int i);

#define MAXSIZ 4*1024*1024

int main(int argc, UCHAR** argv) {
  UCHAR* fbuf = malloc(MAXSIZ);
  int heap_siz, mmarea, fsiz, dsize, dofs, stksiz, wrksiz, entry, bsssiz;
  int heap_adr, i;
  FILE* fp;
  static UCHAR sign[4] = "Hari";

  /* �p�����[�^�̎擾 */
  if (argc < 4) {
    puts("usage>bim2hrb appname.bim appname.hrb heap-size [mmarea-size]");
    return 1;
  }
  heap_siz = getnum(argv[3]);
  mmarea = 0;
  if (argc >= 5)
    mmarea = getnum(argv[4]);

  /* �t�@�C���ǂݍ��� */
  fp = fopen(argv[1], "rb");
  if (fp == NULL) {
  err_bim:
    puts("bim file read error");
    return 1;
  }
  fsiz = fread(fbuf, 1, MAXSIZ, fp);
  fclose(fp);
  if (fsiz >= MAXSIZ || fsiz < 0) {
    goto err_bim;
  }
  /* �w�b�_�m�F */
  if (get32(&fbuf[4]) != 0x24) { /* �t�@�C������.text�X�^�[�g�A�h���X */
  err_form:
    puts("bim file format error");
    return 1;
  }
  if (get32(&fbuf[8]) != 0x24) /* ���������[�h����.text�X�^�[�g�A�h���X */
    goto err_form;
  dsize = get32(&fbuf[12]); /* .data�Z�N�V�����T�C�Y */
  dofs = get32(&fbuf[16]); /* �t�@�C���̂ǂ���.data�Z�N�V���������邩 */
  stksiz = get32(&fbuf[20]); /* �X�^�b�N�T�C�Y */
  entry = get32(&fbuf[24]);  /* �G���g���|�C���g */
  bsssiz = get32(&fbuf[28]); /* bss�T�C�Y */

  /* �w�b�_���� */
  heap_adr = stksiz + dsize + bsssiz;
  heap_adr =
      (heap_adr + 0xf) & 0xfffffff0; /* 16�o�C�g�P�ʂɐ؂�グ */
  wrksiz = heap_adr + heap_siz;
  wrksiz = (wrksiz + 0xfff) & 0xfffff000; /* 4KB�P�ʂɐ؂�グ */
  put32(&fbuf[0], wrksiz);
  for (i = 0; i < 4; i++)
    fbuf[4 + i] = sign[i];
  put32(&fbuf[8], mmarea);
  put32(&fbuf[12], stksiz);
  put32(&fbuf[16], dsize);
  put32(&fbuf[20], dofs);
  put32(&fbuf[24], 0xe9000000);
  put32(&fbuf[28], entry - 0x20);
  put32(&fbuf[32], heap_adr);

  /* �t�@�C���������� */
  fp = fopen(argv[2], "wb");
  if (fp == NULL) {
  err_hrb:
    puts("hrb file write error");
    return 1;
  }
  i = fwrite(fbuf, 1, fsiz, fp);
  fclose(fp);
  if (fsiz != i)
    goto err_hrb;

  return 0;
}

int getnum(const UCHAR* p) {
  int i = 0, base = 10, sign = 1;
  UCHAR c;
  if (*p == '-') {
    p++;
    sign = -1;
  }
  if (*p == '0') {
    p++;
    base = 8;
    c = *p;
    if (c >= 'a')
      c -= 'a' - 'A';
    if (c == 'X') {
      p++;
      base = 16;
    }
    if (c == 'O') {
      p++;
      base = 8;
    }
    if (c == 'B') {
      p++;
      base = 2;
    }
  }
  for (;;) {
    c = *p++;
    if ('0' <= c && c <= '9')
      c -= '0';
    else if ('A' <= c && c <= 'F')
      c -= 'A' - 10;
    else if ('a' <= c && c <= 'f')
      c -= 'a' - 10;
    else
      break;
    if (c >= base)
      break;
    i = i * base + c;
  }
  if (c >= 'a')
    c -= 'a' - 'A';
  if (c == 'K')
    i <<= 10;
  if (c == 'M')
    i <<= 20;
  if (c == 'G')
    i <<= 30;
  return i * sign;
}

int get32(const UCHAR* p) {
  return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

void put32(UCHAR* p, int i) {
  p[0] = i & 0xff;
  p[1] = (i >> 8) & 0xff;
  p[2] = (i >> 16) & 0xff;
  p[3] = (i >> 24) & 0xff;
  return;
}

/*

memo

[ .bim�t�@�C���̍\�� ]

+ 0 : .text�T�C�Y
+ 4 : �t�@�C������.text�X�^�[�g�A�h���X�i0x24�j
+ 8 : ���������[�h����.text�X�^�[�g�A�h���X�i0x24�j
+12 : .data�T�C�Y
+16 : �t�@�C������.data�X�^�[�g�A�h���X
+20 : ���������[�h����.data�X�^�[�g�A�h���X
+24 : �G���g���|�C���g
+28 : bss�̈�̃o�C�g��
+36 : �R�[�h

[ .hrb�t�@�C���̍\�� ]

+ 0 : stack+.data+heap �̑傫���i4KB�̔{���j
+ 4 : �V�O�l�`�� "Hari"
+ 8 : mmarea �̑傫���i4KB�̔{���j
+12 : �X�^�b�N�����l��.data�]����
+16 : .data�̃T�C�Y
+20 : .data�̏����l�񂪃t�@�C���̂ǂ��ɂ��邩
+24 : 0xe9000000
+28 : �G���g���A�h���X-0x20
+32 : heap�̈�imalloc�̈�j�J�n�A�h���X

*/

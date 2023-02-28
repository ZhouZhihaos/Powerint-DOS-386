
/*
uPNG -- derived from LodePNG version 20100808
Copyright (c) 2005-2010 Lode Vandevenne
Copyright (c) 2010 Sean Middleditch
This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
                1. The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use this software in
a product, an acknowledgment in the product documentation would be appreciated
but is not required.
                2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.
                3. This notice may not be removed or altered from any source
                distribution.
*/

#ifndef UPNG_H
#define UPNG_H 1
enum upng_format {
  UPNG_BADFORMAT,
  UPNG_RGB8,
  UPNG_RGB16,
  UPNG_RGBA8,
  UPNG_RGBA16,
  UPNG_LUMINANCE1,
  UPNG_LUMINANCE2,
  UPNG_LUMINANCE4,
  UPNG_LUMINANCE8,
  UPNG_LUMINANCE_ALPHA1,
  UPNG_LUMINANCE_ALPHA2,
  UPNG_LUMINANCE_ALPHA4,
  UPNG_LUMINANCE_ALPHA8
};
enum upng_error {
  UPNG_EOK = 0,           /* success (no error) */
  UPNG_ENOMEM = 1,        /* memory allocation failed */
  UPNG_ENOTFOUND = 2,     /* resource not found (file missing) */
  UPNG_ENOTPNG = 3,       /* image data does not have a PNG header */
  UPNG_EMALFORMED = 4,    /* image data is not a valid PNG image */
  UPNG_EUNSUPPORTED = 5,  /* critical PNG chunk type is not supported */
  UPNG_EUNINTERLACED = 6, /* image interlacing is not supported */
  UPNG_EUNFORMAT = 7,     /* image color format is not supported */
  UPNG_EPARAM = 8         /* invalid parameter to method call */
};
typedef enum upng_error upng_error;

typedef enum upng_format upng_format;
typedef enum upng_state {
  UPNG_ERROR = -1,
  UPNG_DECODED = 0,
  UPNG_HEADER = 1,
  UPNG_NEW = 2
} upng_state;

typedef enum upng_color {
  UPNG_LUM = 0,
  UPNG_RGB = 2,
  UPNG_LUMA = 4,
  UPNG_RGBA = 6
} upng_color;

typedef struct upng_source {
  const unsigned char* buffer;
  unsigned long size;
  char owning;
} upng_source;

struct upng_t {
  unsigned width;
  unsigned height;

  upng_color color_type;
  unsigned color_depth;
  upng_format format;

  unsigned char* buffer;
  unsigned long size;

  upng_error error;
  unsigned error_line;

  upng_state state;
  upng_source source;
};

typedef struct huffman_tree {
  unsigned* tree2d;
  unsigned maxbitlen; /*maximum number of bits a single code can get */
  unsigned numcodes;  /*number of symbols in the alphabet = number of codes */
} huffman_tree;
typedef struct upng_t upng_t;

upng_t* upng_new_from_bytes(const unsigned char* buffer, unsigned long size);
upng_t* upng_new_from_file(const char *filename);
void upng_free(upng_t* upng);

upng_error upng_header(upng_t* upng);
upng_error upng_decode(upng_t* upng);

upng_error upng_get_error(const upng_t* upng);
unsigned upng_get_error_line(const upng_t* upng);

unsigned upng_get_width(const upng_t* upng);
unsigned upng_get_height(const upng_t* upng);
unsigned upng_get_bpp(const upng_t* upng);
unsigned upng_get_bitdepth(const upng_t* upng);
unsigned upng_get_components(const upng_t* upng);
unsigned upng_get_pixelsize(const upng_t* upng);
upng_format upng_get_format(const upng_t* upng);

const unsigned char* upng_get_buffer(const upng_t* upng);
unsigned upng_get_size(const upng_t* upng);
#define MAKE_BYTE(b) ((b)&0xFF)
#define MAKE_DWORD(a, b, c, d)                                         \
  ((MAKE_BYTE(a) << 24) | (MAKE_BYTE(b) << 16) | (MAKE_BYTE(c) << 8) | \
   MAKE_BYTE(d))
#define MAKE_DWORD_PTR(p) MAKE_DWORD((p)[0], (p)[1], (p)[2], (p)[3])

#define CHUNK_IHDR MAKE_DWORD('I', 'H', 'D', 'R')
#define CHUNK_IDAT MAKE_DWORD('I', 'D', 'A', 'T')
#define CHUNK_IEND MAKE_DWORD('I', 'E', 'N', 'D')

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285

#define NUM_DEFLATE_CODE_SYMBOLS \
  288 /*256 literals, the end code, some length codes, and 2 unused codes */
#define NUM_DISTANCE_SYMBOLS \
  32 /*the distance codes have their own symbols, 30 used, 2 unused */
#define NUM_CODE_LENGTH_CODES                                                  \
  19 /*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, \
        17: 3-10 zeros, 18: 11-138 zeros */
#define MAX_SYMBOLS 288 /* largest number of symbols used by any tree type */

#define DEFLATE_CODE_BITLEN 15
#define DISTANCE_BITLEN 15
#define CODE_LENGTH_BITLEN 7
#define MAX_BIT_LENGTH 15 /* largest bitlen used by any tree type */

#define DEFLATE_CODE_BUFFER_SIZE (NUM_DEFLATE_CODE_SYMBOLS * 2)
#define DISTANCE_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)
#define CODE_LENGTH_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)

#define SET_ERROR(upng, code)      \
  do {                             \
    (upng)->error = (code);        \
    (upng)->error_line = __LINE__; \
  } while (0)
#define upng_chunk_length(chunk) MAKE_DWORD_PTR(chunk)
#define upng_chunk_type(chunk) MAKE_DWORD_PTR((chunk) + 4)
#define upng_chunk_critical(chunk) (((chunk)[4] & 32) == 0)
#endif /*defined(UPNG_H)*/
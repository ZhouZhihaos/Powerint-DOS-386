#include <errno.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
typedef unsigned int uintmax_t;
typedef uintmax_t uintptr_t;
typedef int intmax_t;

int errno = 0;
FILE* stdout;
FILE* stdin;
FILE* stderr;
int vsnprintf(char* buf, size_t n, const char* fmt, va_list ap);
#define LONG_MAX 0x7fffffff
#define LONG_MIN (-LONG_MAX - 1)
#define INT_MAX 0x7fffffff
#define INT_MIN (-LONG_MAX - 1)
// strcmp
int strcmp(const char* s1, const char* s2) {
  while (*s1 == *s2) {
    if (*s1 == '\0')
      return 0;
    s1++;
    s2++;
  }
  return *s1 - *s2;
}
// strcpy
char* strcpy(char* dest, const char* src) {
  char* tmp = dest;
  while ((*dest++ = *src++) != '\0')
    ;
  return tmp;
}
// strncpy
char* strncpy(char* dest, const char* src, size_t n) {
  char* tmp = dest;
  while (n-- > 0 && (*dest++ = *src++) != '\0')
    ;
  return tmp;
}
// strlen
size_t strlen(const char* s) {
  size_t len = 0;
  while (s[len])
    len++;
  return len;
}
// strcat
char* strcat(char* dest, const char* src) {
  char* tmp = dest;
  while (*dest)
    dest++;
  while ((*dest++ = *src++) != '\0')
    ;
  return tmp;
}
// strncat
char* strncat(char* dest, const char* src, size_t n) {
  char* tmp = dest;
  while (*dest)
    dest++;
  while (n-- > 0 && (*dest++ = *src++) != '\0')
    ;
  return tmp;
}
// memset
void* memset(void* s, int c, size_t n) {
  unsigned char* p = s;
  while (n-- > 0)
    *p++ = c;
  return s;
}
// strtol
long strtol(const char* nptr, char** endptr, int base) {
  long acc = 0;
  int c;
  unsigned long cutoff;
  int neg = 0, any, cutlim;

  /*
   * Skip white space and pick up leading +/- sign if any.
   * If base is 0, allow 0x for hex and 0 for octal, else
   * assume decimal; if base is already 16, allow 0x.
   */
  do {
    c = *nptr++;
  } while (isspace(c));
  if (c == '-') {
    neg = 1;
    c = *nptr++;
  } else if (c == '+')
    c = *nptr++;
  if ((base == 0 || base == 16) && c == '0' && (*nptr == 'x' || *nptr == 'X')) {
    c = nptr[1];
    nptr += 2;
    base = 16;
  } else if ((base == 0 || base == 2) && c == '0' &&
             (*nptr == 'b' || *nptr == 'B')) {
    c = nptr[1];
    nptr += 2;
    base = 2;
  } else if (base == 0)
    base = c == '0' ? 8 : 10;

  /*
   * Compute the cutoff value between legal numbers and illegal
   * numbers.  That is the largest legal value, divided by the
   * base.  An input number that is greater than this value, if
   * followed by a legal input character, is too big.  One that
   * is equal to this value may be valid or not; the limit
   * between valid and invalid numbers is then based on the last
   * digit.  For instance, if the range for longs is
   * [-2147483648..2147483647] and the input base is 10,
   * cutoff will be set to 214748364 and cutlim to either
   * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
   * a value > 214748364, or equal but the next digit is > 7 (or 8),
   * the number is too big, and we will return a range error.
   */
  cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
  cutlim = cutoff % (unsigned long)base;
  cutoff /= (unsigned long)base;
  for (acc = 0, any = 0;; c = *nptr++) {
    if (isdigit(c))
      c -= '0';
    else if (isalpha(c))
      c -= isupper(c) ? 'A' - 10 : 'a' - 10;
    else
      break;
    if (c >= base)
      break;
    if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
      any = -1;
    else {
      any = 1;
      acc *= base;
      acc += c;
    }
  }
  if (any < 0) {
    acc = neg ? LONG_MIN : LONG_MAX;
    // errno = ERANGE;
    print("panic: strtol: overflow\n");
  } else if (neg)
    acc = -acc;
  if (endptr != 0)
    *endptr = (char*)(any ? nptr : (char*)nptr - 1);
  return (acc);
}
// memcmp
int memcmp(const void* s1, const void* s2, size_t n) {
  const unsigned char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2)
      return *p1 - *p2;
    p1++, p2++;
  }
  return 0;
}
// memcpy
void* memcpy(void* s, const void* ct, size_t n) {
  if (NULL == s || NULL == ct || n <= 0)
    return NULL;
  while (n--)
    *(char*)s++ = *(char*)ct++;
  return s;
}
// isspace
int isspace(int c) {
  return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' ||
          c == '\v');
}
// isdigit
int isdigit(int c) {
  return (c >= '0' && c <= '9');
}
// isalpha
int isalpha(int c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
// isupper
int isupper(int c) {
  return (c >= 'A' && c <= 'Z');
}
// strncmp
int strncmp(const char* s1, const char* s2, size_t n) {
  const unsigned char *p1 = (const unsigned char*)s1,
                      *p2 = (const unsigned char*)s2;
  while (n-- > 0) {
    if (*p1 != *p2)
      return *p1 - *p2;
    if (*p1 == '\0')
      return 0;
    p1++, p2++;
  }
  return 0;
}
#define FLAG_ALTNT_FORM 0x01
#define FLAG_ALTNT_FORM_CH '#'

#define FLAG_ZERO_PAD 0x02
#define FLAG_ZERO_PAD_CH '0'

#define FLAG_LEFT_ADJUST 0x04
#define FLAG_LEFT_ADJUST_CH '-'

#define FLAG_SPACE_BEFORE_POS_NUM 0x08
#define FLAG_SPACE_BEFORE_POS_NUM_CH ' '

#define FLAG_SIGN 0x10
#define FLAG_SIGN_CH '+'

#define FLAG_LOWER 0x20

#define INT_TYPE_CHAR 0x1
#define INT_TYPE_SHORT 0x2
#define INT_TYPE_INT 0x4
#define INT_TYPE_LONG 0x8
#define INT_TYPE_LONG_LONG 0x10
#define INT_TYPE_MIN INT_TYPE_CHAR
#define INT_TYPE_MAX INT_TYPE_LONG_LONG

#define BUF_SIZE 4096

static char str_buf[BUF_SIZE];
static char num_str_buf[BUF_SIZE];

static char* int32_to_str_dec(int32_t num, int flag, int width) {
  int32_t num_tmp = num;
  char* p = num_str_buf;
  char* q = NULL;
  int len = 0;
  char* str_first = NULL;
  char* str_end = NULL;
  char ch = 0;

  memset(num_str_buf, 0, sizeof(num_str_buf));

  char dic[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

  if (num_tmp < 0) {
    *p++ = '-';
  } else if (flag & FLAG_SIGN) {
    *p++ = '+';
  } else {
    *p++ = ' ';
  }
  str_first = num_str_buf;

  if (num_tmp >= 0 && !(flag & FLAG_SIGN)) {
    str_first++;
  }

  if (num_tmp == 0) {
    *p++ = '0';
  } else {
    if (num_tmp < 0)
      num_tmp = -num_tmp;

    while (num_tmp) {
      *p++ = dic[num_tmp % 10];
      num_tmp /= 10;
    }
  }
  *p = '\0';

  str_end = p;
  len = str_end - str_first;

  q = num_str_buf + 1;
  p--;
  while (q < p) {
    ch = *q;
    *q = *p;
    *p = ch;
    p--;
    q++;
  }

  if (len < width) {
    p = str_end;

    if (flag & FLAG_LEFT_ADJUST) {
      for (int i = 0; i < width - len; i++)
        *p++ = ' ';
      *p = '\0';
      str_end = p;
    } else {
      while (p >= str_first) {
        *(p + width - len) = *p;
        p--;
      }

      if (flag & FLAG_ZERO_PAD) {
        for (int i = 0; i < width - len; i++) {
          num_str_buf[i + 1] = '0';
        }
      } else {
        for (int i = 0; i < width - len; i++)
          str_first[i] = ' ';
      }
    }
  }

  return str_first;
}

static char* int64_to_str_dec(int64_t num, int flag, int width) {
  return 0;
}

static char* uint32_to_str_hex(uint32_t num, int flag, int width) {
  uint32_t num_tmp = num;
  char* p = num_str_buf;
  char* q = NULL;
  int len = 0;
  char* str_first = NULL;
  char* str_end = NULL;
  char ch = 0;

  memset(num_str_buf, 0, sizeof(num_str_buf));

  char dic_lower[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  char dic_upper[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  char* dic = (flag & FLAG_LOWER) ? dic_lower : dic_upper;

  str_first = num_str_buf;

  *p++ = '0';
  *p++ = (flag & FLAG_LOWER) ? 'x' : 'X';

  if (!(flag & FLAG_ALTNT_FORM)) {
    str_first += 2;
  }

  do {
    *p++ = dic[num_tmp % 16];
    num_tmp /= 16;
  } while (num_tmp);
  *p = '\0';

  str_end = p;
  len = str_end - str_first;

  q = num_str_buf + 2;
  p--;
  while (q < p) {
    ch = *q;
    *q = *p;
    *p = ch;
    p--;
    q++;
  }

  if (len < width) {
    p = str_end;

    if (flag & FLAG_LEFT_ADJUST) {
      for (int i = 0; i < width - len; i++)
        *p++ = ' ';
      *p = '\0';
      str_end = p;
    } else {
      while (p >= str_first) {
        *(p + width - len) = *p;
        p--;
      }
      if (flag & FLAG_ALTNT_FORM)
        str_first[1] = (flag & FLAG_LOWER) ? 'x' : 'X';

      if (flag & FLAG_ZERO_PAD) {
        for (int i = 0; i < width - len; i++) {
          num_str_buf[i + 2] = '0';
        }
      } else {
        for (int i = 0; i < width - len; i++)
          str_first[i] = ' ';
      }
    }
  }

  if (num == 0 && flag & FLAG_ALTNT_FORM)
    str_first[1] = '0';

  return str_first;
}

static char* uint64_to_str_hex(uint64_t num, int flag, int width) {
  uint64_t num_tmp = num;
  char* p = num_str_buf;
  char* q = NULL;
  int len = 0;
  char* str_first = NULL;
  char* str_end = NULL;
  char ch = 0;

  memset(num_str_buf, 0, sizeof(num_str_buf));

  char dic_lower[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  char dic_upper[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  char* dic = (flag & FLAG_LOWER) ? dic_lower : dic_upper;

  str_first = num_str_buf;

  *p++ = '0';
  *p++ = (flag & FLAG_LOWER) ? 'x' : 'X';

  if (!(flag & FLAG_ALTNT_FORM)) {
    str_first += 2;
  }

  while (num_tmp) {
    *p++ = dic[num_tmp % 16];
    num_tmp /= 16;
  }
  *p = '\0';

  str_end = p;
  len = str_end - str_first;

  q = num_str_buf + 2;
  p--;
  while (q < p) {
    ch = *q;
    *q = *p;
    *p = ch;
    p--;
    q++;
  }

  if (len < width) {
    p = str_end;

    while (p >= str_first) {
      *(p + width - len) = *p;
      p--;
    }

    if (flag & FLAG_ZERO_PAD) {
      for (int i = 0; i < width - len; i++) {
        num_str_buf[i + 2] = '0';
      }
    } else {
      for (int i = 0; i < width - len; i++)
        str_first[i] = ' ';
    }
  }

  return str_first;
}

static char* uint32_to_str_oct(uint32_t num, int flag, int width) {
  uint32_t num_tmp = num;
  char* p = num_str_buf;
  char* q = NULL;
  int len = 0;
  char* str_first = NULL;
  char* str_end = NULL;
  char ch = 0;

  memset(num_str_buf, 0, sizeof(num_str_buf));

  char dic[] = {'0', '1', '2', '3', '4', '5', '6', '7'};

  str_first = num_str_buf;

  *p++ = '0';

  if (!(flag & FLAG_ALTNT_FORM)) {
    str_first += 1;
  }

  while (num_tmp) {
    *p++ = dic[num_tmp % 8];
    num_tmp /= 8;
  }
  *p = '\0';

  str_end = p;
  len = str_end - str_first;

  q = num_str_buf + 1;
  p--;
  while (q < p) {
    ch = *q;
    *q = *p;
    *p = ch;
    p--;
    q++;
  }

  if (len < width) {
    p = str_end;

    if (flag & FLAG_LEFT_ADJUST) {
      for (int i = 0; i < width - len; i++)
        *p++ = ' ';
      *p = '\0';
      str_end = p;
    } else {
      while (p >= str_first) {
        *(p + width - len) = *p;
        p--;
      }

      if (flag & FLAG_ZERO_PAD) {
        for (int i = 0; i < width - len; i++) {
          num_str_buf[i + 1] = '0';
        }
      } else {
        for (int i = 0; i < width - len; i++)
          str_first[i] = ' ';
      }
    }
  }

  return str_first;
}

static char* insert_str(char* buf, const char* str) {
  char* p = buf;

  while (*str) {
    *p++ = *str++;
  }

  return p;
}
enum flags {
  FL_ZERO = 0x01,   /* Zero modifier */
  FL_MINUS = 0x02,  /* Minus modifier */
  FL_PLUS = 0x04,   /* Plus modifier */
  FL_TICK = 0x08,   /* ' modifier */
  FL_SPACE = 0x10,  /* Space modifier */
  FL_HASH = 0x20,   /* # modifier */
  FL_SIGNED = 0x40, /* Number is signed */
  FL_UPPER = 0x80,  /* Upper case digits */
};

/*
 * These may have to be adjusted on certain implementations
 */
enum ranks {
  rank_char = -2,
  rank_short = -1,
  rank_int = 0,
  rank_long = 1,
  rank_longlong = 2,
};

#define MIN_RANK rank_char
#define MAX_RANK rank_longlong
#define INTMAX_RANK rank_longlong
#define SIZE_T_RANK rank_long
#define PTRDIFF_T_RANK rank_long

#define EMIT(x)   \
  ({              \
    if (o < n) {  \
      *q++ = (x); \
    }             \
    o++;          \
  })

static size_t format_int(char* q,
                         size_t n,
                         uintmax_t val,
                         enum flags flags,
                         int base,
                         int width,
                         int prec) {
  char* qq;
  size_t o = 0, oo;
  static const char lcdigits[] = "0123456789abcdef";
  static const char ucdigits[] = "0123456789ABCDEF";
  const char* digits;
  uintmax_t tmpval;
  int minus = 0;
  int ndigits = 0, nchars;
  int tickskip, b4tick;

  /*
   * Select type of digits
   */
  digits = (flags & FL_UPPER) ? ucdigits : lcdigits;

  /*
   * If signed, separate out the minus
   */
  if ((flags & FL_SIGNED) && ((intmax_t)val < 0)) {
    minus = 1;
    val = (uintmax_t)(-(intmax_t)val);
  }

  /*
   * Count the number of digits needed.  This returns zero for 0
   */
  tmpval = val;
  while (tmpval) {
    tmpval /= base;
    ndigits++;
  }

  /*
   * Adjust ndigits for size of output
   */
  if ((flags & FL_HASH) && (base == 8)) {
    if (prec < ndigits + 1)
      prec = ndigits + 1;
  }

  if (ndigits < prec) {
    ndigits = prec; /* Mandatory number padding */
  } else if (val == 0) {
    ndigits = 1; /* Zero still requires space */
  }

  /*
   * For ', figure out what the skip should be
   */
  if (flags & FL_TICK) {
    tickskip = (base == 16) ? 4 : 3;
  } else {
    tickskip = ndigits; /* No tick marks */
  }

  /*
   * Tick marks aren't digits, but generated by the number converter
   */
  ndigits += (ndigits - 1) / tickskip;

  /*
   * Now compute the number of nondigits
   */
  nchars = ndigits;

  if (minus || (flags & (FL_PLUS | FL_SPACE)))
    nchars++; /* Need space for sign */
  if ((flags & FL_HASH) && (base == 16)) {
    nchars += 2; /* Add 0x for hex */
  }

  /*
   * Emit early space padding
   */
  if (!(flags & (FL_MINUS | FL_ZERO)) && (width > nchars)) {
    while (width > nchars) {
      EMIT(' ');
      width--;
    }
  }

  /*
   * Emit nondigits
   */
  if (minus)
    EMIT('-');
  else if (flags & FL_PLUS)
    EMIT('+');
  else if (flags & FL_SPACE)
    EMIT(' ');

  if ((flags & FL_HASH) && (base == 16)) {
    EMIT('0');
    EMIT((flags & FL_UPPER) ? 'X' : 'x');
  }

  /*
   * Emit zero padding
   */
  if (((flags & (FL_MINUS | FL_ZERO)) == FL_ZERO) && (width > ndigits)) {
    while (width > nchars) {
      EMIT('0');
      width--;
    }
  }

  /*
   * Generate the number.  This is done from right to left
   */
  q += ndigits; /* Advance the pointer to end of number */
  o += ndigits;
  qq = q;
  oo = o; /* Temporary values */

  b4tick = tickskip;
  while (ndigits > 0) {
    if (!b4tick--) {
      qq--;
      oo--;
      ndigits--;
      if (oo < n)
        *qq = '_';
      b4tick = tickskip - 1;
    }
    qq--;
    oo--;
    ndigits--;
    if (oo < n)
      *qq = digits[val % base];
    val /= base;
  }

  /*
   * Emit late space padding
   */
  while ((flags & FL_MINUS) && (width > nchars)) {
    EMIT(' ');
    width--;
  }

  return o;
}

#define CVT_BUFSZ (309 + 43)

static char* cvt(double arg,
                 int ndigits,
                 int* decpt,
                 int* sign,
                 char* buf,
                 int eflag) {
  int r2;
  double fi, fj;
  char *p, *p1;

  if (ndigits < 0)
    ndigits = 0;
  if (ndigits >= CVT_BUFSZ - 1)
    ndigits = CVT_BUFSZ - 2;

  r2 = 0;
  *sign = 0;
  p = &buf[0];

  if (arg < 0) {
    *sign = 1;
    arg = -arg;
  }
  arg = modf(arg, &fi);
  p1 = &buf[CVT_BUFSZ];

  if (fi != 0) {
    p1 = &buf[CVT_BUFSZ];
    while (fi != 0) {
      fj = modf(fi / 10, &fi);
      *--p1 = (int)((fj + .03) * 10) + '0';
      r2++;
    }
    while (p1 < &buf[CVT_BUFSZ])
      *p++ = *p1++;
  } else if (arg > 0) {
    while ((fj = arg * 10) < 1) {
      arg = fj;
      r2--;
    }
  }

  p1 = &buf[ndigits];
  if (eflag == 0)
    p1 += r2;
  *decpt = r2;
  if (p1 < &buf[0]) {
    buf[0] = '\0';
    return buf;
  }

  while (p <= p1 && p < &buf[CVT_BUFSZ]) {
    arg *= 10;
    arg = modf(arg, &fj);
    *p++ = (int)fj + '0';
  }

  if (p1 >= &buf[CVT_BUFSZ]) {
    buf[CVT_BUFSZ - 1] = '\0';
    return buf;
  }
  p = p1;
  *p1 += 5;

  while (*p1 > '9') {
    *p1 = '0';
    if (p1 > buf)
      ++*--p1;
    else {
      *p1 = '1';
      (*decpt)++;
      if (eflag == 0) {
        if (p > buf)
          *p = '0';
        p++;
      }
    }
  }

  *p = '\0';
  return buf;
}

static void cfltcvt(double value, char* buffer, char fmt, int precision) {
  int decpt, sign, exp, pos;
  char* digits = 0;
  char cvtbuf[CVT_BUFSZ];
  int capexp = 0;
  int magnitude;

  if (fmt == 'G' || fmt == 'E') {
    capexp = 1;
    fmt += 'a' - 'A';
  }

  if (fmt == 'g') {
    digits = cvt(value, precision, &decpt, &sign, cvtbuf, 1);

    magnitude = decpt - 1;
    if (magnitude < -4 || magnitude > precision - 1) {
      fmt = 'e';
      precision -= 1;
    } else {
      fmt = 'f';
      precision -= decpt;
    }
  }

  if (fmt == 'e') {
    digits = cvt(value, precision + 1, &decpt, &sign, cvtbuf, 1);

    if (sign)
      *buffer++ = '-';
    *buffer++ = *digits;
    if (precision > 0)
      *buffer++ = '.';
    memcpy(buffer, digits + 1, precision);
    buffer += precision;
    *buffer++ = capexp ? 'E' : 'e';

    if (decpt == 0) {
      if (value == 0.0)
        exp = 0;
      else
        exp = -1;
    } else
      exp = decpt - 1;

    if (exp < 0) {
      *buffer++ = '-';
      exp = -exp;
    } else
      *buffer++ = '+';

    buffer[2] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[1] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[0] = (exp % 10) + '0';
    buffer += 3;
  } else if (fmt == 'f') {
    digits = cvt(value, precision, &decpt, &sign, cvtbuf, 0);

    if (sign)
      *buffer++ = '-';
    if (*digits) {
      if (decpt <= 0) {
        *buffer++ = '0';
        *buffer++ = '.';
        for (pos = 0; pos < -decpt; pos++)
          *buffer++ = '0';
        while (*digits)
          *buffer++ = *digits++;
      } else {
        pos = 0;
        while (*digits) {
          if (pos++ == decpt)
            *buffer++ = '.';
          *buffer++ = *digits++;
        }
      }
    } else {
      *buffer++ = '0';
      if (precision > 0) {
        *buffer++ = '.';
        for (pos = 0; pos < precision; pos++)
          *buffer++ = '0';
      }
    }
  }

  *buffer = '\0';
}

static void forcdecpt(char* buffer) {
  while (*buffer) {
    if (*buffer == '.')
      return;
    if (*buffer == 'e' || *buffer == 'E')
      break;
    buffer++;
  }

  if (*buffer) {
    int n = strlen(buffer);
    while (n > 0) {
      buffer[n + 1] = buffer[n];
      n--;
    }

    *buffer = '.';
  } else {
    *buffer++ = '.';
    *buffer = '\0';
  }
}

static void cropzeros(char* buffer) {
  char* stop;

  while (*buffer && *buffer != '.')
    buffer++;
  if (*buffer++) {
    while (*buffer && *buffer != 'e' && *buffer != 'E')
      buffer++;
    stop = buffer--;
    while (*buffer == '0')
      buffer--;
    if (*buffer == '.')
      buffer--;
    while ((*++buffer = *stop++))
      ;
  }
}

static size_t format_float(char* q,
                           size_t n,
                           double val,
                           enum flags flags,
                           char fmt,
                           int width,
                           int prec) {
  size_t o = 0;
  char tmp[CVT_BUFSZ];
  char c, sign;
  int len, i;

  if (flags & FL_MINUS)
    flags &= ~FL_ZERO;

  c = (flags & FL_ZERO) ? '0' : ' ';
  sign = 0;
  if (flags & FL_SIGNED) {
    if (val < 0.0) {
      sign = '-';
      val = -val;
      width--;
    } else if (flags & FL_PLUS) {
      sign = '+';
      width--;
    } else if (flags & FL_SPACE) {
      sign = ' ';
      width--;
    }
  }

  if (prec < 0)
    prec = 6;
  else if (prec == 0 && fmt == 'g')
    prec = 1;

  cfltcvt(val, tmp, fmt, prec);

  if ((flags & FL_HASH) && prec == 0)
    forcdecpt(tmp);

  if (fmt == 'g' && !(flags & FL_HASH))
    cropzeros(tmp);

  len = strlen(tmp);
  width -= len;

  if (!(flags & (FL_ZERO | FL_MINUS)))
    while (width-- > 0)
      EMIT(' ');

  if (sign)
    EMIT(sign);

  if (!(flags & FL_MINUS)) {
    while (width-- > 0)
      EMIT(c);
  }

  for (i = 0; i < len; i++)
    EMIT(tmp[i]);

  while (width-- > 0)
    EMIT(' ');

  return o;
}

int vsnprintf(char* buf, size_t n, const char* fmt, va_list ap) {
  const char* p = fmt;
  char ch;
  char* q = buf;
  size_t o = 0; /* Number of characters output */
  uintmax_t val = 0;
  int rank = rank_int; /* Default rank */
  int width = 0;
  int prec = -1;
  int base;
  size_t sz;
  enum flags flags = 0;
  enum {
    st_normal,    /* Ground state */
    st_flags,     /* Special flags */
    st_width,     /* Field width */
    st_prec,      /* Field precision */
    st_modifiers, /* Length or conversion modifiers */
  } state = st_normal;
  const char* sarg; /* %s string argument */
  char carg;        /* %c char argument */
  int slen;         /* String length */

  while ((ch = *p++)) {
    switch (state) {
      case st_normal:
        if (ch == '%') {
          state = st_flags;
          flags = 0;
          rank = rank_int;
          width = 0;
          prec = -1;
        } else {
          EMIT(ch);
        }
        break;

      case st_flags:
        switch (ch) {
          case '-':
            flags |= FL_MINUS;
            break;
          case '+':
            flags |= FL_PLUS;
            break;
          case '\'':
            flags |= FL_TICK;
            break;
          case ' ':
            flags |= FL_SPACE;
            break;
          case '#':
            flags |= FL_HASH;
            break;
          case '0':
            flags |= FL_ZERO;
            break;
          default:
            state = st_width;
            p--; /* Process this character again */
            break;
        }
        break;

      case st_width:
        if (ch >= '0' && ch <= '9') {
          width = width * 10 + (ch - '0');
        } else if (ch == '*') {
          width = va_arg(ap, int);
          if (width < 0) {
            width = -width;
            flags |= FL_MINUS;
          }
        } else if (ch == '.') {
          prec = 0; /* Precision given */
          state = st_prec;
        } else {
          state = st_modifiers;
          p--; /* Process this character again */
        }
        break;

      case st_prec:
        if (ch >= '0' && ch <= '9') {
          prec = prec * 10 + (ch - '0');
        } else if (ch == '*') {
          prec = va_arg(ap, int);
          if (prec < 0)
            prec = -1;
        } else {
          state = st_modifiers;
          p--; /* Process this character again */
        }
        break;

      case st_modifiers:
        switch (ch) {
          /*
           * Length modifiers - nonterminal sequences
           */
          case 'h':
            rank--; /* Shorter rank */
            break;
          case 'l':
            rank++; /* Longer rank */
            break;
          case 'j':
            rank = INTMAX_RANK;
            break;
          case 'z':
            rank = SIZE_T_RANK;
            break;
          case 't':
            rank = PTRDIFF_T_RANK;
            break;
          case 'L':
          case 'q':
            rank += 2;
            break;
          default:
            /*
             * Next state will be normal
             */
            state = st_normal;

            /*
             * Canonicalize rank
             */
            if (rank < MIN_RANK)
              rank = MIN_RANK;
            else if (rank > MAX_RANK)
              rank = MAX_RANK;

            switch (ch) {
              case 'P': /* Upper case pointer */
                flags |= FL_UPPER;
                break;
              case 'p': /* Pointer */
                base = 16;
                prec = (8 * sizeof(void*) + 3) / 4;
                flags |= FL_HASH;
                val = (uintmax_t)(uintptr_t)va_arg(ap, void*);
                goto is_integer;

              case 'd': /* Signed decimal output */
              case 'i':
                base = 10;
                flags |= FL_SIGNED;
                switch (rank) {
                  case rank_char:
                    /* Yes, all these casts are needed */
                    val = (uintmax_t)(intmax_t)(signed char)va_arg(ap,
                                                                   signed int);
                    break;
                  case rank_short:
                    val = (uintmax_t)(intmax_t)(signed short)va_arg(ap,
                                                                    signed int);
                    break;
                  case rank_int:
                    val = (uintmax_t)(intmax_t)va_arg(ap, signed int);
                    break;
                  case rank_long:
                    val = (uintmax_t)(intmax_t)va_arg(ap, signed long);
                    break;
                  case rank_longlong:
                    val = (uintmax_t)(intmax_t)va_arg(ap, signed long long);
                    break;
                }
                goto is_integer;
              case 'o': /* Octal */
                base = 8;
                goto is_unsigned;
              case 'u': /* Unsigned decimal */
                base = 10;
                goto is_unsigned;
              case 'X': /* Upper case hexadecimal */
                flags |= FL_UPPER;
                base = 16;
                goto is_unsigned;
              case 'x': /* Hexadecimal */
                base = 16;
                goto is_unsigned;

              is_unsigned:
                switch (rank) {
                  case rank_char:
                    val = (uintmax_t)(unsigned char)va_arg(ap, unsigned int);
                    break;
                  case rank_short:
                    val = (uintmax_t)(unsigned short)va_arg(ap, unsigned int);
                    break;
                  case rank_int:
                    val = (uintmax_t)va_arg(ap, unsigned int);
                    break;
                  case rank_long:
                    val = (uintmax_t)va_arg(ap, unsigned long);
                    break;
                  case rank_longlong:
                    val = (uintmax_t)va_arg(ap, unsigned long long);
                    break;
                }

              is_integer:
                sz = format_int(q, (o < n) ? n - o : 0, val, flags, base, width,
                                prec);
                q += sz;
                o += sz;
                break;

              case 'c': /* Character */
                carg = (char)va_arg(ap, int);
                sarg = &carg;
                slen = 1;
                goto is_string;
              case 's': /* String */
                sarg = va_arg(ap, const char*);
                sarg = sarg ? sarg : "(null)";
                slen = strlen(sarg);
                goto is_string;

              is_string : {
                char sch;
                int i;

                if (prec != -1 && slen > prec)
                  slen = prec;

                if (width > slen && !(flags & FL_MINUS)) {
                  char pad = (flags & FL_ZERO) ? '0' : ' ';
                  while (width > slen) {
                    EMIT(pad);
                    width--;
                  }
                }
                for (i = slen; i; i--) {
                  sch = *sarg++;
                  EMIT(sch);
                }
                if (width > slen && (flags & FL_MINUS)) {
                  while (width > slen) {
                    EMIT(' ');
                    width--;
                  }
                }
              } break;

              case 'n': {
                /*
                 * Output the number of characters written
                 */
                switch (rank) {
                  case rank_char:
                    *va_arg(ap, signed char*) = o;
                    break;
                  case rank_short:
                    *va_arg(ap, signed short*) = o;
                    break;
                  case rank_int:
                    *va_arg(ap, signed int*) = o;
                    break;
                  case rank_long:
                    *va_arg(ap, signed long*) = o;
                    break;
                  case rank_longlong:
                    *va_arg(ap, signed long long*) = o;
                    break;
                }
              } break;

              case 'E':
              case 'G':
              case 'e':
              case 'f':
              case 'g':
                sz = format_float(q, (o < n) ? n - o : 0,
                                  (double)(va_arg(ap, double)), flags, ch,
                                  width, prec);
                q += sz;
                o += sz;
                break;

              default: /* Anything else, including % */
                EMIT(ch);
                break;
            }
            break;
        }
        break;
    }
  }

  /*
   * Null-terminate the string
   */
  if (o < n)
    *q = '\0'; /* No overflow */
  else if (n > 0)
    buf[n - 1] = '\0'; /* Overflow - terminate at end of buffer */

  return o;
}
int vsprintf(char* buf, const char* fmt, va_list args) {
  int rv = vsnprintf(buf, ~(size_t)0, fmt, args);
  return rv;
}
// sprintf
int sprintf(char* buf, const char* fmt, ...) {
  va_list ap;
  int rv;

  va_start(ap, fmt);
  rv = vsnprintf(buf, ~(size_t)0, fmt, ap);
  va_end(ap);

  return rv;
}
void strrev(char* s) {
  if (NULL == s)
    return;

  char* pBegin = s;
  char* pEnd = s + strlen(s) - 1;

  char pTemp;

  while (pBegin < pEnd) {
    pTemp = *pBegin;
    *pBegin = *pEnd;
    *pEnd = pTemp;

    ++pBegin, --pEnd;
  }
}
int printf(const char* format, ...) {
  va_list ap;
  int rv;
  char buf[1000];
  va_start(ap, format);
  rv = vsnprintf(buf, ~(size_t)0, format, ap);
  puts(buf);
  va_end(ap);

  return rv;
}
void F2S(double d, char* str, int l) {}
char* strchr(const char* s, int c) {
  char* p = s;
  while (*p && *p != c) {
    p++;
  }
  if (*p == c) {
    return p;
  }
  return NULL;
}
char* strrchr(const char* s1, int ch) {
  char* s2;
  char* s3;
  s2 = strchr(s1, ch);
  while (s2 != NULL) {
    s3 = strchr(s2 + 1, ch);
    if (s3 != NULL) {
      s2 = s3;
    } else {
      return s2;
    }
  }
  return NULL;
}
void* memmove(void* dest, const void* src, int n) {
  /*因为char类型为1字节，所以将数据转化为char*
  进行操作。并不是因为操作的对象是字符串*/
  char* pdest = (char*)dest;
  const char* psrc = (const char*)src;
  if (pdest <= psrc && pdest >= psrc + n)  //正常情况下从前向后拷贝
  {
    while (n--) {
      *pdest = *psrc;
    }
  } else  //当出现内存覆盖时从后向前拷贝
  {
    while (n--) {
      *(pdest + n) = *(psrc + n);
    }
  }
  return dest;
}

#define FREE_MAX_NUM 4096
#define ERRNO_NOPE 0
#define ERRNO_NO_ENOGHT_MEMORY 1
#define ERRNO_NO_MORE_FREE_MEMBER 2
#define MEM_MAX(a, b) (a) > (b) ? (a) : (b)
typedef struct {
  uint32_t start;
  uint32_t end;  // end和start都等于0说明这个free结构没有使用
} free_member;
typedef struct freeinfo freeinfo;
typedef struct freeinfo {
  free_member* f;
  freeinfo* next;
} freeinfo;
typedef struct {
  freeinfo* freeinf;
  int memerrno;
} memory;
memory* mm;
char* alloc_start;
uint32_t total_size;
void swap(free_member* a, free_member* b) {
  free_member temp = *a;
  *a = *b;
  *b = temp;
}
int cmp(free_member a, free_member b) {
  return a.start <= b.start;
}
int partition(free_member* arr, int low, int high) {
  free_member pivot = arr[high];
  int i = (low - 1);

  for (int j = low; j <= high - 1; j++) {
    if (cmp(arr[j], pivot)) {
      i++;
      swap(&arr[i], &arr[j]);
    }
  }

  swap(&arr[i + 1], &arr[high]);
  return (i + 1);
}

void quicksort(free_member* arr, int low, int high) {
  if (low < high) {
    int pi = partition(arr, low, high);
    quicksort(arr, low, pi - 1);
    quicksort(arr, pi + 1, high);
  }
}
freeinfo* make_next_freeinfo(memory* mem) {
  const int size = FREE_MAX_NUM * sizeof(free_member) + sizeof(freeinfo);
  freeinfo* fi = NULL;
  freeinfo* finf = mem->freeinf;
  freeinfo* old = NULL;
  uint32_t s, n;
  while (finf) {
    old = finf;
    for (int i = 0; i < FREE_MAX_NUM; i++) {
      if (finf->f[i].start + finf->f[i].end == 0) {
        break;
      }
      if (finf->f[i].end - finf->f[i].start >= size) {
        uint32_t start = finf->f[i].start;
        s = finf->f[i].start;
        n = finf->f[i].end;
        mem_delete(i, finf);
        fi = (freeinfo*)start;
        break;
      }
    }
    if (fi) {
      break;
    }
    finf = finf->next;
  }
  if (!fi) {
    mem->memerrno = ERRNO_NO_ENOGHT_MEMORY;
    return NULL;
  }
  fi->next = 0;
  while (finf) {
    old = finf;
    finf = finf->next;
  }
  old->next = fi;
  fi->f = (free_member*)((uint32_t)fi + sizeof(freeinfo));
  for (int i = 0; i < FREE_MAX_NUM; i++) {
    fi->f[i].start = 0;
    fi->f[i].end = 0;
  }

  if (n - s > size) {
    mem_free_finf(mem, fi, s + size, n - s - size);  // 一点也不浪费
  }

  return fi;
}
free_member* mem_insert(int pos, freeinfo* finf) {
  int j = 0;
  for (int i = 0; i < FREE_MAX_NUM; i++) {
    if (finf->f[i].start + finf->f[i].end != 0) {
      ++j;
    }
  }
  if (j == FREE_MAX_NUM) {
    return NULL;
  }
  for (int i = j - 1; i >= pos; i--) {
    finf->f[i + 1] = finf->f[i];
  }
  return &(finf->f[pos]);
}
free_member* mem_add(freeinfo* finf) {
  int j = 0;
  for (int i = 0; i < FREE_MAX_NUM; i++) {
    if (finf->f[i].start + finf->f[i].end != 0) {
      ++j;
    }
  }
  if (j == FREE_MAX_NUM) {
    return NULL;
  }
  return &(finf->f[j]);
}
void mem_delete(int pos, freeinfo* finf) {
  int i;
  for (i = pos; i < FREE_MAX_NUM - 1; i++) {
    if (finf->f[i].start == 0 && finf->f[i].end == 0) {
      return;
    }
    finf->f[i] = finf->f[i + 1];
  }
  finf->f[i].start = 0;
  finf->f[i].end = 0;
}
uint32_t mem_get_all_finf(freeinfo* finf) {
  for (int i = 0; i < FREE_MAX_NUM; i++) {
    if (finf->f[i].start + finf->f[i].end == 0) {
      return i;
    }
  }
  return FREE_MAX_NUM;
}
// 内存整理
void mem_defragmenter(freeinfo* finf) {
  for (int i = 0; i < FREE_MAX_NUM - 1; i++) {
    if (finf->f[i].start + finf->f[i].end == 0) {
      break;
    }
    if (finf->f[i].end == finf->f[i + 1].start) {
      int end = finf->f[i + 1].end;
      mem_delete(i + 1, finf);
      finf->f[i].end = end;
    }
    if (finf->f[i + 1].start == finf->f[i].start) {
      int end = MEM_MAX(finf->f[i].end, finf->f[i + 1].end);
      mem_delete(i + 1, finf);
      finf->f[i].end = end;
    }
    if (finf->f[i + 1].start < finf->f[i].end) {
      int end = MEM_MAX(finf->f[i].end, finf->f[i + 1].end);
      mem_delete(i + 1, finf);
      finf->f[i].end = end;
    }
  }
}
// 内存分割
uint32_t mem_partition(freeinfo* finf,
                       uint32_t start,
                       uint32_t end,
                       uint32_t index) {
  if (finf->f[index].start == start && finf->f[index].end == end) {
    return index;
  }
  int times = 0;
  int result = 0;
  if (finf->f[index].start == start || finf->f[index].end == end) {
    times = 1;
  } else if (finf->f[index].start < start) {
    times = 2;
  }
  if (times == 0) {
    printf("bug\n");
    for (;;)
      ;
  }
  switch (times) {
    case 1:
      if (finf->f[index].start == start) {
        uint32_t i = finf->f[index].start;
        uint32_t j = finf->f[index].end;
        free_member* f = mem_insert(index, finf);
        if (!f) {
          return -1;
        }
        f->start = start;
        f->end = end;
        finf->f[index + 1].start = end;
        finf->f[index + 1].end = j;
        result = index;
      } else if (finf->f[index].end == end) {
        int i = finf->f[index].start;
        int j = finf->f[index].end;
        free_member* f = mem_insert(index, finf);
        if (!f) {
          return -1;
        }
        f->start = i;
        f->end = start;
        finf->f[index + 1].start = start;
        finf->f[index + 1].end = end;
        result = index + 1;
      }
      return result;
    case 2: {
      int i = finf->f[index].start;
      int j = finf->f[index].end;
      free_member* f;
      f = mem_insert(index, finf);
      if (!f) {
        return -1;
      }
      f = mem_insert(index, finf);
      if (!f) {
        mem_delete(index, finf);  // 上面成功了，所以要删掉
        return -1;
      }
      f->start = i;
      f->end = start;
      finf->f[index + 1].start = start;
      finf->f[index + 1].end = end;
      finf->f[index + 2].start = end;
      finf->f[index + 2].end = j;
      return index;
    }
    default:
      // the code mustn't exec here
      break;
  }
}
int mem_free_finf(memory* mem, freeinfo* finf, void* p, uint32_t size) {
  free_member* f = mem_add(finf);
  if (!f) {
    int flag = 0;
    for (int i = 0; i < FREE_MAX_NUM - 1; i++) {
      if (finf->f[i].start + finf->f[i].end == 0) {
        break;
      }
      if (finf->f[i].end == (uint32_t)p) {
        flag = 1;
        int end = (uint32_t)p + size;
        finf->f[i].end = end;
      } else if ((uint32_t)p == finf->f[i].start) {
        flag = 1;
        int end = MEM_MAX(finf->f[i].end, (uint32_t)p + size);
        finf->f[i].end = end;
      } else if ((uint32_t)p < finf->f[i].end) {
        flag = 1;
        int end = MEM_MAX(finf->f[i].end, (uint32_t)p + size);
        finf->f[i].end = end;
      }
    }
    if (flag) {
      mem_defragmenter(finf);
      mem->memerrno = ERRNO_NOPE;
      return 1;
    } else {
      mem->memerrno = ERRNO_NO_MORE_FREE_MEMBER;
      return 0;
    }
  }
  mem->memerrno = ERRNO_NOPE;
  f->start = (uint32_t)p;
  f->end = (uint32_t)p + size;
  quicksort(finf->f, 0, mem_get_all_finf(finf) - 1);
  mem_defragmenter(finf);
  return 1;
}
void* mem_alloc_finf(memory* mem,
                     freeinfo* finf,
                     uint32_t size,
                     freeinfo* if_nomore) {
  for (int i = 0; i < FREE_MAX_NUM; i++) {
    if (finf->f[i].start == 0 && finf->f[i].end == 0) {
      break;
    }
    if (finf->f[i].end - finf->f[i].start >= size) {
      uint32_t start = finf->f[i].start;
      uint32_t idx = mem_partition(finf, start, start + size, i);
      if (idx == -1) {
        if (if_nomore) {
          if (FREE_MAX_NUM - mem_get_all_finf(if_nomore) >= 1) {
            int e = finf->f[i].end;
            mem_delete(i, finf);
            mem_defragmenter(finf);
            mem_free_finf(mem, finf, (void*)start, e - start);
            return mem_alloc_finf(mem, if_nomore, size, NULL);
          }
        }
        mem->memerrno = ERRNO_NO_MORE_FREE_MEMBER;
        mem_defragmenter(finf);
        return NULL;
      }
      mem->memerrno = ERRNO_NOPE;
      mem_delete(idx, finf);
      mem_defragmenter(finf);
      memset(start, 0, size);
      return (void*)start;
    }
  }
  mem->memerrno = ERRNO_NO_ENOGHT_MEMORY;
  return NULL;
}
void* mem_alloc(memory* mem, uint32_t size) {
  freeinfo* finf = mem->freeinf;
  int flag = 0;
  freeinfo* if_nomore = NULL;
  while (finf) {
    if (flag && !if_nomore) {
      break;
      ;
    }
    void* result = mem_alloc_finf(mem, finf, size, if_nomore);
    if (mem->memerrno != ERRNO_NOPE) {
      if (mem->memerrno == ERRNO_NO_MORE_FREE_MEMBER) {
        if (!flag) {
          if_nomore = finf;
          flag = 1;
        }
      }
    } else {
      return result;
    }
    if (flag) {
      if_nomore = if_nomore->next;
    } else {
      finf = finf->next;
    }
  }
  if (flag) {
    freeinfo* new_f = make_next_freeinfo(mem);
    if (!new_f) {
      return NULL;
    }
    return mem_alloc(mem, size);
  }
  return NULL;
}
void mem_free(memory* mem, void* p, uint32_t size) {
  freeinfo* finf = mem->freeinf;
  while (finf) {
    if (mem_free_finf(mem, finf, p, size)) {
      return;
    }
    finf = finf->next;
  }
  freeinfo* new_f = make_next_freeinfo(mem);
  if (new_f) {
    mem_free_finf(mem, new_f, p, size);
  }
}
void show_mem(memory* mem) {
  freeinfo* finf = mem->freeinf;
  while (finf) {
    for (int i = 0; i < FREE_MAX_NUM; i++) {
      if (finf->f[i].start == 0 && finf->f[i].end == 0) {
        break;
      }
      printf("START: %08x END: %08x SIZE: %08x Bytes\n", finf->f[i].start,
             finf->f[i].end, finf->f[i].end - finf->f[i].start);
    }
    finf = finf->next;
  }
}
void* mm_alloc(uint32_t size) {
  unsigned int a;
retry:
  a = mem_alloc(mm, size);
  if (!a) {
    sbrk(size);
    mem_free(mm, alloc_start + total_size, size);
    total_size += size;
    goto retry;
  }
  return a;
}
void mm_free(uint32_t addr, uint32_t size) {
  int i;
  mem_free(mm, addr, size);
}
void* mem_alloc_nb(memory* mem, uint32_t size, uint32_t n) {
  size = ((size - 1) / n + 1) * n;
  return mm_alloc(size);
}
void mem_free_nb(memory* mem, void* p, uint32_t size, uint32_t n) {
  size = ((size - 1) / n + 1) * n;
  mm_free(p, size);
}
memory* memory_init(uint32_t start, uint32_t size) {
  memory* mem;
  mem = (memory*)start;
  start += sizeof(memory);
  size -= sizeof(memory);
  if (size < 0) {
    printf("mm init error.\n");
    for (;;)
      ;
  }
  mem->freeinf = (freeinfo*)start;
  start += sizeof(freeinfo);
  size -= sizeof(freeinfo);
  if (size < 0) {
    printf("mm init error.\n");
    for (;;)
      ;
  }
  mem->freeinf->next = 0;
  mem->freeinf->f = (free_member*)start;
  start += FREE_MAX_NUM * sizeof(free_member);
  size -= FREE_MAX_NUM * sizeof(free_member);
  if ((int)size < 0) {
    printf("mm init error.\n");
    for (;;)
      ;
  }
  for (int i = 0; i < FREE_MAX_NUM; i++) {
    mem->freeinf->f[i].start = 0;
    mem->freeinf->f[i].end = 0;
  }
  mem_free(mem, (void*)start, size);
  return mem;
}
char flag = 0;
void init_mem() {
  alloc_start = api_malloc(1);
  mm = memory_init(alloc_start, api_heapsize());
  total_size = api_heapsize();
  //  printf("total size = %08x\n",total_size);
  flag = 1;
}
void* malloc(int size) {
  // printf("malloc.\n");
  if (flag) {
    void* p = mem_alloc_nb(mm, size + sizeof(int), 512);
    *(int*)p = size;
    return (char*)p + sizeof(int);
  } else {
    return __builtin_alloca(size);
  }
}
void free(void* p) {
  if (p == NULL)
    return;
  int size = *(int*)((char*)p - sizeof(int));
  mem_free_nb(mm, (char*)p - sizeof(int), size + sizeof(int), 512);
}
int _Znaj(uint32_t size) {
  // printk("_Znaj:%d\n", size);
  return malloc(size);
}
void _ZdaPv(void* ptr) {
  // printk("_ZdaPv:%08x\n", ptr);
  free(ptr);
}
//_ZdlPvj
void _ZdlPvj(void* ptr, uint32_t size) {
  // printk("_ZdlPvj %08x %d\n", ptr, size);
  free(ptr);
}
//_Znwj
void* _Znwj(uint32_t size) {
  // printk("_Znwj:%d\n", size);
  return malloc(size);
}
void* realloc(void* ptr, uint32_t size) {
  void* new = malloc(size);
  if (ptr) {
    memcpy(new, ptr, *(int*)((int)ptr - 4));
    free(ptr);
  }
  return new;
}
double scalbn(double x, int n) {
  union {
    double f;
    uint64_t i;
  } u;
  double_t y = x;

  if (n > 1023) {
    y *= 0x1p1023;
    n -= 1023;
    if (n > 1023) {
      y *= 0x1p1023;
      n -= 1023;
      if (n > 1023)
        n = 1023;
    }
  } else if (n < -1022) {
    y *= 0x1p-1022;
    n += 1022;
    if (n < -1022) {
      y *= 0x1p-1022;
      n += 1022;
      if (n < -1022)
        n = -1022;
    }
  }
  u.i = (uint64_t)(0x3ff + n) << 52;
  x = y * u.f;
  return x;
}
static const double pio2_hi =
                        1.57079632679489655800e+00, /* 0x3FF921FB, 0x54442D18 */
    pio2_lo = 6.12323399573676603587e-17,           /* 0x3C91A626, 0x33145C07 */
    /* coefficients for R(x^2) */
    pS0 = 1.66666666666666657415e-01,  /* 0x3FC55555, 0x55555555 */
    pS1 = -3.25565818622400915405e-01, /* 0xBFD4D612, 0x03EB6F7D */
    pS2 = 2.01212532134862925881e-01,  /* 0x3FC9C155, 0x0E884455 */
    pS3 = -4.00555345006794114027e-02, /* 0xBFA48228, 0xB5688F3B */
    pS4 = 7.91534994289814532176e-04,  /* 0x3F49EFE0, 0x7501B288 */
    pS5 = 3.47933107596021167570e-05,  /* 0x3F023DE1, 0x0DFDF709 */
    qS1 = -2.40339491173441421878e+00, /* 0xC0033A27, 0x1C8A2D4B */
    qS2 = 2.02094576023350569471e+00,  /* 0x40002AE5, 0x9C598AC8 */
    qS3 = -6.88283971605453293030e-01, /* 0xBFE6066C, 0x1B8D0159 */
    qS4 = 7.70381505559019352791e-02;  /* 0x3FB3B8C5, 0xB12E9282 */

static double R(double z) {
  double_t p, q;
  p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
  q = 1.0 + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
  return p / q;
}

double asin(double x) {
  double z, r, s;
  uint32_t hx, ix;

  GET_HIGH_WORD(hx, x);
  ix = hx & 0x7fffffff;
  /* |x| >= 1 or nan */
  if (ix >= 0x3ff00000) {
    uint32_t lx;
    GET_LOW_WORD(lx, x);
    if (((ix - 0x3ff00000) | lx) == 0)
      /* asin(1) = +-pi/2 with inexact */
      return x * pio2_hi + 0x1p-120f;
    return 0 / (x - x);
  }
  /* |x| < 0.5 */
  if (ix < 0x3fe00000) {
    /* if 0x1p-1022 <= |x| < 0x1p-26, avoid raising underflow */
    if (ix < 0x3e500000 && ix >= 0x00100000)
      return x;
    return x + x * R(x * x);
  }
  /* 1 > |x| >= 0.5 */
  z = (1 - fabs(x)) * 0.5;
  s = sqrt(z);
  r = R(z);
  if (ix >= 0x3fef3333) { /* if |x| > 0.975 */
    x = pio2_hi - (2 * (s + s * r) - pio2_lo);
  } else {
    double f, c;
    /* f+c = sqrt(z) */
    f = s;
    SET_LOW_WORD(f, 0);
    c = (z - f * f) / (s + f);
    x = 0.5 * pio2_hi -
        (2 * s * r - (pio2_lo - 2 * c) - (0.5 * pio2_hi - 2 * f));
  }
  if (hx >> 31)
    return -x;
  return x;
}
static const double atanhi[] = {
    4.63647609000806093515e-01, /* atan(0.5)hi 0x3FDDAC67, 0x0561BB4F */
    7.85398163397448278999e-01, /* atan(1.0)hi 0x3FE921FB, 0x54442D18 */
    9.82793723247329054082e-01, /* atan(1.5)hi 0x3FEF730B, 0xD281F69B */
    1.57079632679489655800e+00, /* atan(inf)hi 0x3FF921FB, 0x54442D18 */
};

static const double atanlo[] = {
    2.26987774529616870924e-17, /* atan(0.5)lo 0x3C7A2B7F, 0x222F65E2 */
    3.06161699786838301793e-17, /* atan(1.0)lo 0x3C81A626, 0x33145C07 */
    1.39033110312309984516e-17, /* atan(1.5)lo 0x3C700788, 0x7AF0CBBD */
    6.12323399573676603587e-17, /* atan(inf)lo 0x3C91A626, 0x33145C07 */
};

static const double aT[] = {
    3.33333333333329318027e-01,  /* 0x3FD55555, 0x5555550D */
    -1.99999999998764832476e-01, /* 0xBFC99999, 0x9998EBC4 */
    1.42857142725034663711e-01,  /* 0x3FC24924, 0x920083FF */
    -1.11111104054623557880e-01, /* 0xBFBC71C6, 0xFE231671 */
    9.09088713343650656196e-02,  /* 0x3FB745CD, 0xC54C206E */
    -7.69187620504482999495e-02, /* 0xBFB3B0F2, 0xAF749A6D */
    6.66107313738753120669e-02,  /* 0x3FB10D66, 0xA0D03D51 */
    -5.83357013379057348645e-02, /* 0xBFADDE2D, 0x52DEFD9A */
    4.97687799461593236017e-02,  /* 0x3FA97B4B, 0x24760DEB */
    -3.65315727442169155270e-02, /* 0xBFA2B444, 0x2C6A6C2F */
    1.62858201153657823623e-02,  /* 0x3F90AD3A, 0xE322DA11 */
};

double atan(double x) {
  double_t w, s1, s2, z;
  uint32_t ix, sign;
  int id;

  GET_HIGH_WORD(ix, x);
  sign = ix >> 31;
  ix &= 0x7fffffff;
  if (ix >= 0x44100000) { /* if |x| >= 2^66 */
    if (isnan(x))
      return x;
    z = atanhi[3] + 0x1p-120f;
    return sign ? -z : z;
  }
  if (ix < 0x3fdc0000) {   /* |x| < 0.4375 */
    if (ix < 0x3e400000) { /* |x| < 2^-27 */
      if (ix < 0x00100000)
        /* raise underflow for subnormal x */
        FORCE_EVAL((float)x);
      return x;
    }
    id = -1;
  } else {
    x = fabs(x);
    if (ix < 0x3ff30000) {   /* |x| < 1.1875 */
      if (ix < 0x3fe60000) { /*  7/16 <= |x| < 11/16 */
        id = 0;
        x = (2.0 * x - 1.0) / (2.0 + x);
      } else { /* 11/16 <= |x| < 19/16 */
        id = 1;
        x = (x - 1.0) / (x + 1.0);
      }
    } else {
      if (ix < 0x40038000) { /* |x| < 2.4375 */
        id = 2;
        x = (x - 1.5) / (1.0 + 1.5 * x);
      } else { /* 2.4375 <= |x| < 2^66 */
        id = 3;
        x = -1.0 / x;
      }
    }
  }
  /* end of argument reduction */
  z = x * x;
  w = z * z;
  /* break sum from i=0 to 10 aT[i]z**(i+1) into odd and even poly */
  s1 = z * (aT[0] +
            w * (aT[2] + w * (aT[4] + w * (aT[6] + w * (aT[8] + w * aT[10])))));
  s2 = w * (aT[1] + w * (aT[3] + w * (aT[5] + w * (aT[7] + w * aT[9]))));
  if (id < 0)
    return x - x * (s1 + s2);
  z = atanhi[id] - (x * (s1 + s2) - atanlo[id] - x);
  return sign ? -z : z;
}
double scalbln(double x, long n) {
  if (n > INT_MAX)
    n = INT_MAX;
  else if (n < INT_MIN)
    n = INT_MIN;
  return scalbn(x, n);
}
double ldexp(double x, int n) {
  return scalbn(x, n);
}

static const double pi = 3.1415926535897931160E+00, /* 0x400921FB, 0x54442D18 */
    pi_lo = 1.2246467991473531772E-16;              /* 0x3CA1A626, 0x33145C07 */

double atan2(double y, double x) {
  double z;
  uint32_t m, lx, ly, ix, iy;

  if (isnan(x) || isnan(y))
    return x + y;
  EXTRACT_WORDS(ix, lx, x);
  EXTRACT_WORDS(iy, ly, y);
  if (((ix - 0x3ff00000) | lx) == 0) /* x = 1.0 */
    return atan(y);
  m = ((iy >> 31) & 1) | ((ix >> 30) & 2); /* 2*sign(x)+sign(y) */
  ix = ix & 0x7fffffff;
  iy = iy & 0x7fffffff;

  /* when y = 0 */
  if ((iy | ly) == 0) {
    switch (m) {
      case 0:
      case 1:
        return y; /* atan(+-0,+anything)=+-0 */
      case 2:
        return pi; /* atan(+0,-anything) = pi */
      case 3:
        return -pi; /* atan(-0,-anything) =-pi */
    }
  }
  /* when x = 0 */
  if ((ix | lx) == 0)
    return m & 1 ? -pi / 2 : pi / 2;
  /* when x is INF */
  if (ix == 0x7ff00000) {
    if (iy == 0x7ff00000) {
      switch (m) {
        case 0:
          return pi / 4; /* atan(+INF,+INF) */
        case 1:
          return -pi / 4; /* atan(-INF,+INF) */
        case 2:
          return 3 * pi / 4; /* atan(+INF,-INF) */
        case 3:
          return -3 * pi / 4; /* atan(-INF,-INF) */
      }
    } else {
      switch (m) {
        case 0:
          return 0.0; /* atan(+...,+INF) */
        case 1:
          return -0.0; /* atan(-...,+INF) */
        case 2:
          return pi; /* atan(+...,-INF) */
        case 3:
          return -pi; /* atan(-...,-INF) */
      }
    }
  }
  /* |y/x| > 0x1p64 */
  if (ix + (64 << 20) < iy || iy == 0x7ff00000)
    return m & 1 ? -pi / 2 : pi / 2;

  /* z = atan(|y/x|) without spurious underflow */
  if ((m & 2) && iy + (64 << 20) < ix) /* |y/x| < 0x1p-64, x<0 */
    z = 0;
  else
    z = atan(fabs(y / x));
  switch (m) {
    case 0:
      return z; /* atan(+,+) */
    case 1:
      return -z; /* atan(-,+) */
    case 2:
      return pi - (z - pi_lo); /* atan(+,-) */
    default:                   /* case 3 */
      return (z - pi_lo) - pi; /* atan(-,-) */
  }
}
#define EPS (2.22044604925031308085e-16)
static const double_t toint = 1 / EPS;

double floor(double x) {
  int flag = 1;
  if (fabs(x) == x) {
    flag = 0;
  }
  if (flag) {
    double r = (x);  // 如果小于，则返回x-1的整数部分
    double s;
    modf(r, &s);
    return s - 1;
  } else {
    double r = (x);  // 如果大于等于，则返回x的整数部分
    double s;
    modf(r, &s);
    return s;
  }
}
double ceil(double x) {
  int flag = 0;
  if (fabs(x) == x) {
    flag = 1;
  }
  if (flag) {
    double r = (x);  // 如果小于，则返回x-1的整数部分
    double s;
    modf(r, &s);
    return s + 1;
  } else {
    double r = (x);  // 如果大于等于，则返回x的整数部分
    double s;
    modf(r, &s);
    return s;
  }
}
static const double bp[] =
    {
        1.0,
        1.5,
},
                    dp_h[] =
                        {
                            0.0,
                            5.84962487220764160156e-01,
}, /* 0x3FE2B803, 0x40000000 */
    dp_l[] =
        {
            0.0,
            1.35003920212974897128e-08,
},                              /* 0x3E4CFDEB, 0x43CFD006 */
    two53 = 9007199254740992.0, /* 0x43400000, 0x00000000 */
    huge = 1.0e300, tiny = 1.0e-300,
                    /* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
    L1 = 5.99999999999994648725e-01,     /* 0x3FE33333, 0x33333303 */
    L2 = 4.28571428578550184252e-01,     /* 0x3FDB6DB6, 0xDB6FABFF */
    L3 = 3.33333329818377432918e-01,     /* 0x3FD55555, 0x518F264D */
    L4 = 2.72728123808534006489e-01,     /* 0x3FD17460, 0xA91D4101 */
    L5 = 2.30660745775561754067e-01,     /* 0x3FCD864A, 0x93C9DB65 */
    L6 = 2.06975017800338417784e-01,     /* 0x3FCA7E28, 0x4A454EEF */
    P1 = 1.66666666666666019037e-01,     /* 0x3FC55555, 0x5555553E */
    P2 = -2.77777777770155933842e-03,    /* 0xBF66C16C, 0x16BEBD93 */
    P3 = 6.61375632143793436117e-05,     /* 0x3F11566A, 0xAF25DE2C */
    P4 = -1.65339022054652515390e-06,    /* 0xBEBBBD41, 0xC5D26BF1 */
    P5 = 4.13813679705723846039e-08,     /* 0x3E663769, 0x72BEA4D0 */
    lg2 = 6.93147180559945286227e-01,    /* 0x3FE62E42, 0xFEFA39EF */
    lg2_h = 6.93147182464599609375e-01,  /* 0x3FE62E43, 0x00000000 */
    lg2_l = -1.90465429995776804525e-09, /* 0xBE205C61, 0x0CA86C39 */
    ovt = 8.0085662595372944372e-017,    /* -(1024-(ovfl+.5ulp)) */
    cp = 9.61796693925975554329e-01,     /* 0x3FEEC709, 0xDC3A03FD =2/(3ln2) */
    cp_h = 9.61796700954437255859e-01,   /* 0x3FEEC709, 0xE0000000 =(float)cp */
    cp_l =
        -7.02846165095275826516e-09, /* 0xBE3E2FE0, 0x145B01F5 =tail of cp_h*/
    ivln2 = 1.44269504088896338700e+00,   /* 0x3FF71547, 0x652B82FE =1/ln2 */
    ivln2_h = 1.44269502162933349609e+00, /* 0x3FF71547, 0x60000000 =24b 1/ln2*/
    ivln2_l =
        1.92596299112661746887e-08; /* 0x3E54AE0B, 0xF85DDF44 =1/ln2 tail*/

double pow(double x, double y) {
  double z, ax, z_h, z_l, p_h, p_l;
  double y1, t1, t2, r, s, t, u, v, w;
  int32_t i, j, k, yisint, n;
  int32_t hx, hy, ix, iy;
  uint32_t lx, ly;

  EXTRACT_WORDS(hx, lx, x);
  EXTRACT_WORDS(hy, ly, y);
  ix = hx & 0x7fffffff;
  iy = hy & 0x7fffffff;

  /* x**0 = 1, even if x is NaN */
  if ((iy | ly) == 0)
    return 1.0;
  /* 1**y = 1, even if y is NaN */
  if (hx == 0x3ff00000 && lx == 0)
    return 1.0;
  /* NaN if either arg is NaN */
  if (ix > 0x7ff00000 || (ix == 0x7ff00000 && lx != 0) || iy > 0x7ff00000 ||
      (iy == 0x7ff00000 && ly != 0))
    return x + y;

  /* determine if y is an odd int when x < 0
   * yisint = 0       ... y is not an integer
   * yisint = 1       ... y is an odd int
   * yisint = 2       ... y is an even int
   */
  yisint = 0;
  if (hx < 0) {
    if (iy >= 0x43400000)
      yisint = 2; /* even integer y */
    else if (iy >= 0x3ff00000) {
      k = (iy >> 20) - 0x3ff; /* exponent */
      if (k > 20) {
        uint32_t j = ly >> (52 - k);
        if ((j << (52 - k)) == ly)
          yisint = 2 - (j & 1);
      } else if (ly == 0) {
        uint32_t j = iy >> (20 - k);
        if ((j << (20 - k)) == iy)
          yisint = 2 - (j & 1);
      }
    }
  }

  /* special value of y */
  if (ly == 0) {
    if (iy == 0x7ff00000) {              /* y is +-inf */
      if (((ix - 0x3ff00000) | lx) == 0) /* (-1)**+-inf is 1 */
        return 1.0;
      else if (ix >= 0x3ff00000) /* (|x|>1)**+-inf = inf,0 */
        return hy >= 0 ? y : 0.0;
      else /* (|x|<1)**+-inf = 0,inf */
        return hy >= 0 ? 0.0 : -y;
    }
    if (iy == 0x3ff00000) { /* y is +-1 */
      if (hy >= 0)
        return x;
      y = 1 / x;
#if FLT_EVAL_METHOD != 0
      {
        union {
          double f;
          uint64_t i;
        } u = {y};
        uint64_t i = u.i & -1ULL / 2;
        if (i >> 52 == 0 && (i & (i - 1)))
          FORCE_EVAL((float)y);
      }
#endif
      return y;
    }
    if (hy == 0x40000000) /* y is 2 */
      return x * x;
    if (hy == 0x3fe00000) { /* y is 0.5 */
      if (hx >= 0)          /* x >= +0 */
        return sqrt(x);
    }
  }

  ax = fabs(x);
  /* special value of x */
  if (lx == 0) {
    if (ix == 0x7ff00000 || ix == 0 ||
        ix == 0x3ff00000) { /* x is +-0,+-inf,+-1 */
      z = ax;
      if (hy < 0) /* z = (1/|x|) */
        z = 1.0 / z;
      if (hx < 0) {
        if (((ix - 0x3ff00000) | yisint) == 0) {
          z = (z - z) / (z - z); /* (-1)**non-int is NaN */
        } else if (yisint == 1)
          z = -z; /* (x<0)**odd = -(|x|**odd) */
      }
      return z;
    }
  }

  s = 1.0; /* sign of result */
  if (hx < 0) {
    if (yisint == 0) /* (x<0)**(non-int) is NaN */
      return (x - x) / (x - x);
    if (yisint == 1) /* (x<0)**(odd int) */
      s = -1.0;
  }

  /* |y| is huge */
  if (iy > 0x41e00000) {   /* if |y| > 2**31 */
    if (iy > 0x43f00000) { /* if |y| > 2**64, must o/uflow */
      if (ix <= 0x3fefffff)
        return hy < 0 ? huge * huge : tiny * tiny;
      if (ix >= 0x3ff00000)
        return hy > 0 ? huge * huge : tiny * tiny;
    }
    /* over/underflow if x is not close to one */
    if (ix < 0x3fefffff)
      return hy < 0 ? s * huge * huge : s * tiny * tiny;
    if (ix > 0x3ff00000)
      return hy > 0 ? s * huge * huge : s * tiny * tiny;
    /* now |1-x| is tiny <= 2**-20, suffice to compute
       log(x) by x-x^2/2+x^3/3-x^4/4 */
    t = ax - 1.0; /* t has 20 trailing zeros */
    w = (t * t) * (0.5 - t * (0.3333333333333333333333 - t * 0.25));
    u = ivln2_h * t; /* ivln2_h has 21 sig. bits */
    v = t * ivln2_l - w * ivln2;
    t1 = u + v;
    SET_LOW_WORD(t1, 0);
    t2 = v - (t1 - u);
  } else {
    double ss, s2, s_h, s_l, t_h, t_l;
    n = 0;
    /* take care subnormal number */
    if (ix < 0x00100000) {
      ax *= two53;
      n -= 53;
      GET_HIGH_WORD(ix, ax);
    }
    n += ((ix) >> 20) - 0x3ff;
    j = ix & 0x000fffff;
    /* determine interval */
    ix = j | 0x3ff00000; /* normalize ix */
    if (j <= 0x3988E)    /* |x|<sqrt(3/2) */
      k = 0;
    else if (j < 0xBB67A) /* |x|<sqrt(3)   */
      k = 1;
    else {
      k = 0;
      n += 1;
      ix -= 0x00100000;
    }
    SET_HIGH_WORD(ax, ix);

    /* compute ss = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
    u = ax - bp[k]; /* bp[0]=1.0, bp[1]=1.5 */
    v = 1.0 / (ax + bp[k]);
    ss = u * v;
    s_h = ss;
    SET_LOW_WORD(s_h, 0);
    /* t_h=ax+bp[k] High */
    t_h = 0.0;
    SET_HIGH_WORD(t_h, ((ix >> 1) | 0x20000000) + 0x00080000 + (k << 18));
    t_l = ax - (t_h - bp[k]);
    s_l = v * ((u - s_h * t_h) - s_h * t_l);
    /* compute log(ax) */
    s2 = ss * ss;
    r = s2 * s2 *
        (L1 + s2 * (L2 + s2 * (L3 + s2 * (L4 + s2 * (L5 + s2 * L6)))));
    r += s_l * (s_h + ss);
    s2 = s_h * s_h;
    t_h = 3.0 + s2 + r;
    SET_LOW_WORD(t_h, 0);
    t_l = r - ((t_h - 3.0) - s2);
    /* u+v = ss*(1+...) */
    u = s_h * t_h;
    v = s_l * t_h + t_l * ss;
    /* 2/(3log2)*(ss+...) */
    p_h = u + v;
    SET_LOW_WORD(p_h, 0);
    p_l = v - (p_h - u);
    z_h = cp_h * p_h; /* cp_h+cp_l = 2/(3*log2) */
    z_l = cp_l * p_h + p_l * cp + dp_l[k];
    /* log2(ax) = (ss+..)*2/(3*log2) = n + dp_h + z_h + z_l */
    t = (double)n;
    t1 = ((z_h + z_l) + dp_h[k]) + t;
    SET_LOW_WORD(t1, 0);
    t2 = z_l - (((t1 - t) - dp_h[k]) - z_h);
  }

  /* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
  y1 = y;
  SET_LOW_WORD(y1, 0);
  p_l = (y - y1) * t1 + y * t2;
  p_h = y1 * t1;
  z = p_l + p_h;
  EXTRACT_WORDS(j, i, z);
  if (j >= 0x40900000) {             /* z >= 1024 */
    if (((j - 0x40900000) | i) != 0) /* if z > 1024 */
      return s * huge * huge;        /* overflow */
    if (p_l + ovt > z - p_h)
      return s * huge * huge;                  /* overflow */
  } else if ((j & 0x7fffffff) >= 0x4090cc00) { /* z <= -1075 */
    if (((j - 0xc090cc00) | i) != 0)           /* z < -1075 */
      return s * tiny * tiny;                  /* underflow */
    if (p_l <= z - p_h)
      return s * tiny * tiny; /* underflow */
  }
  /*
   * compute 2**(p_h+p_l)
   */
  i = j & 0x7fffffff;
  k = (i >> 20) - 0x3ff;
  n = 0;
  if (i > 0x3fe00000) { /* if |z| > 0.5, set n = [z+0.5] */
    n = j + (0x00100000 >> (k + 1));
    k = ((n & 0x7fffffff) >> 20) - 0x3ff; /* new k for n */
    t = 0.0;
    SET_HIGH_WORD(t, n & ~(0x000fffff >> k));
    n = ((n & 0x000fffff) | 0x00100000) >> (20 - k);
    if (j < 0)
      n = -n;
    p_h -= t;
  }
  t = p_l + p_h;
  SET_LOW_WORD(t, 0);
  u = t * lg2_h;
  v = (p_l - (t - p_h)) * lg2 + t * lg2_l;
  z = u + v;
  w = v - (z - u);
  t = z * z;
  t1 = z - t * (P1 + t * (P2 + t * (P3 + t * (P4 + t * P5))));
  r = (z * t1) / (t1 - 2.0) - (w + z * w);
  z = 1.0 - (r - z);
  GET_HIGH_WORD(j, z);
  j += n << 20;
  if ((j >> 20) <= 0) /* subnormal output */
    z = scalbn(z, n);
  else
    SET_HIGH_WORD(z, j);
  return s * z;
}
static const double half[2] = {0.5, -0.5},
                    ln2hi =
                        6.93147180369123816490e-01, /* 0x3fe62e42, 0xfee00000 */
    ln2lo = 1.90821492927058770002e-10,             /* 0x3dea39ef, 0x35793c76 */
    invln2 = 1.44269504088896338700e+00,            /* 0x3ff71547, 0x652b82fe */
    P11 = 1.66666666666666019037e-01,               /* 0x3FC55555, 0x5555553E */
    P21 = -2.77777777770155933842e-03,              /* 0xBF66C16C, 0x16BEBD93 */
    P31 = 6.61375632143793436117e-05,               /* 0x3F11566A, 0xAF25DE2C */
    P41 = -1.65339022054652515390e-06,              /* 0xBEBBBD41, 0xC5D26BF1 */
    P51 = 4.13813679705723846039e-08;               /* 0x3E663769, 0x72BEA4D0 */

double exp(double x) {
  double_t hi, lo, c, xx, y;
  int k, sign;
  uint32_t hx;

  GET_HIGH_WORD(hx, x);
  sign = hx >> 31;
  hx &= 0x7fffffff; /* high word of |x| */

  /* special cases */
  if (hx >= 0x4086232b) { /* if |x| >= 708.39... */
    if (isnan(x))
      return x;
    if (x > 709.782712893383973096) {
      /* overflow if x!=inf */
      x *= 0x1p1023;
      return x;
    }
    if (x < -708.39641853226410622) {
      /* underflow if x!=-inf */
      FORCE_EVAL((float)(-0x1p-149 / x));
      if (x < -745.13321910194110842)
        return 0;
    }
  }

  /* argument reduction */
  if (hx > 0x3fd62e42) {  /* if |x| > 0.5 ln2 */
    if (hx >= 0x3ff0a2b2) /* if |x| >= 1.5 ln2 */
      k = (int)(invln2 * x + half[sign]);
    else
      k = 1 - sign - sign;
    hi = x - k * ln2hi; /* k*ln2hi is exact here */
    lo = k * ln2lo;
    x = hi - lo;
  } else if (hx > 0x3e300000) { /* if |x| > 2**-28 */
    k = 0;
    hi = x;
    lo = 0;
  } else {
    /* inexact if x!=0 */
    FORCE_EVAL(0x1p1023 + x);
    return 1 + x;
  }

  /* x is now in primary range */
  xx = x * x;
  c = x - xx * (P11 + xx * (P21 + xx * (P31 + xx * (P41 + xx * P51))));
  y = 1 + (x * c / (2 - c) - lo + hi);
  if (k == 0)
    return y;
  return scalbn(y, k);
}
double fmod(double x, double y) {
  union {
    double f;
    uint64_t i;
  } ux = {x}, uy = {y};
  int ex = ux.i >> 52 & 0x7ff;
  int ey = uy.i >> 52 & 0x7ff;
  int sx = ux.i >> 63;
  uint64_t i;

  /* in the followings uxi should be ux.i, but then gcc wrongly adds */
  /* float load/store to inner loops ruining performance and code size */
  uint64_t uxi = ux.i;

  if (uy.i << 1 == 0 || isnan(y) || ex == 0x7ff)
    return (x * y) / (x * y);
  if (uxi << 1 <= uy.i << 1) {
    if (uxi << 1 == uy.i << 1)
      return 0 * x;
    return x;
  }

  /* normalize x and y */
  if (!ex) {
    for (i = uxi << 12; i >> 63 == 0; ex--, i <<= 1)
      ;
    uxi <<= -ex + 1;
  } else {
    uxi &= -1ULL >> 12;
    uxi |= 1ULL << 52;
  }
  if (!ey) {
    for (i = uy.i << 12; i >> 63 == 0; ey--, i <<= 1)
      ;
    uy.i <<= -ey + 1;
  } else {
    uy.i &= -1ULL >> 12;
    uy.i |= 1ULL << 52;
  }

  /* x mod y */
  for (; ex > ey; ex--) {
    i = uxi - uy.i;
    if (i >> 63 == 0) {
      if (i == 0)
        return 0 * x;
      uxi = i;
    }
    uxi <<= 1;
  }
  i = uxi - uy.i;
  if (i >> 63 == 0) {
    if (i == 0)
      return 0 * x;
    uxi = i;
  }
  for (; uxi >> 52 == 0; uxi <<= 1, ex--)
    ;

  /* scale result */
  if (ex > 0) {
    uxi -= 1ULL << 52;
    uxi |= (uint64_t)ex << 52;
  } else {
    uxi >>= -ex + 1;
  }
  uxi |= (uint64_t)sx << 63;
  ux.i = uxi;
  return ux.f;
}
uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t* rem_p) {
  uint64_t quot = 0, qbit = 1;

  if (den == 0) {
    __asm__ __volatile__("int $0");
    return 0; /* If trap returns... */
  }

  /* Left-justify denominator and count shift */
  while ((int64_t)den >= 0) {
    den <<= 1;
    qbit <<= 1;
  }

  while (qbit) {
    if (den <= num) {
      num -= den;
      quot += qbit;
    }
    den >>= 1;
    qbit >>= 1;
  }

  if (rem_p)
    *rem_p = num;

  return quot;
}
typedef unsigned int UQItype __attribute__((mode(QI)));
typedef int SItype __attribute__((mode(SI)));
typedef unsigned int USItype __attribute__((mode(SI)));
typedef int DItype __attribute__((mode(DI)));
typedef unsigned int UDItype __attribute__((mode(DI)));
#define Wtype SItype
#define HWtype SItype
#define DWtype DItype
#define UWtype USItype
#define UHWtype USItype
#define UDWtype UDItype
#define W_TYPE_SIZE 32
DWtype __divdi3(DWtype u, DWtype v) {
  Wtype c = 0;
  DWtype w;

  if (u < 0) {
    c = ~c;
    u = -u;
  }
  if (v < 0) {
    c = ~c;
    v = -v;
  }
  w = __udivmoddi4(u, v, NULL);
  if (c)
    w = -w;
  return w;
}
// strong_alias (__divdi3, __divdi3_internal)

DWtype __moddi3(DWtype u, DWtype v) {
  Wtype c = 0;
  DWtype w;

  if (u < 0) {
    c = ~c;
    u = -u;
  }
  if (v < 0)
    v = -v;
  __udivmoddi4(u, v, (UDWtype*)&w);
  if (c)
    w = -w;
  return w;
}
// strong_alias (__moddi3, __moddi3_internal)

UDWtype __udivdi3(UDWtype u, UDWtype v) {
  return __udivmoddi4(u, v, NULL);
}
// strong_alias (__udivdi3, __udivdi3_internal)

UDWtype __umoddi3(UDWtype u, UDWtype v) {
  UDWtype w;

  __udivmoddi4(u, v, &w);
  return w;
}
static const double ivln10hi =
                        4.34294481878168880939e-01, /* 0x3fdbcb7b, 0x15200000 */
    ivln10lo = 2.50829467116452752298e-11,          /* 0x3dbb9438, 0xca9aadd5 */
    log10_2hi = 3.01029995663611771306e-01,         /* 0x3FD34413, 0x509F6000 */
    log10_2lo = 3.69423907715893078616e-13,         /* 0x3D59FEF3, 0x11F12B36 */
    Lg1 = 6.666666666666735130e-01,                 /* 3FE55555 55555593 */
    Lg2 = 3.999999999940941908e-01,                 /* 3FD99999 9997FA04 */
    Lg3 = 2.857142874366239149e-01,                 /* 3FD24924 94229359 */
    Lg4 = 2.222219843214978396e-01,                 /* 3FCC71C5 1D8E78AF */
    Lg5 = 1.818357216161805012e-01,                 /* 3FC74664 96CB03DE */
    Lg6 = 1.531383769920937332e-01,                 /* 3FC39A09 D078C69F */
    Lg7 = 1.479819860511658591e-01;                 /* 3FC2F112 DF3E5244 */

double log10(double x) {
  union {
    double f;
    uint64_t i;
  } u = {x};
  double_t hfsq, f, s, z, R, w, t1, t2, dk, y, hi, lo, val_hi, val_lo;
  uint32_t hx;
  int k;

  hx = u.i >> 32;
  k = 0;
  if (hx < 0x00100000 || hx >> 31) {
    if (u.i << 1 == 0)
      return -1 / (x * x); /* log(+-0)=-inf */
    if (hx >> 31)
      return (x - x) / 0.0; /* log(-#) = NaN */
    /* subnormal number, scale x up */
    k -= 54;
    x *= 0x1p54;
    u.f = x;
    hx = u.i >> 32;
  } else if (hx >= 0x7ff00000) {
    return x;
  } else if (hx == 0x3ff00000 && u.i << 32 == 0)
    return 0;

  /* reduce x into [sqrt(2)/2, sqrt(2)] */
  hx += 0x3ff00000 - 0x3fe6a09e;
  k += (int)(hx >> 20) - 0x3ff;
  hx = (hx & 0x000fffff) + 0x3fe6a09e;
  u.i = (uint64_t)hx << 32 | (u.i & 0xffffffff);
  x = u.f;

  f = x - 1.0;
  hfsq = 0.5 * f * f;
  s = f / (2.0 + f);
  z = s * s;
  w = z * z;
  t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
  t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
  R = t2 + t1;

  /* See log2.c for details. */
  /* hi+lo = f - hfsq + s*(hfsq+R) ~ log(1+f) */
  hi = f - hfsq;
  u.f = hi;
  u.i &= (uint64_t)-1 << 32;
  hi = u.f;
  lo = f - hi - hfsq + s * (hfsq + R);

  /* val_hi+val_lo ~ log10(1+f) + k*log10(2) */
  val_hi = hi * ivln10hi;
  dk = k;
  y = dk * log10_2hi;
  val_lo = dk * log10_2lo + (lo + hi) * ivln10lo + lo * ivln10hi;

  /*
   * Extra precision in for adding y is not strictly needed
   * since there is no very large cancellation near x = sqrt(2) or
   * x = 1/sqrt(2), but we do it anyway since it costs little on CPUs
   * with some parallelism and it reduces the error for many args.
   */
  w = y + val_hi;
  val_lo += (y - w) + val_hi;
  val_hi = w;

  return val_lo + val_hi;
}
void* memchr(const void* s, int c, size_t n) {
  const unsigned char* p = s;

  while (n-- != 0) {
    if ((unsigned char)c == *p++) {
      return (void*)(p - 1);
    }
  }

  return NULL;
}
/*
 * Convert string to double
 */
double strtod(const char* nptr, char** endptr) {
  double number;
  int exponent;
  int negative;
  char* p = (char*)nptr;
  double p10;
  int n;
  int num_digits;
  int num_decimals;

  /* Skip leading whitespace */
  while (isspace(*p))
    p++;

  /* Handle optional sign */
  negative = 0;
  switch (*p) {
    case '-':
      negative = 1;
      p++;
      break;
    case '+':
      p++;
      break;
  }

  number = 0.;
  exponent = 0;
  num_digits = 0;
  num_decimals = 0;

  /* Process string of digits */
  while (isdigit(*p)) {
    number = number * 10. + (*p - '0');
    p++;
    num_digits++;
  }

  /* Process decimal part */
  if (*p == '.') {
    p++;

    while (isdigit(*p)) {
      number = number * 10. + (*p - '0');
      p++;
      num_digits++;
      num_decimals++;
    }

    exponent -= num_decimals;
  }

  if (num_digits == 0) {
    return 0.0;
  }

  /* Correct for sign */
  if (negative)
    number = -number;

  /* Process an exponent string */
  if (*p == 'e' || *p == 'E') {
    /* Handle optional sign */
    negative = 0;
    switch (*++p) {
      case '-':
        negative = 1;
        p++;
        break;
      case '+':
        p++;
        break;
    }

    /* Process string of digits */
    n = 0;
    while (isdigit(*p)) {
      n = n * 10 + (*p - '0');
      p++;
    }

    if (negative)
      exponent -= n;
    else
      exponent += n;
  }

  if (exponent < -307 || exponent > 308) {
    return 0.0;
  }

  /* Scale the result */
  p10 = 10.;
  n = exponent;
  if (n < 0)
    n = -n;
  while (n) {
    if (n & 1) {
      if (exponent < 0)
        number /= p10;
      else
        number *= p10;
    }
    n >>= 1;
    p10 *= p10;
  }

  if (endptr)
    *endptr = p;

  return number;
}
void abort(void) {
  exit();
}

const char* strstr(const char* p1, const char* p2) {
  char* s1 = (char*)p1;
  char* s2 = (char*)p2;
  char* cur = (char*)p1;
  if (*p2 == '\0') {
    return p1;
  }
  while (*cur) {
    s1 = cur;
    s2 = (char*)p2;
    while ((*s1 != '\0') && (*s2 != '\0') && (*s1 == *s2)) {
      s1++;
      s2++;
    }
    if (*s2 == '\0') {
      return cur;
    }
    cur++;
  }
  return NULL;
}
int toupper(int c) {
  if ((c >= 'a') && (c <= 'z'))
    return c + ('A' - 'a');
  return c;
}
int tolower(int c) {
  if ((c >= 'A') && (c <= 'Z'))
    return c + ('a' - 'A');
  return c;
}
int isalnum(int ch) {
  return (unsigned int)((ch | 0x20) - 'a') < 26u ||
         (unsigned int)(ch - '0') < 10u;
}
int iscntrl(int ch) {
  return (unsigned int)ch < 32u || ch == 127;
}  //判断字符c是否为控制字符。当c在0x00-0x1F之间或等于0x7F(DEL)时，返回非零值，否则返回零。
int islower(int ch) {
  return (unsigned int)(ch - 'a') < 26u;
}  //判断字符c是否为小写英文字母
int isxdigit(int c) {
  if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
      ('A' <= c && c <= 'F')) {
    return 1;
  }
  return 0;
}
int isgraph(int ch) {
  return (unsigned int)(ch - '!') < 127u - '!';
}  //判断字符c是否为除空格外的可打印字符。可打印字符（0x21-0x7e）。
int isprint(int ch) {
  return (unsigned int)(ch - ' ') < 127u - ' ';
}  //判断字符c是否为可打印字符（含空格）。当c为可打印字符（0x20-0x7e）时，返回非零值，否则返回零。
int ispunct(int ch) {
  return isprint(ch) && !isalnum(ch) && !isspace(ch);
}  //判断字符c是否为标点符号。标点符号指那些既不是字母数字，也不是空格的可打印字符。
char* strpbrk(s, accept)
register const char* s;
register const char* accept;
{
  while (*s != '\0') {
    const char* a = accept;
    while (*a != '\0')
      if (*a++ == *s)
        return (char*)s;
    ++s;
  }

  return 0;
}
size_t strspn(const char* s, const char* accept) {
  const char* p = s;
  const char* a;
  size_t count = 0;

  for (; *p != '\0'; ++p) {
    for (a = accept; *a != '\0'; ++a) {
      if (*p == *a)
        break;
    }
    if (*a == '\0')
      return count;
    ++count;
  }
  return count;
}
int strcoll(const char* str1, const char* str2) {
  return strcmp(str1, str2);
}
double frexp(double x, int* e) {
  union {
    double d;
    uint64_t i;
  } y = {x};
  int ee = y.i >> 52 & 0x7ff;

  if (!ee) {
    if (x) {
      x = frexp(x * 0x1p64, e);
      *e -= 64;
    } else
      *e = 0;
    return x;
  } else if (ee == 0x7ff) {
    return x;
  }

  *e = ee - 0x3fe;
  y.i &= 0x800fffffffffffffull;
  y.i |= 0x3fe0000000000000ull;
  return y.d;
}
static const struct lconv posix_lconv = {
    .decimal_point = ".",
    .thousands_sep = "",
    .grouping = "",
    .int_curr_symbol = "",
    .currency_symbol = "",
    .mon_decimal_point = "",
    .mon_thousands_sep = "",
    .mon_grouping = "",
    .positive_sign = "",
    .negative_sign = "",
    .int_frac_digits = -1,
    .frac_digits = -1,
    .p_cs_precedes = -1,
    .p_sep_by_space = -1,
    .n_cs_precedes = -1,
    .n_sep_by_space = -1,
    .p_sign_posn = -1,
    .n_sign_posn = -1,
    .int_p_cs_precedes = -1,
    .int_p_sep_by_space = -1,
    .int_n_cs_precedes = -1,
    .int_n_sep_by_space = -1,
    .int_p_sign_posn = -1,
    .int_n_sign_posn = -1,
};

struct lconv* localeconv(void) {
  return (struct lconv*)&posix_lconv;
}
double modf(double x, double* iptr) {
  union {
    double f;
    uint64_t i;
  } u = {x};
  uint64_t mask;
  int e = (int)(u.i >> 52 & 0x7ff) - 0x3ff;

  /* no fractional part */
  if (e >= 52) {
    *iptr = x;
    if (e == 0x400 && u.i << 12 != 0) /* nan */
      return x;
    u.i &= 1ULL << 63;
    return u.f;
  }

  /* no integral part*/
  if (e < 0) {
    u.i &= 1ULL << 63;
    *iptr = u.f;
    return x;
  }

  mask = -1ULL >> 12 >> e;
  if ((u.i & mask) == 0) {
    *iptr = x;
    u.i &= 1ULL << 63;
    return u.f;
  }
  u.i &= ~mask;
  *iptr = u.f;
  return x - u.f;
}
char* gets(char* str) {
  scan(str, 1024);
  return str;
}
int puts(char* str) {
  print(str);
  return strlen(str);
}

int fseek(FILE* fp, int offset, int whence) {
  if (whence == 0) {
    fp->p = offset;
  } else if (whence == 1) {
    fp->p += offset;
  } else if (whence == 2) {
    fp->p = fp->fileSize + offset;
  } else {
    return -1;
  }
  return 0;
}
long ftell(FILE* stream) {
  return stream->p;
}
#define CANREAD(flag) ((flag)&READ || (flag)&PLUS)
#define CANWRITE(flag) ((flag)&WRITE || (flag)&PLUS || (flag)&APPEND)
FILE* fopen(char* filename, char* mode) {
  unsigned int flag = 0;
  FILE* fp = (FILE*)malloc(sizeof(FILE));
  while (*mode != '\0') {
    switch (*mode) {
      case 'a':
        flag |= APPEND;
        break;
      case 'b':
        break;
      case 'r':
        flag |= READ;
        break;
      case 'w':
        flag |= WRITE;
        break;
      case '+':
        flag |= PLUS;
        break;
      default:
        break;
    }
    mode++;
  }
  if (filesize(filename) == -1) {
    if (flag & READ) {
      free(fp);
      errno = ENOENT;
      return NULL;  // 找不到
    }
    if (flag & WRITE || flag & APPEND) {
      mkfile(filename);
    }
  } else if (flag & WRITE) {
    char buffe2[100];
    sprintf(buffe2, "del %s", filename);
    system(buffe2);
    mkfile(filename);
  }
  if (flag & WRITE) {
    fp->fileSize = 0;
  } else {
    fp->fileSize = filesize(filename);
  }
  fp->bufferSize = 0;
  if (flag & READ || flag & PLUS || flag & APPEND) {
    fp->bufferSize = filesize(filename);
    // printf("[Set]BufferSize=%d\n",fp->bufferSize);
  }
  if (flag & WRITE || flag & PLUS || flag & APPEND) {
    fp->bufferSize += 100;
  }
  if (fp->bufferSize == 0) {
    fp->bufferSize = 1;
  }
  fp->buffer = malloc(fp->bufferSize);
  if (flag & PLUS || flag & APPEND || flag & READ) {
    //	printf("ReadFile........\n");
    api_ReadFile(filename, fp->buffer);
  }
  fp->p = 0;
  if (flag & APPEND) {
    fp->p = fp->fileSize;
  }
  fp->name = malloc(strlen(filename) + 1);
  strcpy(fp->name, filename);
  fp->mode = flag;
  //	printf("[fopen]BufferSize=%d\n",fp->bufferSize);
  return fp;
}
int fgetc(FILE* stream) {
  if (CANREAD(stream->mode)) {
    if (stream == stdin) {
      return getch();
    }
    if (stream->p >= stream->fileSize) {
      return EOF;
    } else {
      return stream->buffer[stream->p++];
    }
  } else {
    return EOF;
  }
}
int fputc(int ch, FILE* stream) {
  if (CANWRITE(stream->mode)) {
    if (stream == stdout || stream == stderr) {
      putch(ch);
      return ch;
    }
    //		printf("Current Buffer=%s\n",stream->buffer);
    if (stream->p >= stream->bufferSize) {
      //	printf("Realloc....(%d,%d)\n",stream->p,stream->bufferSize);
      stream->buffer = realloc(stream->buffer, stream->bufferSize + 100);
      stream->bufferSize += 100;
    }
    if (stream->p >= stream->fileSize) {
      stream->fileSize++;
    }
    //		printf("Current Buffer=%s(A)\n",stream->buffer);
    stream->buffer[stream->p++] = ch;
    //	printf("Current Buffer=%s(B)\n",stream->buffer);
    return ch;
  }
  return EOF;
}
unsigned int fwrite(const void* ptr,
                    unsigned int size,
                    unsigned int nmemb,
                    FILE* stream) {
  if (CANWRITE(stream->mode)) {
    unsigned char* c_ptr = (unsigned char*)ptr;
    for (int i = 0; i < size * nmemb; i++) {
      fputc(c_ptr[i], stream);
    }
    return nmemb;
  } else {
    return 0;
  }
}
unsigned int fread(void* buffer,
                   unsigned int size,
                   unsigned int count,
                   FILE* stream) {
  if (CANREAD(stream->mode)) {
    if (stream == stdin) {
      scan(buffer, size * count);
      return count;
    }
    unsigned char* c_ptr = (unsigned char*)buffer;
    for (int i = 0; i < size * count; i++) {
      unsigned int ch = fgetc(stream);
      if (ch == EOF) {
        return i;
      } else {
        c_ptr[i] = ch;
      }
    }
    return count;
  } else {
    return 0;
  }
}
int fclose(FILE* fp) {
  if (fp == NULL) {
    return EOF;
  }
  if (CANWRITE(fp->mode)) {
    //		printf("Save file.....(%s) Size =
    //%d\n",fp->buffer,fp->fileSize);
    Edit_File(fp->name, fp->buffer, fp->fileSize, 0);
  }
  free(fp->buffer);
  free(fp->name);
  free(fp);
  return 0;
}
int fflush(FILE* stream) {
  return 0;  // Who needs fflush??????????
}
char* fgets(char* str, int n, FILE* stream) {
  if (CANREAD(stream->mode)) {
    if (stream == stdin) {
      scan(str, n);
      return str;
    } else {
      for (int i = 0; i < n; i++) {
        unsigned int ch = fgetc(stream);
        if (ch == EOF) {
          if (i == 0) {
            return NULL;
          } else {
            break;
          }
        }
        if (ch == '\n') {
          break;
        }
        str[i] = ch;
      }
      return str;
    }
  }
  return NULL;
}
int fputs(const char* str, FILE* stream) {
  if (CANWRITE(stream->mode)) {
    if (stream == stdout || stream == stderr) {
      print(str);
      return 0;
    } else {
      for (int i = 0; i < strlen(str); i++) {
        fputc(str[i], stream);
      }
      return 0;
    }
  }
  return EOF;
}
int fprintf(FILE* stream, const char* format, ...) {
  if (CANWRITE(stream->mode)) {
    int len;
    va_list ap;
    va_start(ap, format);
    char* buf = malloc(1024);
    len = vsprintf(buf, format, ap);
    fputs(buf, stream);
    free(buf);
    va_end(ap);
    return len;
  } else {
    // printf("CAN NOT WRITE\n");
    return EOF;
  }
}

int feof(FILE* stream) {
  if (stream->p >= stream->fileSize) {
    return EOF;
  }
  return 0;
}
int ferror(FILE* stream) {
  return 0;
}
int getc(FILE* stream) {
  return fgetc(stream);
}
char* strerror(int errno) {
  if (errno == ENOENT) {
    return "No such file.";
  }
  return "errno error.";
}
int ungetc(int c, FILE* fp) {
  if (fp->p - 1 < 0) {
    return EOF;
  } else {
    fp->p -= 1;
    fp->buffer[fp->p] = c;
    return c;
  }
}
enum flags1 {
  FL_SPLAT = 0x01,  /* Drop the value, do not assign */
  FL_INV = 0x02,    /* Character-set with inverse */
  FL_WIDTH = 0x04,  /* Field width specified */
  FL_MINUS1 = 0x08, /* Negative number */
};

enum rank1s1 {
  rank1_char = -2,
  rank1_short = -1,
  rank1_int = 0,
  rank1_long = 1,
  rank1_longlong = 2,
  rank1_ptr = INT_MAX, /* Special value used for pointers */
};

#define MIN_rank1 rank1_char
#define MAX_rank1 rank1_longlong
#define INTMAX_rank1 rank1_longlong
#define SIZE_T_rank1 rank1_long
#define PTRDIFF_T_rank1 rank1_long

enum bail {
  bail_none = 0, /* No error condition */
  bail_eof,      /* Hit EOF */
  bail_err,      /* Conversion mismatch */
};

static inline const char* skipspace(const char* p) {
  while (isspace((unsigned char)*p))
    p++;
  return p;
}

static inline void set_bit(unsigned long* bitmap, unsigned int bit) {
  bitmap[bit / (8 * sizeof(long))] |= 1UL << (bit % (8 * sizeof(long)));
}

static inline int test_bit(unsigned long* bitmap, unsigned int bit) {
  return (int)(bitmap[bit / (8 * sizeof(long))] >> (bit % (8 * sizeof(long)))) &
         1;
}
uintmax_t strntoumax(const char* str, char** endptr, int base, size_t n) {
  // 检查参数是否合法
  if (str == NULL || (base != 0 && (base < 2 || base > 36))) {
    errno = EINVAL;  // 设置错误码
    return 0;
  }

  uintmax_t result = 0;  // 用于存储转换结果
  int neg = 0;           // 是否是负数
  size_t count = 0;

  // 跳过空白符
  while (isspace(*str) && count < n) {
    ++str;
    ++count;
  }

  // 处理正负号
  if (*str == '-' && count < n) {
    neg = 1;
    ++str;
    ++count;
  } else if (*str == '+' && count < n) {
    ++str;
    ++count;
  }

  // 确定进制
  if (base == 0) {
    // 自动判断进制
    if (*str == '0' && count < n) {
      ++str;
      ++count;
      if ((*str == 'x' || *str == 'X') && count < n) {
        base = 16;
        ++str;
        ++count;
      } else {
        base = 8;
      }
    } else {
      base = 10;
    }
  } else if (base == 16) {
    // 处理0x/0X前缀
    if (*str == '0' && count < n) {
      ++str;
      ++count;
      if ((*str == 'x' || *str == 'X') && count < n) {
        ++str;
        ++count;
      }
    }
  }

  // 转换数字部分
  int digit;
  while ((digit = isalnum(*str) ? toupper(*str) - '0' : 36) < base &&
         count < n) {
    result = result * base + digit;
    ++str;
    ++count;
  }

  // 检查是否有未处理的部分
  if (endptr != NULL) {
    *endptr = (char*)str;
  }
  while (count < n && isspace(*str)) {
    ++str;
    ++count;
  }
  if (count >= n) {
    errno = ERANGE;  // 设置错误码
  }

  // 添加符号位
  if (neg) {
    result = -result;
  }

  return result;
}
int vsscanf(const char* buf, const char* fmt, va_list ap) {
  const char* p = fmt;
  char ch;
  unsigned char uc;
  const char* q = buf;
  const char* qq;
  uintmax_t val = 0;
  int rank1 = rank1_int;
  unsigned int width = INT_MAX;
  int base;
  enum flags1 flags1 = 0;
  enum {
    st_normal,      /* Ground state */
    st_flags1,      /* Special flags1 */
    st_width,       /* Field width */
    st_modifiers,   /* Length or conversion modifiers */
    st_match_init,  /* Initial state of %[ sequence */
    st_match,       /* Main state of %[ sequence */
    st_match_range, /* After - in a %[ sequence */
  } state = st_normal;
  char* sarg = NULL; /* %s %c or %[ string argument */
  enum bail bail = bail_none;
  int converted = 0; /* Successful conversions */
  unsigned long
      matchmap[((1 << 8) + ((8 * sizeof(long)) - 1)) / (8 * sizeof(long))];
  int matchinv = 0; /* Is match map inverted? */
  unsigned char range_start = 0;

  while ((ch = *p++) && !bail) {
    switch (state) {
      case st_normal:
        if (ch == '%') {
          state = st_flags1;
          flags1 = 0;
          rank1 = rank1_int;
          width = INT_MAX;
        } else if (isspace((unsigned char)ch)) {
          q = skipspace(q);
        } else {
          if (*q == ch)
            q++;
          else
            bail = bail_err;
        }
        break;

      case st_flags1:
        switch (ch) {
          case '*':
            flags1 |= FL_SPLAT;
            break;
          case '0' ... '9':
            width = (ch - '0');
            state = st_width;
            flags1 |= FL_WIDTH;
            break;
          default:
            state = st_modifiers;
            p--; /* Process this character again */
            break;
        }
        break;

      case st_width:
        if (ch >= '0' && ch <= '9') {
          width = width * 10 + (ch - '0');
        } else {
          state = st_modifiers;
          p--; /* Process this character again */
        }
        break;

      case st_modifiers:
        switch (ch) {
          /* Length modifiers - nonterminal sequences */
          case 'h':
            rank1--; /* Shorter rank1 */
            break;
          case 'l':
            rank1++; /* Longer rank1 */
            break;
          case 'j':
            rank1 = INTMAX_rank1;
            break;
          case 'z':
            rank1 = SIZE_T_rank1;
            break;
          case 't':
            rank1 = PTRDIFF_T_rank1;
            break;
          case 'L':
          case 'q':
            rank1 = rank1_longlong; /* long double/long long */
            break;

          default:
            /* Output modifiers - terminal sequences */
            /* Next state will be normal */
            state = st_normal;

            /* Canonicalize rank1 */
            if (rank1 < MIN_rank1)
              rank1 = MIN_rank1;
            else if (rank1 > MAX_rank1)
              rank1 = MAX_rank1;

            switch (ch) {
              case 'P': /* Upper case pointer */
              case 'p': /* Pointer */
                rank1 = rank1_ptr;
                base = 0;
                goto scan_int;

              case 'i': /* Base-independent integer */
                base = 0;
                goto scan_int;

              case 'd': /* Decimal integer */
                base = 10;
                goto scan_int;

              case 'o': /* Octal integer */
                base = 8;
                goto scan_int;

              case 'u': /* Unsigned decimal integer */
                base = 10;
                goto scan_int;

              case 'x': /* Hexadecimal integer */
              case 'X':
                base = 16;
                goto scan_int;

              case 'n': /* # of characters consumed */
                val = (q - buf);
                goto set_integer;

              scan_int:
                q = skipspace(q);
                if (!*q) {
                  bail = bail_eof;
                  break;
                }
                val = strntoumax(q, (char**)&qq, base, width);
                if (qq == q) {
                  bail = bail_err;
                  break;
                }
                q = qq;
                if (!(flags1 & FL_SPLAT))
                  converted++;

              set_integer:
                if (!(flags1 & FL_SPLAT)) {
                  switch (rank1) {
                    case rank1_char:
                      *va_arg(ap, unsigned char*) = val;
                      break;
                    case rank1_short:
                      *va_arg(ap, unsigned short*) = val;
                      break;
                    case rank1_int:
                      *va_arg(ap, unsigned int*) = val;
                      break;
                    case rank1_long:
                      *va_arg(ap, unsigned long*) = val;
                      break;
                    case rank1_longlong:
                      *va_arg(ap, unsigned long long*) = val;
                      break;
                    case rank1_ptr:
                      *va_arg(ap, void**) = (void*)(uintptr_t)val;
                      break;
                  }
                }
                break;

              case 'c': /* Character */
                width = (flags1 & FL_WIDTH) ? width : 1;
                if (flags1 & FL_SPLAT) {
                  while (width--) {
                    if (!*q) {
                      bail = bail_eof;
                      break;
                    }
                  }
                } else {
                  sarg = va_arg(ap, char*);
                  while (width--) {
                    if (!*q) {
                      bail = bail_eof;
                      break;
                    }
                    *sarg++ = *q++;
                  }
                  if (!bail)
                    converted++;
                }
                break;

              case 's': /* String */
                uc = 1; /* Anything nonzero */
                if (flags1 & FL_SPLAT) {
                  while (width-- && (uc = *q) && !isspace(uc)) {
                    q++;
                  }
                } else {
                  char* sp;
                  sp = sarg = va_arg(ap, char*);
                  while (width-- && (uc = *q) && !isspace(uc)) {
                    *sp++ = uc;
                    q++;
                  }
                  if (sarg != sp) {
                    /*
                     * Terminate output
                     */
                    *sp = '\0';
                    converted++;
                  }
                }
                if (!uc)
                  bail = bail_eof;
                break;

              case '[': /* Character range */
                sarg = (flags1 & FL_SPLAT) ? NULL : va_arg(ap, char*);
                state = st_match_init;
                matchinv = 0;
                memset(matchmap, 0, sizeof matchmap);
                break;

              case '%': /* %% sequence */
                if (*q == '%')
                  q++;
                else
                  bail = bail_err;
                break;

              default: /* Anything else */
                /* Unknown sequence */
                bail = bail_err;
                break;
            }
            break;
        }
        break;

      case st_match_init: /* Initial state for %[ match */
        if (ch == '^' && !(flags1 & FL_INV)) {
          matchinv = 1;
        } else {
          set_bit(matchmap, (unsigned char)ch);
          state = st_match;
        }
        break;

      case st_match: /* Main state for %[ match */
        if (ch == ']') {
          goto match_run;
        } else if (ch == '-') {
          range_start = (unsigned char)ch;
          state = st_match_range;
        } else {
          set_bit(matchmap, (unsigned char)ch);
        }
        break;

      case st_match_range: /* %[ match after - */
        if (ch == ']') {
          /* - was last character */
          set_bit(matchmap, (unsigned char)'-');
          goto match_run;
        } else {
          int i;
          for (i = range_start; i < (unsigned char)ch; i++)
            set_bit(matchmap, i);
          state = st_match;
        }
        break;

      match_run: /* Match expression finished */
        qq = q;
        uc = 1; /* Anything nonzero */
        while (width && (uc = *q) && (test_bit(matchmap, uc) ^ matchinv)) {
          if (sarg)
            *sarg++ = uc;
          q++;
        }
        if (q != qq && sarg) {
          *sarg = '\0';
          converted++;
        } else {
          bail = bail_err;
        }
        if (!uc)
          bail = bail_eof;
        break;
    }
  }

  if (bail == bail_eof && !converted)
    converted = -1;

  return converted;
}
int sscanf(const char* s, const char* fmt, ...) {
  int ret;
  va_list ap;
  va_start(ap, fmt);
  ret = vsscanf(s, fmt, ap);
  va_end(ap);
  return ret;
}
static inline int a_ctz_32(uint32_t x) {
#ifdef a_clz_32
  return 31 - a_clz_32(x & -x);
#else
  static const char debruijn32[32] = {
      0,  1,  23, 2,  29, 24, 19, 3,  30, 27, 25, 11, 20, 8, 4,  13,
      31, 22, 28, 18, 26, 10, 7,  12, 21, 17, 9,  6,  16, 5, 15, 14};
  return debruijn32[(x & -x) * 0x076be629 >> 27];
#endif
}
static inline int a_ctz_l(unsigned long x) {
  return a_ctz_32(x);
}
#define ntz(x) a_ctz_l((x))

typedef int (*cmpfun)(const void*, const void*, void*);

static inline int pntz(size_t p[2]) {
  int r = ntz(p[0] - 1);
  if (r != 0 || (r = 8 * sizeof(size_t) + ntz(p[1])) != 8 * sizeof(size_t)) {
    return r;
  }
  return 0;
}

static void cycle(size_t width, unsigned char* ar[], int n) {
  unsigned char tmp[256];
  size_t l;
  int i;

  if (n < 2) {
    return;
  }

  ar[n] = tmp;
  while (width) {
    l = sizeof(tmp) < width ? sizeof(tmp) : width;
    memcpy(ar[n], ar[0], l);
    for (i = 0; i < n; i++) {
      memcpy(ar[i], ar[i + 1], l);
      ar[i] += l;
    }
    width -= l;
  }
}

/* shl() and shr() need n > 0 */
static inline void shl(size_t p[2], int n) {
  if (n >= 8 * sizeof(size_t)) {
    n -= 8 * sizeof(size_t);
    p[1] = p[0];
    p[0] = 0;
  }
  p[1] <<= n;
  p[1] |= p[0] >> (sizeof(size_t) * 8 - n);
  p[0] <<= n;
}

static inline void shr(size_t p[2], int n) {
  if (n >= 8 * sizeof(size_t)) {
    n -= 8 * sizeof(size_t);
    p[0] = p[1];
    p[1] = 0;
  }
  p[0] >>= n;
  p[0] |= p[1] << (sizeof(size_t) * 8 - n);
  p[1] >>= n;
}

static void sift(unsigned char* head,
                 size_t width,
                 cmpfun cmp,
                 void* arg,
                 int pshift,
                 size_t lp[]) {
  unsigned char *rt, *lf;
  unsigned char* ar[14 * sizeof(size_t) + 1];
  int i = 1;

  ar[0] = head;
  while (pshift > 1) {
    rt = head - width;
    lf = head - width - lp[pshift - 2];

    if (cmp(ar[0], lf, arg) >= 0 && cmp(ar[0], rt, arg) >= 0) {
      break;
    }
    if (cmp(lf, rt, arg) >= 0) {
      ar[i++] = lf;
      head = lf;
      pshift -= 1;
    } else {
      ar[i++] = rt;
      head = rt;
      pshift -= 2;
    }
  }
  cycle(width, ar, i);
}

static void trinkle(unsigned char* head,
                    size_t width,
                    cmpfun cmp,
                    void* arg,
                    size_t pp[2],
                    int pshift,
                    int trusty,
                    size_t lp[]) {
  unsigned char *stepson, *rt, *lf;
  size_t p[2];
  unsigned char* ar[14 * sizeof(size_t) + 1];
  int i = 1;
  int trail;

  p[0] = pp[0];
  p[1] = pp[1];

  ar[0] = head;
  while (p[0] != 1 || p[1] != 0) {
    stepson = head - lp[pshift];
    if (cmp(stepson, ar[0], arg) <= 0) {
      break;
    }
    if (!trusty && pshift > 1) {
      rt = head - width;
      lf = head - width - lp[pshift - 2];
      if (cmp(rt, stepson, arg) >= 0 || cmp(lf, stepson, arg) >= 0) {
        break;
      }
    }

    ar[i++] = stepson;
    head = stepson;
    trail = pntz(p);
    shr(p, trail);
    pshift += trail;
    trusty = 0;
  }
  if (!trusty) {
    cycle(width, ar, i);
    sift(head, width, cmp, arg, pshift, lp);
  }
}

void qsort(void* base, size_t nel, size_t width, cmpfun cmp, void* arg) {
  size_t lp[12 * sizeof(size_t)];
  size_t i, size = width * nel;
  unsigned char *head, *high;
  size_t p[2] = {1, 0};
  int pshift = 1;
  int trail;

  if (!size)
    return;

  head = base;
  high = head + size - width;

  /* Precompute Leonardo numbers, scaled by element width */
  for (lp[0] = lp[1] = width, i = 2;
       (lp[i] = lp[i - 2] + lp[i - 1] + width) < size; i++)
    ;

  while (head < high) {
    if ((p[0] & 3) == 3) {
      sift(head, width, cmp, arg, pshift, lp);
      shr(p, 2);
      pshift += 2;
    } else {
      if (lp[pshift - 1] >= high - head) {
        trinkle(head, width, cmp, arg, p, pshift, 0, lp);
      } else {
        sift(head, width, cmp, arg, pshift, lp);
      }

      if (pshift == 1) {
        shl(p, 1);
        pshift = 0;
      } else {
        shl(p, pshift - 1);
        pshift = 1;
      }
    }

    p[0] |= 1;
    head += width;
  }

  trinkle(head, width, cmp, arg, p, pshift, 0, lp);

  while (pshift != 1 || p[0] != 1 || p[1] != 0) {
    if (pshift <= 1) {
      trail = pntz(p);
      shr(p, trail);
      pshift += trail;
    } else {
      shl(p, 2);
      pshift -= 2;
      p[0] ^= 7;
      shr(p, 1);
      trinkle(head - lp[pshift] - width, width, cmp, arg, p, pshift + 1, 1, lp);
      shl(p, 1);
      p[0] |= 1;
      trinkle(head - width, width, cmp, arg, p, pshift, 1, lp);
    }
    head -= width;
  }
}
int snprintf(char* s, size_t n, const char* fmt, ...) {
  int ret;
  va_list ap;
  va_start(ap, fmt);
  ret = vsnprintf(s, n, fmt, ap);
  va_end(ap);
  return ret;
}
void getenv() {
  return NULL;
}
#define weak __attribute__((__weak__))
#define hidden __attribute__((__visibility__("hidden")))
#define weak_alias(old, new) \
  extern __typeof(old) new __attribute__((__weak__, __alias__(#old)))
#define LOG2_TABLE_BITS 6
#define LOG2_POLY_ORDER 7
#define LOG2_POLY1_ORDER 11
extern hidden const struct log2_data {
  double invln2hi;
  double invln2lo;
  double poly[LOG2_POLY_ORDER - 1];
  double poly1[LOG2_POLY1_ORDER - 1];
  struct {
    double invc, logc;
  } tab[1 << LOG2_TABLE_BITS];
#if !__FP_FAST_FMA
  struct {
    double chi, clo;
  } tab2[1 << LOG2_TABLE_BITS];
#endif
} __log2_data;
#define N (1 << LOG2_TABLE_BITS)

const struct log2_data __log2_data = {
    // First coefficient: 0x1.71547652b82fe1777d0ffda0d24p0
    .invln2hi = 0x1.7154765200000p+0,
    .invln2lo = 0x1.705fc2eefa200p-33,
    .poly1 =
        {
            // relative error: 0x1.2fad8188p-63
            // in -0x1.5b51p-5 0x1.6ab2p-5
            -0x1.71547652b82fep-1,
            0x1.ec709dc3a03f7p-2,
            -0x1.71547652b7c3fp-2,
            0x1.2776c50f05be4p-2,
            -0x1.ec709dd768fe5p-3,
            0x1.a61761ec4e736p-3,
            -0x1.7153fbc64a79bp-3,
            0x1.484d154f01b4ap-3,
            -0x1.289e4a72c383cp-3,
            0x1.0b32f285aee66p-3,
        },
    .poly =
        {
            // relative error: 0x1.a72c2bf8p-58
            // abs error: 0x1.67a552c8p-66
            // in -0x1.f45p-8 0x1.f45p-8
            -0x1.71547652b8339p-1,
            0x1.ec709dc3a04bep-2,
            -0x1.7154764702ffbp-2,
            0x1.2776c50034c48p-2,
            -0x1.ec7b328ea92bcp-3,
            0x1.a6225e117f92ep-3,
        },
    /* Algorithm:

            x = 2^k z
            log2(x) = k + log2(c) + log2(z/c)
            log2(z/c) = poly(z/c - 1)

    where z is in [1.6p-1; 1.6p0] which is split into N subintervals and z falls
    into the ith one, then table entries are computed as

            tab[i].invc = 1/c
            tab[i].logc = (double)log2(c)
            tab2[i].chi = (double)c
            tab2[i].clo = (double)(c - (double)c)

    where c is near the center of the subinterval and is chosen by trying +-2^29
    floating point invc candidates around 1/center and selecting one for which

            1) the rounding error in 0x1.8p10 + logc is 0,
            2) the rounding error in z - chi - clo is < 0x1p-64 and
            3) the rounding error in (double)log2(c) is minimized (< 0x1p-68).

    Note: 1) ensures that k + logc can be computed without rounding error, 2)
    ensures that z/c - 1 can be computed as (z - chi - clo)*invc with close to a
    single rounding error when there is no fast fma for z*invc - 1, 3) ensures
    that logc + poly(z/c - 1) has small error, however near x == 1 when
    |log2(x)| < 0x1p-4, this is not enough so that is special cased.  */
    .tab =
        {
            {0x1.724286bb1acf8p+0, -0x1.1095feecdb000p-1},
            {0x1.6e1f766d2cca1p+0, -0x1.08494bd76d000p-1},
            {0x1.6a13d0e30d48ap+0, -0x1.00143aee8f800p-1},
            {0x1.661ec32d06c85p+0, -0x1.efec5360b4000p-2},
            {0x1.623fa951198f8p+0, -0x1.dfdd91ab7e000p-2},
            {0x1.5e75ba4cf026cp+0, -0x1.cffae0cc79000p-2},
            {0x1.5ac055a214fb8p+0, -0x1.c043811fda000p-2},
            {0x1.571ed0f166e1ep+0, -0x1.b0b67323ae000p-2},
            {0x1.53909590bf835p+0, -0x1.a152f5a2db000p-2},
            {0x1.5014fed61adddp+0, -0x1.9217f5af86000p-2},
            {0x1.4cab88e487bd0p+0, -0x1.8304db0719000p-2},
            {0x1.49539b4334feep+0, -0x1.74189f9a9e000p-2},
            {0x1.460cbdfafd569p+0, -0x1.6552bb5199000p-2},
            {0x1.42d664ee4b953p+0, -0x1.56b23a29b1000p-2},
            {0x1.3fb01111dd8a6p+0, -0x1.483650f5fa000p-2},
            {0x1.3c995b70c5836p+0, -0x1.39de937f6a000p-2},
            {0x1.3991c4ab6fd4ap+0, -0x1.2baa1538d6000p-2},
            {0x1.3698e0ce099b5p+0, -0x1.1d98340ca4000p-2},
            {0x1.33ae48213e7b2p+0, -0x1.0fa853a40e000p-2},
            {0x1.30d191985bdb1p+0, -0x1.01d9c32e73000p-2},
            {0x1.2e025cab271d7p+0, -0x1.e857da2fa6000p-3},
            {0x1.2b404cf13cd82p+0, -0x1.cd3c8633d8000p-3},
            {0x1.288b02c7ccb50p+0, -0x1.b26034c14a000p-3},
            {0x1.25e2263944de5p+0, -0x1.97c1c2f4fe000p-3},
            {0x1.234563d8615b1p+0, -0x1.7d6023f800000p-3},
            {0x1.20b46e33eaf38p+0, -0x1.633a71a05e000p-3},
            {0x1.1e2eefdcda3ddp+0, -0x1.494f5e9570000p-3},
            {0x1.1bb4a580b3930p+0, -0x1.2f9e424e0a000p-3},
            {0x1.19453847f2200p+0, -0x1.162595afdc000p-3},
            {0x1.16e06c0d5d73cp+0, -0x1.f9c9a75bd8000p-4},
            {0x1.1485f47b7e4c2p+0, -0x1.c7b575bf9c000p-4},
            {0x1.12358ad0085d1p+0, -0x1.960c60ff48000p-4},
            {0x1.0fef00f532227p+0, -0x1.64ce247b60000p-4},
            {0x1.0db2077d03a8fp+0, -0x1.33f78b2014000p-4},
            {0x1.0b7e6d65980d9p+0, -0x1.0387d1a42c000p-4},
            {0x1.0953efe7b408dp+0, -0x1.a6f9208b50000p-5},
            {0x1.07325cac53b83p+0, -0x1.47a954f770000p-5},
            {0x1.05197e40d1b5cp+0, -0x1.d23a8c50c0000p-6},
            {0x1.03091c1208ea2p+0, -0x1.16a2629780000p-6},
            {0x1.0101025b37e21p+0, -0x1.720f8d8e80000p-8},
            {0x1.fc07ef9caa76bp-1, 0x1.6fe53b1500000p-7},
            {0x1.f4465d3f6f184p-1, 0x1.11ccce10f8000p-5},
            {0x1.ecc079f84107fp-1, 0x1.c4dfc8c8b8000p-5},
            {0x1.e573a99975ae8p-1, 0x1.3aa321e574000p-4},
            {0x1.de5d6f0bd3de6p-1, 0x1.918a0d08b8000p-4},
            {0x1.d77b681ff38b3p-1, 0x1.e72e9da044000p-4},
            {0x1.d0cb5724de943p-1, 0x1.1dcd2507f6000p-3},
            {0x1.ca4b2dc0e7563p-1, 0x1.476ab03dea000p-3},
            {0x1.c3f8ee8d6cb51p-1, 0x1.7074377e22000p-3},
            {0x1.bdd2b4f020c4cp-1, 0x1.98ede8ba94000p-3},
            {0x1.b7d6c006015cap-1, 0x1.c0db86ad2e000p-3},
            {0x1.b20366e2e338fp-1, 0x1.e840aafcee000p-3},
            {0x1.ac57026295039p-1, 0x1.0790ab4678000p-2},
            {0x1.a6d01bc2731ddp-1, 0x1.1ac056801c000p-2},
            {0x1.a16d3bc3ff18bp-1, 0x1.2db11d4fee000p-2},
            {0x1.9c2d14967feadp-1, 0x1.406464ec58000p-2},
            {0x1.970e4f47c9902p-1, 0x1.52dbe093af000p-2},
            {0x1.920fb3982bcf2p-1, 0x1.651902050d000p-2},
            {0x1.8d30187f759f1p-1, 0x1.771d2cdeaf000p-2},
            {0x1.886e5ebb9f66dp-1, 0x1.88e9c857d9000p-2},
            {0x1.83c97b658b994p-1, 0x1.9a80155e16000p-2},
            {0x1.7f405ffc61022p-1, 0x1.abe186ed3d000p-2},
            {0x1.7ad22181415cap-1, 0x1.bd0f2aea0e000p-2},
            {0x1.767dcf99eff8cp-1, 0x1.ce0a43dbf4000p-2},
        },
#if !__FP_FAST_FMA
    .tab2 =
        {
            {0x1.6200012b90a8ep-1, 0x1.904ab0644b605p-55},
            {0x1.66000045734a6p-1, 0x1.1ff9bea62f7a9p-57},
            {0x1.69fffc325f2c5p-1, 0x1.27ecfcb3c90bap-55},
            {0x1.6e00038b95a04p-1, 0x1.8ff8856739326p-55},
            {0x1.71fffe09994e3p-1, 0x1.afd40275f82b1p-55},
            {0x1.7600015590e1p-1, -0x1.2fd75b4238341p-56},
            {0x1.7a00012655bd5p-1, 0x1.808e67c242b76p-56},
            {0x1.7e0003259e9a6p-1, -0x1.208e426f622b7p-57},
            {0x1.81fffedb4b2d2p-1, -0x1.402461ea5c92fp-55},
            {0x1.860002dfafcc3p-1, 0x1.df7f4a2f29a1fp-57},
            {0x1.89ffff78c6b5p-1, -0x1.e0453094995fdp-55},
            {0x1.8e00039671566p-1, -0x1.a04f3bec77b45p-55},
            {0x1.91fffe2bf1745p-1, -0x1.7fa34400e203cp-56},
            {0x1.95fffcc5c9fd1p-1, -0x1.6ff8005a0695dp-56},
            {0x1.9a0003bba4767p-1, 0x1.0f8c4c4ec7e03p-56},
            {0x1.9dfffe7b92da5p-1, 0x1.e7fd9478c4602p-55},
            {0x1.a1fffd72efdafp-1, -0x1.a0c554dcdae7ep-57},
            {0x1.a5fffde04ff95p-1, 0x1.67da98ce9b26bp-55},
            {0x1.a9fffca5e8d2bp-1, -0x1.284c9b54c13dep-55},
            {0x1.adfffddad03eap-1, 0x1.812c8ea602e3cp-58},
            {0x1.b1ffff10d3d4dp-1, -0x1.efaddad27789cp-55},
            {0x1.b5fffce21165ap-1, 0x1.3cb1719c61237p-58},
            {0x1.b9fffd950e674p-1, 0x1.3f7d94194cep-56},
            {0x1.be000139ca8afp-1, 0x1.50ac4215d9bcp-56},
            {0x1.c20005b46df99p-1, 0x1.beea653e9c1c9p-57},
            {0x1.c600040b9f7aep-1, -0x1.c079f274a70d6p-56},
            {0x1.ca0006255fd8ap-1, -0x1.a0b4076e84c1fp-56},
            {0x1.cdfffd94c095dp-1, 0x1.8f933f99ab5d7p-55},
            {0x1.d1ffff975d6cfp-1, -0x1.82c08665fe1bep-58},
            {0x1.d5fffa2561c93p-1, -0x1.b04289bd295f3p-56},
            {0x1.d9fff9d228b0cp-1, 0x1.70251340fa236p-55},
            {0x1.de00065bc7e16p-1, -0x1.5011e16a4d80cp-56},
            {0x1.e200002f64791p-1, 0x1.9802f09ef62ep-55},
            {0x1.e600057d7a6d8p-1, -0x1.e0b75580cf7fap-56},
            {0x1.ea00027edc00cp-1, -0x1.c848309459811p-55},
            {0x1.ee0006cf5cb7cp-1, -0x1.f8027951576f4p-55},
            {0x1.f2000782b7dccp-1, -0x1.f81d97274538fp-55},
            {0x1.f6000260c450ap-1, -0x1.071002727ffdcp-59},
            {0x1.f9fffe88cd533p-1, -0x1.81bdce1fda8bp-58},
            {0x1.fdfffd50f8689p-1, 0x1.7f91acb918e6ep-55},
            {0x1.0200004292367p+0, 0x1.b7ff365324681p-54},
            {0x1.05fffe3e3d668p+0, 0x1.6fa08ddae957bp-55},
            {0x1.0a0000a85a757p+0, -0x1.7e2de80d3fb91p-58},
            {0x1.0e0001a5f3fccp+0, -0x1.1823305c5f014p-54},
            {0x1.11ffff8afbaf5p+0, -0x1.bfabb6680bac2p-55},
            {0x1.15fffe54d91adp+0, -0x1.d7f121737e7efp-54},
            {0x1.1a00011ac36e1p+0, 0x1.c000a0516f5ffp-54},
            {0x1.1e00019c84248p+0, -0x1.082fbe4da5dap-54},
            {0x1.220000ffe5e6ep+0, -0x1.8fdd04c9cfb43p-55},
            {0x1.26000269fd891p+0, 0x1.cfe2a7994d182p-55},
            {0x1.2a00029a6e6dap+0, -0x1.00273715e8bc5p-56},
            {0x1.2dfffe0293e39p+0, 0x1.b7c39dab2a6f9p-54},
            {0x1.31ffff7dcf082p+0, 0x1.df1336edc5254p-56},
            {0x1.35ffff05a8b6p+0, -0x1.e03564ccd31ebp-54},
            {0x1.3a0002e0eaeccp+0, 0x1.5f0e74bd3a477p-56},
            {0x1.3e000043bb236p+0, 0x1.c7dcb149d8833p-54},
            {0x1.4200002d187ffp+0, 0x1.e08afcf2d3d28p-56},
            {0x1.460000d387cb1p+0, 0x1.20837856599a6p-55},
            {0x1.4a00004569f89p+0, -0x1.9fa5c904fbcd2p-55},
            {0x1.4e000043543f3p+0, -0x1.81125ed175329p-56},
            {0x1.51fffcc027f0fp+0, 0x1.883d8847754dcp-54},
            {0x1.55ffffd87b36fp+0, -0x1.709e731d02807p-55},
            {0x1.59ffff21df7bap+0, 0x1.7f79f68727b02p-55},
            {0x1.5dfffebfc3481p+0, -0x1.180902e30e93ep-54},
        },
#endif
};

#define T __log2_data.tab
#define T2 __log2_data.tab2
#define B __log2_data.poly1
#define A __log2_data.poly
#define InvLn2hi __log2_data.invln2hi
#define InvLn2lo __log2_data.invln2lo
#define OFF 0x3fe6000000000000

/* Top 16 bits of a double.  */
static inline uint32_t top16(double x) {
  return asuint64(x) >> 48;
}

double log2(double x) {
  double_t z, r, r2, r4, y, invc, logc, kd, hi, lo, t1, t2, t3, p;
  uint64_t ix, iz, tmp;
  uint32_t top;
  int k, i;

  ix = asuint64(x);
  top = top16(x);
#define LO asuint64(1.0 - 0x1.5b51p-5)
#define HI asuint64(1.0 + 0x1.6ab2p-5)
  if (predict_false(ix - LO < HI - LO)) {
    /* Handle close to 1.0 inputs separately.  */
    /* Fix sign of zero with downward rounding when x==1.  */
    if (predict_false(ix == asuint64(1.0)))
      return 0;
    r = x - 1.0;
#if __FP_FAST_FMA
    hi = r * InvLn2hi;
    lo = r * InvLn2lo + __builtin_fma(r, InvLn2hi, -hi);
#else
    double_t rhi, rlo;
    rhi = asdouble(asuint64(r) & -1ULL << 32);
    rlo = r - rhi;
    hi = rhi * InvLn2hi;
    lo = rlo * InvLn2hi + r * InvLn2lo;
#endif
    r2 = r * r; /* rounding error: 0x1p-62.  */
    r4 = r2 * r2;
    /* Worst-case error is less than 0.54 ULP (0.55 ULP without fma).  */
    p = r2 * (B[0] + r * B[1]);
    y = hi + p;
    lo += hi - y + p;
    lo += r4 * (B[2] + r * B[3] + r2 * (B[4] + r * B[5]) +
                r4 * (B[6] + r * B[7] + r2 * (B[8] + r * B[9])));
    y += lo;
    return eval_as_double(y);
  }
  if (predict_false(top - 0x0010 >= 0x7ff0 - 0x0010)) {
    /* x < 0x1p-1022 or inf or nan.  */
    if (ix * 2 == 0)
      return __math_divzero(1);
    if (ix == asuint64(INFINITY)) /* log(inf) == inf.  */
      return x;
    if ((top & 0x8000) || (top & 0x7ff0) == 0x7ff0)
      return __math_invalid(x);
    /* x is subnormal, normalize it.  */
    ix = asuint64(x * 0x1p52);
    ix -= 52ULL << 52;
  }

  /* x = 2^k z; where z is in range [OFF,2*OFF) and exact.
     The range is split into N subintervals.
     The ith subinterval contains z and c is near its center.  */
  tmp = ix - OFF;
  i = (tmp >> (52 - LOG2_TABLE_BITS)) % N;
  k = (int64_t)tmp >> 52; /* arithmetic shift */
  iz = ix - (tmp & 0xfffULL << 52);
  invc = T[i].invc;
  logc = T[i].logc;
  z = asdouble(iz);
  kd = (double_t)k;

  /* log2(x) = log2(z/c) + log2(c) + k.  */
  /* r ~= z/c - 1, |r| < 1/(2*N).  */
#if __FP_FAST_FMA
  /* rounding error: 0x1p-55/N.  */
  r = __builtin_fma(z, invc, -1.0);
  t1 = r * InvLn2hi;
  t2 = r * InvLn2lo + __builtin_fma(r, InvLn2hi, -t1);
#else
  double_t rhi, rlo;
  /* rounding error: 0x1p-55/N + 0x1p-65.  */
  r = (z - T2[i].chi - T2[i].clo) * invc;
  rhi = asdouble(asuint64(r) & -1ULL << 32);
  rlo = r - rhi;
  t1 = rhi * InvLn2hi;
  t2 = rlo * InvLn2hi + r * InvLn2lo;
#endif

  /* hi + lo = r/ln2 + log2(c) + k.  */
  t3 = kd + logc;
  hi = t3 + t1;
  lo = t3 - hi + t1 + t2;

  /* log2(r+1) = r/ln2 + r^2*poly(r).  */
  /* Evaluation is optimized assuming superscalar pipelined execution.  */
  r2 = r * r; /* rounding error: 0x1p-54/N^2.  */
  r4 = r2 * r2;
  /* Worst-case error if |y| > 0x1p-4: 0.547 ULP (0.550 ULP without fma).
     ~ 0.5 + 2/N/ln2 + abs-poly-error*0x1p56 ULP (+ 0.003 ULP without fma).  */
  p = A[0] + r * A[1] + r2 * (A[2] + r * A[3]) + r4 * (A[4] + r * A[5]);
  y = lo + r2 * p + hi;
  return eval_as_double(y);
}
void __dso_handle() {}
void __cxa_atexit() {}

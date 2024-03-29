#include <io.h>
#include <dos.h>
//系统日志打印
void kprint(char *str) {
  irq_mask_set(0);
  for (int i = 0; i < strlen(str); i++) {
    write_serial(str[i]);
  }
  irq_mask_clear(0);
}
void printk(char *str, ...) {
  char buf[1024];
  va_list ap;
  va_start(ap, str);
  vsprintf(buf, str, ap);
  va_end(ap);
  kprint(buf);
}

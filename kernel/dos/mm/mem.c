#include <dos.h>
#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000
unsigned int memtest_sub(unsigned int,unsigned int);
unsigned int memtest(unsigned int start, unsigned int end) {
  char flg486 = 0;
  unsigned int eflg, cr0, i;

  /* 确认CPU是386还是486以上的 */
  eflg = io_load_eflags();
  eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
  io_store_eflags(eflg);
  eflg = io_load_eflags();
  if ((eflg & EFLAGS_AC_BIT) != 0) {
    /* 如果是386，即使设定AC=1，AC的值还会自动回到0 */
    flg486 = 1;
  }

  eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
  io_store_eflags(eflg);

  if (flg486 != 0) {
    cr0 = load_cr0();
    cr0 |= CR0_CACHE_DISABLE; /* 禁止缓存 */
    store_cr0(cr0);
  }

  i = memtest_sub(start, end);

  if (flg486 != 0) {
    cr0 = load_cr0();
    cr0 &= ~CR0_CACHE_DISABLE; /* 允许缓存 */
    store_cr0(cr0);
  }
  return i;
}

void *malloc(int size) {
  void *p = page_malloc(size + sizeof(int));
  if (p == NULL)
    return NULL;
  *(int *)p = size;
  return (char *)p + sizeof(int);
}
void free(void *p) {
  if (p == NULL)
    return;
  int size = *(int *)((char *)p - sizeof(int));
//  printk("size = %d\n",size);
  page_free((char *)p - sizeof(int), size + sizeof(int));
}

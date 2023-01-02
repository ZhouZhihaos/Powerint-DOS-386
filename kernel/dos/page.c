#include <dos.h>

struct PAGE_INFO *pages = PAGE_MANNAGER;
void C_init_page() {
  //	set_segmdesc(gdt + 101, 4096, AR_DATA32_RW);
  //	set_segmdesc(gdt + 102, 4 * 1024 * 1024, AR_DATA32_RW);
  memset(pages, 0, 1024 * 1024);
  memset(PDE_ADDRESS, 0, PAGE_END - PDE_ADDRESS);
  // 这是初始化PDE 页目录
  for (int addr = PDE_ADDRESS, i = PTE_ADDRESS | PG_P | PG_USU | PG_RWW;
       addr != PTE_ADDRESS; addr += 4, i += 0x1000) {
    *(int *)(addr) = i;
  }
  // 这是初始化PTE 页表
  for (int addr = PTE_ADDRESS, i = PG_P | PG_USU | PG_RWW; addr != PAGE_END;
       addr += 4, i += 0x1000) {
    *(int *)(addr) = i;
  }
  // 将物理地址0x0000~0x901000占用
  for (int i = 0x0000 / (4 * 1024); i < 0x901000 / (4 * 1024); i++) {
    pages[i].flag = 1;
  }
  // 将物理地址0xc0000000~0xffffffff占用
  for (int i = 0xc0000000 / (4 * 1024); i < 0xffffffff / (4 * 1024); i++) {
    pages[i].flag = 1;
  }
  return;
}
void pf_set(unsigned int memsize) {
  uint32_t *pte = PTE_ADDRESS;
  for (int i = 0; pte != PAGE_END; pte++, i++) {
    if (i >= memsize / 4096 && i <= 0xc0000000 / 4096) {
      *pte &= 0xffffffff - 1;
    }
  }
}
int get_line_address(int t, int p, int o) {
  //获取线性地址
  // t:页目录地址
  // p:页表地址
  // o:页内偏移地址
  return (t << 22) + (p << 12) + o;
}
int get_page_from_line_address(int line_address) {
  int t, p, page;
  t = line_address >> 22;
  p = (line_address >> 12) & 0x3ff;
  tpo2page(&page, t, p);
  return page;
}
void page2tpo(int page, int *t, int *p) {
  *t = page / 1024;
  *p = page % 1024;
}
void tpo2page(int *page, int t, int p) { *page = (t * 1024) + p; }
void *page_malloc_one() {
  int i;
  for (i = 0; i != 1024 * 1024; i++) {
    if (pages[i].flag == 0) {
      int t, p;
      page2tpo(i, &t, &p);
      unsigned int addr = get_line_address(t, p, 0);
      pages[i].flag = 1;
      pages[i].task_id = NowTask()->sel / 8 - 103;
      return (void *)addr;
    }
  }
  return NULL;
}
int get_pageinpte_address(int t, int p) {
  int page;
  tpo2page(&page, t, p);
  return (PTE_ADDRESS + page * 4);
}
void page_free_one(void *p) {
  if (get_page_from_line_address((int)p) >= 1024 * 1024)
    return;
  pages[get_page_from_line_address((int)p)].flag = 0;
  pages[get_page_from_line_address((int)p)].task_id = 0;
}
int find_kpage(int line, int n) {
  int free = 0;
  // 找一个连续的线性地址空间
  for (; line != 1024 * 1024; line++) {
    if (pages[line].flag == 0) {
      free++;
    } else {
      free = 0;
    }
    if (free == n) {
      for (int j = line - n; j != line + 1; j++) {
        pages[j].flag = 1;
        pages[j].task_id = NowTask()->sel / 8 - 103;
      }
      line -= n - 1;
      break;
    }
  }
  return line;
}
void *page_kmalloc(int size) {
  Maskirq(0);
  int n = ((size - 1) / (4 * 1024)) + 1;
  int i = find_kpage(0, n);
  int t, p;
  page2tpo(i, &t, &p);
  ClearMaskIrq(0);
  // printk("KMALLOC LINE ADDR:%08x PHY ADDR:%08x SIZE:%d PAGENUM:%d\n",
  //        get_line_address(t, p, 0), *(int *)get_pageinpte_address(t, p) -
  //        0x7, size, n);
  clean(get_line_address(t, p, 0), n * 4 * 1024);
  return (void *)get_line_address(t, p, 0);
}
void page_kfree(int p, int size) {
  Maskirq(0);
  int n = ((size - 1) / (4 * 1024)) + 1;
  p = (int)p & 0xfffff000;
  // printk("KFREE ADDR:%08x SIZE:%d PAGE NUM:%d ", p, size, n);
  for (int i = 0; i < n; i++) {
    page_free_one(p);
    p += 0x1000;
    // printk("FREE PAGE:%d ",get_page_from_line_address(p));
  }
  // printk("\n");
  ClearMaskIrq(0);
}
void *get_phy_address_for_line_address(void *line) {
  int t, p;
  page2tpo(get_page_from_line_address(line), &t, &p);
  return *(int *)get_pageinpte_address(t, p);
}
void set_phy_address_for_line_address(void *line, void *phy) {
  int t, p;
  page2tpo(get_page_from_line_address(line), &t, &p);
  *(int *)get_pageinpte_address(t, p) = (int)phy;
}
// 映射地址
void page_map(void *target, void *start, void *end) {
  io_cli();
  target = (int)target & 0xfffff000;
  start = (int)start & 0xfffff000;
  end = (int)end & 0xfffff000;
  uint32_t n = (int)end - (int)start;
  n /= 4 * 1024;
  n++;
  for (uint32_t i = 0; i < n; i++) {
    uint32_t tmp = get_phy_address_for_line_address(target + i * 4 * 1024);
    uint32_t tmp2 = get_phy_address_for_line_address(start + i * 4 * 1024);
    set_phy_address_for_line_address(target + i * 4 * 1024, tmp2);
    set_phy_address_for_line_address(start + i * 4 * 1024, tmp);
  }
  io_sti();
}
void *page_malloc_lessthan4kb(int size) {
  if (size >= 4 * 1024) // "less than 4 kb"
    return page_kmalloc(size);
  struct TASK *task = NowTask();
  char *memman = task->memman;
  int alloc_addr = task->alloc_addr;
  if (memman == NULL || alloc_addr == NULL) // 你没有alloc_addr和memman还来？
    return page_malloc_one();               // 一个页给你滚蛋
  int len, i;
  for (len = ((size - 1) / 128 + 1), i = 0; i < 4 * 1024; i++) {
    if (len == 0) {
      for (len = 0; len < ((size - 1) / 128 + 1); len++) {
        memman[i - 1 - len] = 0xff;
      }
      // printk("Malloc Less than 4kb:%08x 128 Bytes Pagenum:%d\n",
      //        (alloc_addr + (i - ((size - 1) / 128 + 1)) * 128),
      //        ((size - 1) / 128 + 1));
      return (void *)(alloc_addr + (i - ((size - 1) / 128 + 1)) * 128);
    }
    if (memman[i] == 0x00) {
      len--;
    } else if (memman[i] == 0xff && len != 0) {
      len = ((size - 1) / 128 + 1);
    }
  }
  return page_malloc_one(); // 没位置给你了 一个页对不起
}
void page_free_lessthan4kb(void *p, int size) {
  if (size > 4 * 1024) {
    page_kfree(p, size);
    return;
  }
  struct TASK *task = NowTask();
  char *memman = task->memman;
  int alloc_addr = task->alloc_addr;
  if (memman == NULL || alloc_addr == NULL) {
    page_kfree(p, size);
    return;
  }
  size = ((size - 1) + 128) / 128;
  int ad = (int)p & 0xfffffff0;
  p = (ad - alloc_addr) / 128;
  if (p > 4 * 1024)
    return;
  for (int i = p; i < size + p; i++) {
    memman[i] = 0x00;
  }
  clean((char *)(ad), size * 128);
  // printk("Free less than 4kb:%08x 128 Bytes Pagenum:%d\n",ad,size);
}
void *page_malloc(int size) {
  if (size > 0 && size < 4 * 1024) {
    void *p = page_malloc_lessthan4kb(size);
    return p;
  } else if (size >= 4 * 1024) {
    void *p = page_kmalloc(size);
    return p;
  }
}
void page_free(void *p, int size) {
  struct TASK *task = NowTask();
  if (size > 0 && size < 4 * 1024 &&
      task->alloc_addr <= p <= task->alloc_addr + 512 * 1024) {
    page_free_lessthan4kb(p, size);
  } else {
    page_kfree(p, size);
  }
  return;
}
void change_page_task_id(int task_id, void *p, unsigned int size) {
  int page = get_page_from_line_address(p);
  for (int i = 0; i != ((size - 1) / (4 * 1024)) + 1; i++) {
    // int t,p1;
    // page2tpo(page+i,&t,&p1);
    // printk("Change Page:%d Address:%08x Size:%d TaskID:%d ->
    // %d\n",page+i,get_line_address(t,p1,0),size,pages[page+i].task_id,task_id);
    pages[page + i].task_id = task_id;
  }
}
void showPage() {
  uint32_t *pte = PTE_ADDRESS;
  // printk("size = %d", sizeof(struct PTE_page_table));
  for (int i = 0; pte != PAGE_END; pte++, i++) {
    printf("LINE ADDRESS: %08x PHY ADDRESS: %08x P=%d RW=%d US=%d USING=%d "
           "TASK=%d\n",
           i * 4096, (*pte >> 12) << 12, ((*pte) << 31) >> 31,
           ((*pte) << 30) >> 31, ((*pte) << 29) >> 31, pages[i].flag,
           pages[i].task_id);
    //*pte &= 0xffffffff-1;
  }
}
void PF(uint32_t eax) {
  void *line_address = eax;
  if (!pages[(uint32_t)line_address >> 12].flag) {
    return;
  }
  // printf("LINE ADDRESS: %08x\n", line_address);
  uint32_t *pte = PTE_ADDRESS;
  void *phy_address = page_malloc_one();
  while ((pte[(uint32_t)phy_address >> 12] & 1) != 1) {
    phy_address = page_malloc_one();
  }
  page_free_one(phy_address);
  // printf("PHY ADDRESS: %08x\n", phy_address);
  page_map(line_address, phy_address, phy_address + 4096 - 1);
  uint32_t i = (uint32_t)phy_address >> 12;
  uint32_t j = (uint32_t)line_address >> 12;
  pte[j] |= 1;
  pte[i] &= 0xffffffff - 1;
  /*printf("LINE ADDRESS: %08x PHY ADDRESS: %08x P=%d RW=%d US=%d USING=%d "
         "TASK=%d\n",
         i * 4096, (pte[(uint32_t)phy_address >> 12] >> 12) << 12,
         ((pte[(uint32_t)phy_address >> 12]) << 31) >> 31,
         ((pte[(uint32_t)phy_address >> 12]) << 30) >> 31,
         ((pte[(uint32_t)phy_address >> 12]) << 29) >> 31, pages[i].flag,
         pages[i].task_id);
  printf("LINE ADDRESS: %08x PHY ADDRESS: %08x P=%d RW=%d US=%d USING=%d "
         "TASK=%d\n",
         j * 4096, (pte[(uint32_t)line_address >> 12] >> 12) << 12,
         ((pte[(uint32_t)line_address >> 12]) << 31) >> 31,
         ((pte[(uint32_t)line_address >> 12]) << 30) >> 31,
         ((pte[(uint32_t)line_address >> 12]) << 29) >> 31, pages[j].flag,
         pages[j].task_id);*/
  return;
}

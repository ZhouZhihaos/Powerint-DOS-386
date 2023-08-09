#include <dos.h>

struct PAGE_INFO *pages = (struct PAGE_INFO *)PAGE_MANNAGER;
void C_init_page() {
  //	set_segmdesc(gdt + 101, 4096, AR_DATA32_RW);
  //	set_segmdesc(gdt + 102, 4 * 1024 * 1024, AR_DATA32_RW);
  memset(pages, 0, 1024 * 1024);
  memset((void *)PDE_ADDRESS, 0, PAGE_END - PDE_ADDRESS);
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
  uint32_t *pte = (uint32_t *)PTE_ADDRESS;
  for (int i = 0; pte != (uint32_t *)PAGE_END; pte++, i++) {
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
      pages[i].task_id = current_task()->sel / 8 - 103;
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
unsigned get_shell_tid(struct TASK *task) {
  if (task->app == 0) {
    return get_tid(task);
  }
  if (task->app == 1) {
    return get_shell_tid(task->thread.father);
  }
  return 0;
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
        pages[j].task_id = get_shell_tid(current_task());
      }
      line -= n - 1;
      break;
    }
  }
  return line;
}
void *page_malloc(int size) {
  irq_mask_set(0);
  int n = ((size - 1) / (4 * 1024)) + 1;
  int i = find_kpage(0, n);
  int t, p;
  page2tpo(i, &t, &p);
  irq_mask_clear(0);
  clean((char *)get_line_address(t, p, 0), n * 4 * 1024);
  return (void *)get_line_address(t, p, 0);
}
void page_free(void *p, int size) {
  irq_mask_set(0);
  int n = ((size - 1) / (4 * 1024)) + 1;
  p = (int)p & 0xfffff000;
  for (int i = 0; i < n; i++) {
    page_free_one((void *)p);
    p += 0x1000;
  }
  irq_mask_clear(0);
}
void *get_phy_address_for_line_address(void *line) {
  int t, p;
  page2tpo(get_page_from_line_address((int)line), &t, &p);
  return (void *)(*(int *)get_pageinpte_address(t, p));
}
void set_phy_address_for_line_address(void *line, void *phy) {
  int t, p;
  page2tpo(get_page_from_line_address((int)line), &t, &p);
  *(int *)get_pageinpte_address(t, p) = (int)phy;
}
// 映射地址
void page_map(void *target, void *start, void *end) {
  io_cli();
  target = (void *)((int)target & 0xfffff000);
  start = (void *)((int)start & 0xfffff000);
  end = (void *)((int)end & 0xfffff000);
  uint32_t n = (int)end - (int)start;
  n /= 4 * 1024;
  n++;
  for (uint32_t i = 0; i < n; i++) {
    uint32_t tmp = (uint32_t)get_phy_address_for_line_address(
        (void *)((uint32_t)target + i * 4 * 1024));
    uint32_t tmp2 = (uint32_t)get_phy_address_for_line_address(
        (void *)((uint32_t)start + i * 4 * 1024));
    set_phy_address_for_line_address((void *)((uint32_t)target + i * 4 * 1024),
                                     (void *)tmp2);
    set_phy_address_for_line_address((void *)((uint32_t)start + i * 4 * 1024),
                                     (void *)tmp);
  }
  io_sti();
}
void change_page_task_id(int task_id, void *p, unsigned int size) {
  int page = get_page_from_line_address((int)p);
  for (int i = 0; i != ((size - 1) / (4 * 1024)) + 1; i++) {
    pages[page + i].task_id = task_id;
  }
}
void showPage() {
  uint32_t *pte = (uint32_t *)PTE_ADDRESS;
  // printk("size = %d", sizeof(struct PTE_page_table));
  for (int i = 0; pte != (uint32_t *)PAGE_END; pte++, i++) {
    printf("LINE ADDRESS: %08x PHY ADDRESS: %08x P=%d RW=%d US=%d USING=%d "
           "TASK=%d\n",
           i * 4096, (*pte >> 12) << 12, ((*pte) << 31) >> 31,
           ((*pte) << 30) >> 31, ((*pte) << 29) >> 31, pages[i].flag,
           pages[i].task_id);
    //*pte &= 0xffffffff-1;
  }
}
void PF(uint32_t eax) {
  void *line_address = (void *)eax;
  if (!pages[(uint32_t)line_address >> 12].flag) {
    return;
  }
  // printf("LINE ADDRESS: %08x\n", line_address);
  uint32_t *pte = (uint32_t *)PTE_ADDRESS;
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

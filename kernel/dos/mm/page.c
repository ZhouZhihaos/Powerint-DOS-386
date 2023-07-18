#include <dos.h>

struct PAGE_INFO* pages = (struct PAGE_INFO*)PAGE_MANNAGER;
void C_init_page() {
  //	set_segmdesc(gdt + 101, 4096, AR_DATA32_RW);
  //	set_segmdesc(gdt + 102, 4 * 1024 * 1024, AR_DATA32_RW);
  memset(pages, 0, 1024 * 1024);
  memset((void*)PDE_ADDRESS, 0, PAGE_END - PDE_ADDRESS);
  // 这是初始化PDE 页目录
  for (int addr = PDE_ADDRESS, i = PTE_ADDRESS | PG_P | PG_USU | PG_RWW;
       addr != PTE_ADDRESS; addr += 4, i += 0x1000) {
    *(int*)(addr) = i;
  }
  // 这是初始化PTE 页表
  for (int addr = PTE_ADDRESS, i = PG_P | PG_USU | PG_RWW; addr != PAGE_END;
       addr += 4, i += 0x1000) {
    *(int*)(addr) = i;
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
  uint32_t* pte = (uint32_t*)PTE_ADDRESS;
  for (int i = 0; pte != (uint32_t*)PAGE_END; pte++, i++) {
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
void page2tpo(int page, int* t, int* p) {
  *t = page / 1024;
  *p = page % 1024;
}
void tpo2page(int* page, int t, int p) {
  *page = (t * 1024) + p;
}
void* page_malloc_one() {
  int i;
  for (i = 0; i != 1024 * 1024; i++) {
    if (pages[i].flag == 0) {
      int t, p;
      page2tpo(i, &t, &p);
      unsigned int addr = get_line_address(t, p, 0);
      pages[i].flag = 1;
      pages[i].task_id = current_task()->sel / 8 - 103;
      return (void*)addr;
    }
  }
  return NULL;
}
int get_pageinpte_address(int t, int p) {
  int page;
  tpo2page(&page, t, p);
  return (PTE_ADDRESS + page * 4);
}
void page_free_one(void* p) {
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
        pages[j].task_id = current_task()->sel / 8 - 103;
      }
      line -= n - 1;
      break;
    }
  }
  return line;
}
void* page_kmalloc(int size) {
  Maskirq(0);
  int n = ((size - 1) / (4 * 1024)) + 1;
  int i = find_kpage(0, n);
  int t, p;
  page2tpo(i, &t, &p);
  ClearMaskIrq(0);
  // printk("KMALLOC LINE ADDR:%08x PHY ADDR:%08x SIZE:%d PAGENUM:%d\n",
  //        get_line_address(t, p, 0), *(int *)get_pageinpte_address(t, p) -
  //        0x7, size, n);
  clean((char*)get_line_address(t, p, 0), n * 4 * 1024);
  return (void*)get_line_address(t, p, 0);
}
void page_kfree(int p, int size) {
  Maskirq(0);
  int n = ((size - 1) / (4 * 1024)) + 1;
  p = (int)p & 0xfffff000;
  // printk("KFREE ADDR:%08x SIZE:%d PAGE NUM:%d ", p, size, n);
  for (int i = 0; i < n; i++) {
    page_free_one((void*)p);
    p += 0x1000;
    // printk("FREE PAGE:%d ",get_page_from_line_address(p));
  }
  // printk("\n");
  ClearMaskIrq(0);
}
void* get_phy_address_for_line_address(void* line) {
  int t, p;
  page2tpo(get_page_from_line_address((int)line), &t, &p);
  return (void*)(*(int*)get_pageinpte_address(t, p));
}
void set_phy_address_for_line_address(void* line, void* phy) {
  int t, p;
  page2tpo(get_page_from_line_address((int)line), &t, &p);
  *(int*)get_pageinpte_address(t, p) = (int)phy;
}
// 映射地址
void page_map(void* target, void* start, void* end) {
  io_cli();
  target = (void*)((int)target & 0xfffff000);
  start = (void*)((int)start & 0xfffff000);
  end = (void*)((int)end & 0xfffff000);
  uint32_t n = (int)end - (int)start;
  n /= 4 * 1024;
  n++;
  for (uint32_t i = 0; i < n; i++) {
    uint32_t tmp = (uint32_t)get_phy_address_for_line_address(
        (void*)((uint32_t)target + i * 4 * 1024));
    uint32_t tmp2 = (uint32_t)get_phy_address_for_line_address(
        (void*)((uint32_t)start + i * 4 * 1024));
    set_phy_address_for_line_address((void*)((uint32_t)target + i * 4 * 1024),
                                     (void*)tmp2);
    set_phy_address_for_line_address((void*)((uint32_t)start + i * 4 * 1024),
                                     (void*)tmp);
  }
  io_sti();
}
void memman_init(struct MEMMAN* man) {
  man->frees = 0;    /* 可用信息数目 */
  man->maxfrees = 0; /* 用于观察可用状况：frees的最大值 */
  man->lostsize = 0; /* 释放失败的内存的大小总和 */
  man->losts = 0;    /* 释放失败次数 */
  return;
}

unsigned int memman_total(struct MEMMAN* man)
/* 报告空余内存大小的合计 */
{
  unsigned int i, t = 0;
  for (i = 0; i < man->frees; i++) {
    t += man->free[i].size;
  }
  return t;
}

unsigned int memman_alloc(struct MEMMAN* man, unsigned int size)
/* 分配 */
{
  unsigned int i, a;
  for (i = 0; i < man->frees; i++) {
    if (man->free[i].size >= size) {
      /* 找到了足够大的内存 */
      a = man->free[i].addr;
      man->free[i].addr += size;
      man->free[i].size -= size;
      if (man->free[i].size == 0) {
        /* 如果free[i]变成了0，就减掉一条可用信息 */
        man->frees--;
        for (; i < man->frees; i++) {
          man->free[i] = man->free[i + 1]; /* 代入结构体 */
        }
      }
      clean((char*)a, size);
      return a;
    }
  }
  printk("no more memmory! %d %08x\n", size, ((uint32_t*)man)[-1]);
  return 0; /* 没有可用空间 */
}

int memman_free(struct MEMMAN* man, unsigned int addr, unsigned int size)
/* 释放 */
{
  int i, j;
  /* 为便于归纳内存，将free[]按照addr的顺序排列 */
  /* 所以，先决定应该放在哪里 */
  for (i = 0; i < man->frees; i++) {
    if (man->free[i].addr > addr) {
      break;
    }
  }
  /* free[i - 1].addr < addr < free[i].addr */
  if (i > 0) {
    /* 前面有可用内存 */
    if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
      /* 可以与前面的可用内存归纳到一起 */
      man->free[i - 1].size += size;
      if (i < man->frees) {
        /* 后面也有 */
        if (addr + size == man->free[i].addr) {
          /* 也可以与后面的可用内存归纳到一起 */
          man->free[i - 1].size += man->free[i].size;
          /* man->free[i]删除 */
          /* free[i]变成0后归纳到前面去 */
          man->frees--;
          for (; i < man->frees; i++) {
            man->free[i] = man->free[i + 1]; /* 结构体赋值 */
          }
        }
      }
      return 0; /* 成功完成 */
    }
  }
  /* 不能与前面的可用空间归纳到一起 */
  if (i < man->frees) {
    /* 后面还有 */
    if (addr + size == man->free[i].addr) {
      /* 可以与后面的内容归纳到一起 */
      man->free[i].addr = addr;
      man->free[i].size += size;
      return 0; /* 成功完成 */
    }
  }
  /* 既不能与前面归纳到一起，也不能与后面归纳到一起 */
  if (man->frees < MEMMAN_FREES) {
    /* free[i]之后的，向后移动，腾出一点可用空间 */
    for (j = man->frees; j > i; j--) {
      man->free[j] = man->free[j - 1];
    }
    man->frees++;
    if (man->maxfrees < man->frees) {
      man->maxfrees = man->frees; /* 更新最大值 */
    }
    man->free[i].addr = addr;
    man->free[i].size = size;
    return 0; /* 成功完成 */
  }
  /* 不能往后移动 */
  man->losts++;
  man->lostsize += size;
  return -1; /* 失败 */
}
unsigned int memman_alloc_128b(struct MEMMAN* man, unsigned int size) {
  unsigned int a;
  size = (size + 0x7f) & 0xffffff80;
  a = memman_alloc(man, size);
  return a;
}

int memman_free_128b(struct MEMMAN* man, unsigned int addr, unsigned int size) {
  int i;
  size = (size + 0x7f) & 0xffffff80;
  i = memman_free(man, addr, size);
  return i;
}
void init_mem(struct TASK* task) {
  if (task->alloc_addr == 0 || task->alloc_size == 0) {
    return;
  }
  unsigned int start = (unsigned int)task->alloc_addr;
  task->mm = (struct MEMMAN *)start;
  memman_init((struct MEMMAN*)start);
  start += sizeof(struct MEMMAN);
  memman_free(task->mm, start, task->alloc_size - sizeof(struct MEMMAN));
}
void* page_malloc_lessthan4kb(int size) {
  if (current_task()->mm != NULL) {
    void* p = (void *)memman_alloc_128b(current_task()->mm, size);
    return p;
  }
  return page_kmalloc(size);
}
void page_free_lessthan4kb(void* p, int size) {
  if (p == NULL || current_task()->mm == NULL) {
    page_kfree((int)p, size);
    return;
  }
  memman_free_128b(current_task()->mm, (int)p, size);
}
void* page_malloc(int size) {
  if (size > 0 && size < 4 * 1024) {
    void* p = page_malloc_lessthan4kb(size);
    return p;
  } else if (size >= 4 * 1024) {
    void* p = page_kmalloc(size);
    return p;
  }
  return NULL;
}
void page_free(void* p, int size) {
  struct TASK* task = current_task();
  if (size > 0 && size < 4 * 1024 && task->alloc_addr <= (int)p &&
      p <= (void*)((uint32_t)task->alloc_addr + task->alloc_addr)) {
    //printk("free --- %d\n", size);
    page_free_lessthan4kb(p, size);
  } else {
    page_kfree((int)p, size);
  }
  return;
}
void change_page_task_id(int task_id, void* p, unsigned int size) {
  int page = get_page_from_line_address((int)p);
  for (int i = 0; i != ((size - 1) / (4 * 1024)) + 1; i++) {
    // int t,p1;
    // page2tpo(page+i,&t,&p1);
    // printk("Change Page:%d Address:%08x Size:%d TaskID:%d ->
    // %d\n",page+i,get_line_address(t,p1,0),size,pages[page+i].task_id,task_id);
    pages[page + i].task_id = task_id;
  }
}
void showPage() {
  uint32_t* pte = (uint32_t*)PTE_ADDRESS;
  // printk("size = %d", sizeof(struct PTE_page_table));
  for (int i = 0; pte != (uint32_t*)PAGE_END; pte++, i++) {
    printf(
        "LINE ADDRESS: %08x PHY ADDRESS: %08x P=%d RW=%d US=%d USING=%d "
        "TASK=%d\n",
        i * 4096, (*pte >> 12) << 12, ((*pte) << 31) >> 31,
        ((*pte) << 30) >> 31, ((*pte) << 29) >> 31, pages[i].flag,
        pages[i].task_id);
    //*pte &= 0xffffffff-1;
  }
}
void PF(uint32_t eax) {
  void* line_address = (void*)eax;
  if (!pages[(uint32_t)line_address >> 12].flag) {
    return;
  }
  // printf("LINE ADDRESS: %08x\n", line_address);
  uint32_t* pte = (uint32_t*)PTE_ADDRESS;
  void* phy_address = page_malloc_one();
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

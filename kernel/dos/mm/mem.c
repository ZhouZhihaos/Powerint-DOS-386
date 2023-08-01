#include <dos.h>
#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000
unsigned int memtest_sub(unsigned int, unsigned int);
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
    // it's ok, so we do nothing
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
            mem_free_finf(mem, finf, (void *)start, e - start);
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
    //  printk("1 %d %08x\n", flag, if_nomore);
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
memory* memory_init(uint32_t start, uint32_t size) {
  memory* mem;
  mem = (memory*)start;
  start += sizeof(memory);
  mem->freeinf = (freeinfo*)start;
  start += sizeof(freeinfo);
  mem->freeinf->next = 0;
  mem->freeinf->f = (free_member*)start;
  start += FREE_MAX_NUM * sizeof(free_member);
  for (int i = 0; i < FREE_MAX_NUM; i++) {
    mem->freeinf->f[i].start = 0;
    mem->freeinf->f[i].end = 0;
  }
  mem_free(mem, (void *)start, size);
  return mem;
}

void *malloc(int size) {
  void *p;
  if (current_task()->mm != NULL && size < 4 * 1024 - sizeof(int)) {
    p = mem_alloc(current_task()->mm, size + sizeof(int));
    if (current_task()->mm->memerrno != ERRNO_NOPE) {
      p = page_malloc(size + sizeof(int));
    }
  } else {
    p = page_malloc(size + sizeof(int));
  }
  if (p == NULL)
    return NULL;
  *(int *)p = size;
  return (char *)p + sizeof(int);
}
void free(void *p) {
  if (p == NULL)
    return;
  int size = *(int *)((char *)p - sizeof(int));
  if (p >= current_task()->alloc_addr &&
      (char *)p <=
          (char *)current_task()->alloc_addr + current_task()->alloc_size) {
    mem_free(current_task()->mm, (char *)p - sizeof(int), size + sizeof(int));
  } else {
    page_free((char *)p - sizeof(int), size + sizeof(int));
  }
}
void* realloc(void* ptr, uint32_t size) {
  void* new = malloc(size);
  if (ptr) {
    memcpy(new, ptr, *(int*)((int)ptr - 4));
    free(ptr);
  }
  return new;
}

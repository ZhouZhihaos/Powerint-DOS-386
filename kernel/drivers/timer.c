#include <dos.h>
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC 1 /* 已配置状态 */
#define TIMER_FLAGS_USING 2 /* 定时器运行中 */
extern int cg_flag0;
extern struct TASK* c_task;
void init_pit(void) {
  __init_PIT();
  int i;
  struct TIMER* t;
  timerctl.count = 0;
  for (i = 0; i < MAX_TIMER; i++) {
    timerctl.timers0[i].flags = 0; /* 没有使用 */
  }
  t = timer_alloc(); /* 取得一个 */
  t->timeout = 0xffffffff;
  t->flags = TIMER_FLAGS_USING;
  t->next = 0;     /* 末尾 */
  timerctl.t0 = t; /* 因为现在只有哨兵，所以他就在最前面*/
  timerctl.next =
      0xffffffff; /* 因为只有哨兵，所以下一个超时时刻就是哨兵的时刻 */
  return;
}

struct TIMER* timer_alloc(void) {
  int i;
  for (i = 0; i < MAX_TIMER; i++) {
    if (timerctl.timers0[i].flags == 0) {
      timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
      return &timerctl.timers0[i];
    }
  }
  return 0; /* 没找到 */
}

void timer_free(struct TIMER* timer) {
  timer->flags = 0; /* 未使用 */
  return;
}

void timer_init(struct TIMER* timer, struct FIFO8* fifo, unsigned char data) {
  timer->fifo = fifo;
  timer->data = data;
  return;
}

void timer_settime(struct TIMER* timer, unsigned int timeout) {
  int e;
  struct TIMER *t, *s;
  timer->timeout = timeout + timerctl.count;
  timer->flags = TIMER_FLAGS_USING;
  e = io_load_eflags();
  io_cli();
  t = timerctl.t0;
  if (timer->timeout <= t->timeout) {
    /* 插入最前面的情况 */
    timerctl.t0 = timer;
    timer->next = t; /* 下面是设定t */
    timerctl.next = timer->timeout;
    io_store_eflags(e);
    return;
  }
  for (;;) {
    s = t;
    t = t->next;
    if (timer->timeout <= t->timeout) {
      /* 插入s和t之间的情况 */
      s->next = timer; /* s下一个是timer */
      timer->next = t; /* timer的下一个是t */
      io_store_eflags(e);
      return;
    }
  }
}

void sleep(int s) {
  struct TIMER* timer;
  timer = timer_alloc();
  unsigned char buf[50];
  struct FIFO8 fifo;
  fifo8_init(&fifo, 50, buf);
  timer_init(timer, &fifo, 1);
  timer_settime(timer, s);
  for (;;) {
    int i = fifo8_get(&fifo);
    if (i == 1) {
      timer_free(timer);
      return;
    }
  }
}

uint32_t mt2flag = 0;
static uint32_t count = 0;
void inthandler20(int* esp) {
  // printk("*");
  extern struct TIMER *mt_timer1, *mt_timer2, *mt_timer3;
  extern int tasknum;
  struct TIMER* timer;
  io_out8(PIC0_OCW2, 0x60); /* 把IRQ-00接收信号结束的信息通知给PIC */
  timerctl.count++;
  if (timerctl.next > timerctl.count) {
    return;
  }
  timer = timerctl.t0; /* 首先把最前面的地址赋给timer */
  char ts = 0;
  for (;;) {
    /* 因为timers的定时器都处于运行状态，所以不确认flags */
    if (timer->timeout > timerctl.count) {
      break;
    }
    /* 超时 */
    timer->flags = TIMER_FLAGS_ALLOC;
    if (timer == mt_timer3) {  // mt_timer3超时
      mt2flag = 0;
      if (count == 3) {
        mt2flag = 1;
        count = 0;
      }
      ts = 3;
      count++;
    } else if (timer == mt_timer2) {  // mt_timer2超时
      ts = 2;
    } else if (timer == mt_timer1) {  // mt_timer1超时
      ts = 1;
    } else {
      fifo8_put(timer->fifo, timer->data);
    }
    timer = timer->next; /* 将下一个定时器的地址赋给timer*/
  }
  timerctl.t0 = timer;
  timerctl.next = timer->timeout;

  // ClearExpFlag();
  // disableExp();
  extern int st_task;

  if (ts == 3) {
    mt_taskswitch3();
  }
  if (ts == 2) {
    mt_taskswitch2();
  }
  if (ts == 1) {
    mt_taskswitch1();
  }
  if (NowTask()->fpu_use == 1 && NowTask()->app == 1) {
    io_cli();
    printk("switch %s\n",NowTask()->name);
    asm volatile("frstor %0" ::"m"(NowTask()->fxsave_region));
    NowTask()->fpu_use = 0;
    io_sti();
  } else {
    //asm volatile("fninit");
  }
  // if(GetExpFlag()) {
  //   printk("Warning: an Error for CPU!\n");
  // }
  // ClearExpFlag();
  // EnableExp();
}

// rtl8139.c
// Realtek rtl8139 网卡驱动
// Copyright (C) zhouzhihao 2023
#include <dos.h>
#define CARD_VENDOR_ID 0x10EC
#define CARD_DEVICE_ID 0x8139
#define MAC0 0x00
#define MAC1 0x01
#define MAC2 0x02
#define MAC3 0x03
#define MAC4 0x04
#define MAC5 0x05
#define MAR 0x08
#define RBSTART 0x30
#define CAPR 0x38
#define CMD 0x37
#define IMR 0x3C
#define ISR 0x3E
#define CONFIG_1 0x52
#define TCR 0x40
#define RCR 0x44
#define TSAD0 0x20
#define TSD0 0x10
void RTL8139_ASM_INTHANDLER(void);
static uint8_t bus, dev, func;
static uint32_t io_base;
extern uint8_t mac0, mac1, mac2, mac3, mac4, mac5;
static uint8_t *sendBuffer[4];
static uint8_t *recvBuffer;
static uint8_t currentSendBuffer;
static void set_handler(int IRQ, int addr) {
  // 注册中断
  struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;
  set_gatedesc(idt + 0x20 + IRQ, (int)addr, 2 * 8, AR_INTGATE32);
  ClearMaskIrq(IRQ);
}
static void init_Card_all() {
  recvBuffer = (uint8_t *)malloc(8192 + 16);
  for (int i = 0; i != 4; i++) {
    sendBuffer[i] = (uint8_t *)malloc(8192 + 16);
  }

  mac0 = io_in8(io_base + MAC0);
  mac1 = io_in8(io_base + MAC1);
  mac2 = io_in8(io_base + MAC2);
  mac3 = io_in8(io_base + MAC3);
  mac4 = io_in8(io_base + MAC4);
  mac5 = io_in8(io_base + MAC5);
  // printk("MAC:%02x:%02x:%02x:%02x:%02x:%02x\n", mac0, mac1, mac2, mac3, mac4,
  //        mac5);

  Maskirq(0);
  io_out8(io_base + CONFIG_1, 0x00);          // 激活RTL8139网卡
  io_out8(io_base + CMD, 0x10);               // 复位
  while ((io_in8(io_base + CMD) & 0x10) != 0) // 等待RST高1变为0（复位成功）
    ;
  io_out16(io_base + CAPR, 0);
  io_out32(io_base + RBSTART, recvBuffer); // 设置接收缓冲区地址
  for (int i = 0; i != 4; i++) {
    io_out32(io_base + TSAD0 + i * 4, 0);
  }
  io_out8(io_base + CMD, 0x0c); // 允许传输和接收
  io_out32(io_base + TCR,
           (1 << 16) | (3 << 24) | (0 << 4) | (7 << 8) | (0 << 17)); // 设置TCR
  io_out32(io_base + RCR,
           (8 << 24) | (5 << 13) | (3 << 11) | (7 << 8) | 0xf); // 设置RCR
  io_out16(io_base + IMR, 0x0005); // 设置IMR 允许接收完成和传输完成的中断

  currentSendBuffer = 0;

  printf("RTL8139 INIT DONE\n");


  ClearMaskIrq(0);

  // 初始化ARP表
}
void Rtl8139Recv() {
  // printk("RECV: ");
  // for (int i = 0; i != 128; i++) {
  //   printk("%02x ", recvBuffer[i]);
  // }
  // printk("\n");
  Card_Recv_Handler(recvBuffer + 4);
}
void Rtl8139Send(uint8_t *buffer, int size) {
  if (size > 1792) {
    size = 1792 - 4;
  } else if (size < 64) {
    size = 64 - 4;
  }
  clean(sendBuffer[currentSendBuffer], size + 1 - 4);
  memcpy(sendBuffer[currentSendBuffer], buffer, size - 4);

  // printk("SENDING: ");
  // for (int i = 0; i != size - 4; i++) {
  //   printk("%02x ", sendBuffer[currentSendBuffer][i]);
  // }
  // printk("\n");

  io_out32(io_base + TSAD0 + 4 * currentSendBuffer,
           sendBuffer[currentSendBuffer]);
  io_out32(io_base + TSD0 + 4 * currentSendBuffer, size - 4);
  currentSendBuffer++;
  currentSendBuffer %= 4;
}
bool rtl8139_find_card() {
	bus = 255;
  PCI_GET_DEVICE(CARD_VENDOR_ID, CARD_DEVICE_ID, &bus, &dev, &func);
  if (bus == 255) {
    return false;
  }
  return true;
}
void init_rtl8139_card() {
  set_handler(PCI_Get_Drive_IRQ_LINE(bus, dev, func),
              (int)RTL8139_ASM_INTHANDLER);
  uint32_t conf = PCI_READ_COMMAND_STATUS(bus, dev, func);
  conf &= 0xffff0000; // 保留STATUS寄存器，清除COMMAND寄存器
  conf |= 0x7;        // 设置第0~2位（允许PCNET网卡产生中断
  PCI_WRITE_COMMAND_STATUS(bus, dev, func, conf);
  io_base = PCI_Get_PORT_Base(bus, dev, func);
  init_Card_all();
}
void RTL8139_IRQ() {
  uint16_t temp = io_in16(io_base + ISR);
  if ((temp & 0x0004) == 0x0004) {
    // printk("RTL8139 SEND\n");
  } else if ((temp & 0x0001) == 0x0001) {
    // printk("RTL8139 RECV\n");
    Rtl8139Recv();
  }

  io_out16(io_base + ISR, temp);

  // printk("RTL8139 IRQ\n");
  // 通知PIC中断处理完毕
  io_out8(PIC1_OCW2, (0x60 + PCI_Get_Drive_IRQ_LINE(bus, dev, func) - 0x8));
  io_out8(PIC0_OCW2, 0x62);
}

// pcnet.c
// AMD pcnet 网卡驱动
// Copyright (C) zhouzhihao & min0911_ 2022
#include <dos.h>
#include <drivers.h>
extern int PCI_ADDR_BASE;
void PCNET_ASM_INTHANDLER(void);
#define CARD_VENDOR_ID 0x1022
#define CARD_DEVICE_ID 0x2000
//  为了使用该寄存器，必须将RAP设置为这些值
#define CSR0 0
#define CSR1 1
#define CSR2 2
#define CSR3 3
#define CSR4 4
#define BCR18 18
#define BCR20 20
// 16位I/O端口（或到I/O空间开始的偏移）。
#define APROM0 0x00
#define APROM1 0x01
#define APROM2 0x02
#define APROM3 0x03
#define APROM4 0x04
#define APROM5 0x05
// 16位读写模式下
#define RDP16 0x10
#define RAP16 0x12
#define RESET16 0x14
#define BDP16 0x16
// 32位读写模式下
#define RDP32 0x10
#define RAP32 0x14
#define RESET32 0x18
#define BDP32 0x1c

extern uint32_t gateway, submask, dns, ip, dhcp_ip;

struct InitializationBlock initBlock;
uint8_t mac0, mac1, mac2, mac3, mac4, mac5;

static int io_base = 0;
static struct BufferDescriptor *sendBufferDesc;
static uint8_t sendBufferDescMemory[2048 + 15];
static uint8_t sendBuffers[8][2048 + 15];
static uint8_t currentSendBuffer;
static struct BufferDescriptor *recvBufferDesc;
static uint8_t recvBufferDescMemory[2048 + 15];
static uint8_t recvBuffers[8][2048 + 15];
static uint8_t currentRecvBuffer;

static uint8_t *IP_Packet_Base[16] = {NULL, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, NULL};
static uint32_t Find_IP_Packet(uint16_t ident) {
  for (int i = 0; i != 16; i++) {
    if (IP_Packet_Base[i] != NULL) {
      struct IPV4Message *ipv4 =
          (struct IPV4Message *)(IP_Packet_Base[i] +
                                 sizeof(struct EthernetFrame_head));
      if (swap16(ipv4->ident) == ident) {
        return i;
      }
    }
  }
  return -1;
}
static void set_handler(int IRQ, int addr) {
  // 注册中断
  struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;
  set_gatedesc(idt + 0x20 + IRQ, (int)addr, 2 * 8, AR_INTGATE32);
  ClearMaskIrq(IRQ);
}
void into_32bitsRW() {
  // 切换到32位读写模式 DWIO（BCR18,bit7）=1
  // 此时还处于16位读写模式
  // 读取BCR18
  io_out16(io_base + RAP16, BCR18);
  uint16_t tmp = io_in16(io_base + BDP16);
  tmp |= 0x80; // DWIO（bit7）=1
  // 写入BCR18
  io_out16(io_base + RAP16, BCR18);
  io_out16(io_base + BDP16, tmp);
  // 此时就处于32位读写模式了
}
void into_16bitsRW() {
  // 切换到16位读写模式 与切换到32位读写模式相反
  io_out32(io_base + RAP32, BCR18);
  uint32_t tmp = io_in32(io_base + BDP32);
  tmp &= ~0x80;
  io_out32(io_base + RAP32, BCR18);
  io_out32(io_base + BDP32, tmp);
}
void reset_card() {
  // PCNET卡复位（约等于切换到16位读写模式
  io_in16(io_base + RESET16);
  io_out16(io_base + RESET16, 0x00);
  // 执行完后需等待（sleep(1)）
}
void Activate() {
  // 激活PCNET IRQ中断
  io_out16(io_base + RAP16, CSR0);
  io_out16(io_base + RDP16, 0x41);

  io_out16(io_base + RAP16, CSR4);
  uint32_t temp = io_in16(io_base + RDP16);
  io_out16(io_base + RAP16, CSR4);
  io_out16(io_base + RDP16, temp | 0xc00);

  io_out16(io_base + RAP16, CSR0);
  io_out16(io_base + RDP16, 0x42);
}
void init_Card_all() {
  currentSendBuffer = 0;
  currentRecvBuffer = 0;

  // 获取MAC地址并保存
  mac0 = io_in8(io_base + APROM0);
  mac1 = io_in8(io_base + APROM1);
  mac2 = io_in8(io_base + APROM2);
  mac3 = io_in8(io_base + APROM3);
  mac4 = io_in8(io_base + APROM4);
  mac5 = io_in8(io_base + APROM5);
  printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac0, mac1, mac2, mac3,
         mac4, mac5);
  // 这里约等于 into_16bitsRW();
  ClearMaskIrq(0);
  reset_card();
  sleep(1);
  Maskirq(0);

  io_out16(io_base + RAP16, BCR20);
  io_out16(io_base + BDP16, 0x102);
  io_out16(io_base + RAP16, CSR0);
  io_out16(io_base + RDP16, 0x0004); // 暂时停止所有传输（用于初始化PCNET网卡

  // initBlock传输初始化（CSR1=IB地址低16位，CSR2=IB地址高16位）
  // &
  // Send/Recv环形缓冲区的初始化
  initBlock.mode = 0;
  initBlock.reserved1numSendBuffers =
      (0 << 4) | 3; // 高4位是reserved1 低4位是numSendBuffers
  initBlock.reserved2numRecvBuffers =
      (0 << 4) | 3; // 高4位是reserved2 低4位是numRecvBuffers
  initBlock.mac0 = mac0;
  initBlock.mac1 = mac1;
  initBlock.mac2 = mac2;
  initBlock.mac3 = mac3;
  initBlock.mac4 = mac4;
  initBlock.mac5 = mac5;
  initBlock.reserved3 = 0;
  initBlock.logicalAddress = 0;

  sendBufferDesc =
      (struct BufferDescriptor *)(((uint32_t)&sendBufferDescMemory[0] + 15) &
                                  0xfffffff0);
  initBlock.sendBufferDescAddress = (uint32_t)sendBufferDesc;
  recvBufferDesc =
      (struct BufferDescriptor *)(((uint32_t)&recvBufferDescMemory[0] + 15) &
                                  0xfffffff0);
  initBlock.recvBufferDescAddress = (uint32_t)recvBufferDesc;

  for (uint8_t i = 0; i < 8; i++) {
    sendBufferDesc[i].address = (((uint32_t)&sendBuffers[i] + 15) & 0xfffffff0);
    sendBufferDesc[i].flags = 0xf7ff;
    sendBufferDesc[i].flags2 = 0;
    sendBufferDesc[i].avail = 0;

    recvBufferDesc[i].address = (((uint32_t)&recvBuffers[i] + 15) & 0xfffffff0);
    recvBufferDesc[i].flags = 0xf7ff | 0x80000000;
    recvBufferDesc[i].flags2 = 0;
    recvBufferDesc[i].avail = 0;
    clean(recvBufferDesc[i].address, 2048);
  }
  // CSR1,CSR2赋值（initBlock地址
  io_out16(io_base + RAP16, CSR1);
  io_out16(io_base + RDP16, (uint16_t)&initBlock);
  io_out16(io_base + RAP16, CSR2);
  io_out16(io_base + RDP16, (uint32_t)&initBlock >> 16);

  Activate();

  initBlock.logicalAddress = 0xFFFFFFFF;
  ip = 0xFFFFFFFF;
  gateway = 0xFFFFFFFF;
  submask = 0xFFFFFFFF;
  dns = 0xFFFFFFFF;
  dhcp_discovery(&mac0);
  while (initBlock.logicalAddress == 0xFFFFFFFF && gateway == 0xFFFFFFFF &&
         submask == 0xFFFFFFFF && dns == 0xFFFFFFFF && ip == 0xFFFFFFFF)
    ;

  ClearMaskIrq(0);

  // 初始化ARP表
  for (uint8_t i = 1; i != 0; i++) {
    IPParseMAC((ip & 0xffffff00) | i);
  }

  // DNSParseIP("baidu.com");

  // UDPProviderSend(0x761ff8d7, initBlock.logicalAddress, 52949, 38,
  //   "来自Powerint DOS 386的消息：我是周志昊！！！", strlen("来自Powerint DOS
  //   386的消息：我是周志昊！！！"));
}
uint8_t bus = 255, dev = 255, func = 255;
void init_pcnet_card() {
  Maskirq(0);
  PCI_GET_DEVICE(CARD_VENDOR_ID, CARD_DEVICE_ID, &bus, &dev, &func);
  if (bus == 255) {
    printf("pcnet card not found.\n");
    ClearMaskIrq(0);
    return;
  }
  // 允许PCNET网卡产生中断
  // 1.注册中断
  set_handler(PCI_Get_Drive_IRQ_LINE(bus, dev, func),
              (int)PCNET_ASM_INTHANDLER);
  // 2,写COMMAND和STATUS寄存器
  uint32_t conf = PCI_READ_COMMAND_STATUS(bus, dev, func);
  conf &= 0xffff0000; // 保留STATUS寄存器，清除COMMAND寄存器
  conf |= 0x7;        // 设置第0~2位（允许PCNET网卡产生中断
  PCI_WRITE_COMMAND_STATUS(bus, dev, func, conf);
  io_base = PCI_Get_PORT_Base(bus, dev, func);
  init_Card_all();
}
static void IP_Assembling(struct IPV4Message *ipv4) {
  uint32_t i_p = Find_IP_Packet(swap16(ipv4->ident));
  struct IPV4Message *ipv4_p =
      (struct IPV4Message *)(IP_Packet_Base[i_p] +
                             sizeof(struct EthernetFrame_head));
  uint32_t size_p = swap16(ipv4_p->totalLength);
  ipv4_p->totalLength =
      swap16(swap16(ipv4->totalLength) + swap16(ipv4_p->totalLength) -
             sizeof(struct IPV4Message));
  IP_Packet_Base[i_p] = (uint8_t *)realloc((void *)IP_Packet_Base[i_p],
                                           swap16(ipv4_p->totalLength));
  memcpy((void *)(IP_Packet_Base[i_p] + size_p),
         recvBufferDesc[currentRecvBuffer].address +
             sizeof(struct EthernetFrame_head) + sizeof(struct IPV4Message),
         swap16(ipv4->totalLength) - sizeof(struct IPV4Message));
  return;
}
void Recv() {
  // printf("\nPCNET RECV: ");
  for (; (recvBufferDesc[currentRecvBuffer].flags & 0x80000000) == 0;
       currentRecvBuffer = (currentRecvBuffer + 1) % 8) {
    if (!(recvBufferDesc[currentRecvBuffer].flags & 0x40000000) &&
        (recvBufferDesc[currentRecvBuffer].flags & 0x03000000) == 0x03000000) {
      uint32_t size = recvBufferDesc[currentRecvBuffer].flags & 0xfff;
      if (size > 128)
        size -= 4;

      uint8_t *buffer = (uint8_t *)(recvBufferDesc[currentRecvBuffer].address);
      for (int i = 0; i < (size > 128 ? 128 : size); i++) {
        // printf("%02x ", buffer[i]);
      }
      // printf("\n");
    }

    struct EthernetFrame_head *header =
        (struct EthernetFrame_head *)(recvBufferDesc[currentRecvBuffer]
                                          .address);
    if (header->type == swap16(IP_PROTOCOL)) { // IP数据报
      struct IPV4Message *ipv4 =
          (struct IPV4Message *)(recvBufferDesc[currentRecvBuffer].address +
                                 sizeof(struct EthernetFrame_head));
      if (ipv4->version == 4) {
        if ((swap16(ipv4->flagsAndOffset) >> IP_MF) & 1) {
          if (Find_IP_Packet(swap16(ipv4->ident)) == -1) {
            for (int i = 0; i != 16; i++) {
              if (IP_Packet_Base[i] == NULL) {
                IP_Packet_Base[i] =
                    (uint8_t *)malloc(swap16(ipv4->totalLength) +
                                      sizeof(struct EthernetFrame_head));
                memcpy((void *)IP_Packet_Base[i],
                       recvBufferDesc[currentRecvBuffer].address,
                       swap16(ipv4->totalLength) +
                           sizeof(struct EthernetFrame_head));
                break;
              }
            }
          } else {
            IP_Assembling(ipv4);
          }
        } else if (!((swap16(ipv4->flagsAndOffset) >> IP_MF) & 1)) {
          uint32_t i_p = Find_IP_Packet(swap16(ipv4->ident));
          void *base = recvBufferDesc[currentRecvBuffer].address;
          if (i_p != -1) {
            IP_Assembling(ipv4);
            base = (void *)IP_Packet_Base[i_p];
          }
          if (ipv4->protocol == ICMP_PROTOCOL) { // ICMP
            icmp_handler(base);
          } else if (ipv4->protocol == UDP_PROTOCOL) { // UDP
            udp_handler(base);
          } else if (ipv4->protocol == TCP_PROTOCOL) { // TCP
            tcp_handler(base);
          }
          if (i_p != -1) {
            free((void *)IP_Packet_Base[i_p]);
            IP_Packet_Base[i_p] = NULL;
          }
        }
      }
    } else if (header->type == swap16(ARP_PROTOCOL)) { // ARP
      arp_handler(recvBufferDesc[currentRecvBuffer].address);
    }

    clean(recvBufferDesc[currentRecvBuffer].address, 2048);
    recvBufferDesc[currentRecvBuffer].flags2 = 0;
    recvBufferDesc[currentRecvBuffer].flags = 0x8000f7ff;
  }
  currentRecvBuffer = 0;
}
void Send(uint8_t *buffer, int size) {
  int sendDesc = currentSendBuffer;
  currentSendBuffer = (currentSendBuffer + 1) % 8;
  clean(sendBufferDesc[currentSendBuffer].address, 2048);
  if (size > MTU + sizeof(struct EthernetFrame_head) +
                 sizeof(struct EthernetFrame_tail))
    size = MTU + sizeof(struct EthernetFrame_head) +
           sizeof(struct EthernetFrame_tail);

  for (uint8_t *src = buffer + size - 1,
               *dst = (uint8_t *)(sendBufferDesc[sendDesc].address + size - 1);
       src >= buffer; src--, dst--)
    *dst = *src;

  // printf("SENDING: ");
  for (int i = 0; i < (size > 128 ? 128 : size); i++) {
    // printf("%02x ", buffer[i]);
  }
  // printf("\n");
  sendBufferDesc[sendDesc].avail = 0;
  sendBufferDesc[sendDesc].flags = 0x8300f000 | ((uint16_t)((-size) & 0xfff));
  sendBufferDesc[sendDesc].flags2 = 0;

  io_out16(io_base + RAP16, CSR0);
  io_out16(io_base + RDP16, 0x48);

  currentSendBuffer = 0;
}
void PCNET_IRQ(int *esp) {
  // printf("PCNET IRQ:%02x ", (0x20 + PCI_Get_Drive_IRQ_LINE(bus, dev,
  // func)));

  io_out16(io_base + RAP16, CSR0);
  uint16_t temp = io_in16(io_base + RDP16);

  // if ((temp & 0x8000) == 0x8000)
  //   printf("PCNET ERROR\n");
  // else if ((temp & 0x2000) == 0x2000)
  //   printf("PCNET COLLISION ERROR\n");
  // else if ((temp & 0x1000) == 0x1000)
  //   printf("PCNET MISSED FRAME\n");
  // else if ((temp & 0x0800) == 0x0800)
  //   printf("PCNET MEMORY ERROR\n");
  /*else*/ if ((temp & 0x0400) == 0x0400)
    Recv();
  // else if ((temp & 0x0200) == 0x0200)
  //   printf("PCNET SEND\n");

  io_out16(io_base + RAP16, CSR0);
  io_out16(io_base + RDP16, temp); // 通知PCNET网卡 中断处理完毕

  if ((temp & 0x0100) == 0x0100)
    printf("PCNET INIT DONE\n");

  // 通知PIC中断处理完毕
  io_out8(PIC1_OCW2, (0x60 + PCI_Get_Drive_IRQ_LINE(bus, dev, func) - 0x8));
  io_out8(PIC0_OCW2, 0x62);
  return;
}

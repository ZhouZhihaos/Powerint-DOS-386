#include <dos.h>
static struct Socket *wait[10] = {NULL, NULL, NULL, NULL, NULL,
                                  NULL, NULL, NULL, NULL, NULL};
static void *data[10];
static uint32_t size[10];
static void Handler_UDP(struct Socket *socket, void *base) {
  struct IPV4Message *ipv4 =
      (struct IPV4Message *)(base + sizeof(struct EthernetFrame_head));
  struct UDPMessage *udp =
      (struct UDPMessage *)(base + sizeof(struct EthernetFrame_head) +
                            sizeof(struct IPV4Message));
  void *dat = base + sizeof(struct EthernetFrame_head) +
              sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
  uint32_t total_size = swap16(ipv4->totalLength) - sizeof(struct IPV4Message) -
                        sizeof(struct UDPMessage);
  for (uint32_t i = 0; i != 10; i++) {
    if (socket == wait[i]) {
      uint32_t size0;
      if (size[i] > total_size) {
        size0 = total_size;
      } else {
        size0 = size[i];
      }
      memcpy(data[i], dat, size0);
      wait[i] = NULL;
      data[i] = NULL;
      size[i] = 0;
      break;
    }
  }
}
static void Handler_TCP(struct Socket *socket, void *base) {
  struct IPV4Message *ipv4 =
      (struct IPV4Message *)(base + sizeof(struct EthernetFrame_head));
  struct TCPMessage *tcp =
      (struct TCPMessage *)(base + sizeof(struct EthernetFrame_head) +
                            sizeof(struct IPV4Message));
  void *dat = base + sizeof(struct EthernetFrame_head) +
              sizeof(struct IPV4Message) + (tcp->headerLength * 4);
  uint32_t total_size = swap16(ipv4->totalLength) - sizeof(struct IPV4Message) -
                        (tcp->headerLength * 4);
  for (uint32_t i = 0; i != 10; i++) {
    if (socket == wait[i]) {
      uint32_t size0;
      if (size[i] > total_size) {
        size0 = total_size;
      } else {
        size0 = size[i];
      }
      memcpy(data[i], dat, size0);
      wait[i] = NULL;
      data[i] = NULL;
      size[i] = 0;
      break;
    }
  }
}
void net_API(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx,
             int eax) {
  struct TASK *task = NowTask();
  int cs_base = task->cs_base;
  int ds_base = task->ds_base;
  int alloc_addr = task->alloc_addr; // malloc地址
  if (eax == 0x01) {                 // Socket
    struct Socket *socket = Socket_Alloc(ebx);
    if (ebx == UDP_PROTOCOL) {
      Socket_Bind(socket, Handler_UDP);
    } else if (ebx == TCP_PROTOCOL) {
      Socket_Bind(socket, Handler_TCP);
    }
    intreturn(socket, ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x02) {
    Socket_Free((struct Socket *)ebx);
  } else if (eax == 0x03) {
    struct Socket *socket = (struct Socket *)ebx;
    socket->Send(socket, (uint8_t *)(ds_base + ecx), edx);
  } else if (eax == 0x04) {
    uint32_t t;
    for (uint32_t i = 0; i != 10; i++) {
      if (wait[i] == NULL) {
        t = i;
        break;
      }
    }
    wait[t] = (struct Socket *)ebx;
    data[t] = (void *)(ds_base + ecx);
    size[t] = edx;
    while (wait[t] != NULL)
      ;
  } else if (eax == 0x05) {
    struct Socket *socket = (struct Socket *)ebx;
    Socket_Init(socket, ecx, edx, esi, edi);
  } else if (eax == 0x06) {
    extern uint32_t ip;
    intreturn(ip, ebx, ecx, edx, esi, edi, ebp);
  } else if (eax == 0x07) {
    intreturn(ping(ebx), ebx, ecx, edx, esi, edi, ebp);
  }
}
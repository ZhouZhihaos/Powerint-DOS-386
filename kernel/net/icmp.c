#include <net.h>
// ICMP
static uint32_t ping_reply_flags = 1;
static uint8_t ping_reply_data[2048];
uint8_t *ICMP_Packet(uint8_t type, uint8_t code, uint16_t ID, uint16_t sequence,
                     uint8_t *data, uint32_t size) {
  struct ICMPMessage *res =
      (struct ICMPMessage *)page_malloc(sizeof(struct ICMPMessage) + size);
  uint8_t *dat = (uint8_t *)res;
  memcpy(dat + sizeof(struct ICMPMessage), data, size);
  res->type = type;
  res->code = code;
  res->ID = swap16(ID);
  res->sequence = swap16(sequence);
  res->checksum = 0;
  res->checksum = CheckSum((uint16_t *)dat, size + sizeof(struct ICMPMessage));
  return (uint8_t *)res;
}
void ICMPProviderSend(uint32_t destip, uint32_t srcip, uint8_t type,
                      uint8_t code, uint16_t ID, uint16_t sequence,
                      uint8_t *data, uint32_t size) {
  IPV4ProviderSend(1, IPParseMAC(destip), destip, srcip,
                   ICMP_Packet(type, code, ID, sequence, data, size),
                   size + sizeof(struct ICMPMessage));
}
int ping(uint32_t dstIP) {
  extern uint32_t ip;
  uint8_t data[PING_SIZE];
  for (int i = 0; i != PING_SIZE; i++)
    data[i] = PING_DATA;
  clean(ping_reply_data, 2048);
  ping_reply_flags = 1;
  ICMPProviderSend(dstIP, ip, 8, 0, PING_ID, PING_SEQ, data, PING_SIZE);
  extern struct TIMERCTL timerctl;
  uint32_t time = timerctl.count;
  while (ping_reply_flags) {
    if (timerctl.count - time > PING_WAITTIME) {
      return -1;
    }
  }
  struct ICMPMessage *icmp =
      (struct ICMPMessage *)(ping_reply_data +
                             sizeof(struct EthernetFrame_head) +
                             sizeof(struct IPV4Message));
  uint8_t *dat = ping_reply_data + sizeof(struct EthernetFrame_head) +
                 sizeof(struct IPV4Message) + sizeof(struct ICMPMessage);
  if (swap16(icmp->ID) != PING_ID || swap16(icmp->sequence) != PING_SEQ ||
      strncmp(dat, data, PING_SIZE) != 0) {
    return -1;
  }
  return 0;
}
void cmd_ping(char *cmdline) {
  uint32_t dst_ip;
  uint8_t ip0, ip1, ip2, ip3;
  if (cmdline[0] < '0' || cmdline[0] > '9') { // 不是数字
    uint8_t *dns = (uint8_t *)page_malloc(strlen(cmdline) + 1);
    memcpy(dns + 1, cmdline, strlen(cmdline));
    dst_ip = DNSParseIP(dns + 1);
    page_free(dns, strlen(cmdline) + 1);
  } else {
    dst_ip = IP2UINT32_T(cmdline);
  }
  ip0 = (uint8_t)(dst_ip >> 24);
  ip1 = (uint8_t)(dst_ip >> 16);
  ip2 = (uint8_t)(dst_ip >> 8);
  ip3 = (uint8_t)(dst_ip);
  printf("Pinging %d.%d.%d.%d from %d bytes data:\n\n", ip0, ip1, ip2, ip3,
         PING_SIZE + 2 + 2);
  uint8_t lost = 0, get = 4;
  extern uint32_t ip;
  uint64_t mac_address = IPParseMAC(dst_ip);
  uint8_t data[PING_SIZE];
  for (int i = 0; i != PING_SIZE; i++)
    data[i] = PING_DATA;
  uint8_t *packet = ICMP_Packet(8, 0, PING_ID, PING_SEQ, data, PING_SIZE);
  for (int i = 0; i != 4; i++) {
    ping_reply_flags = 1;
    clean(ping_reply_data, 2048);
    uint32_t time = timerctl.count;
    IPV4ProviderSend(1, mac_address, dst_ip, ip, packet,
                     PING_SIZE + sizeof(struct ICMPMessage));
    while (ping_reply_flags) {
      if (timerctl.count - time >= PING_WAITTIME) {
        printf("Reply timed out.\n");
        lost++;
        get--;
        break;
      }
    }
    time = timerctl.count - time;
    sleep(1000); // 不要着急 休息一下
    struct IPV4Message *ipv4 =
        (struct IPV4Message *)(ping_reply_data +
                               sizeof(struct EthernetFrame_head));
    struct ICMPMessage *icmp =
        (struct ICMPMessage *)(ping_reply_data +
                               sizeof(struct EthernetFrame_head) +
                               sizeof(struct IPV4Message));
    uint8_t *dat = ping_reply_data + sizeof(struct EthernetFrame_head) +
                   sizeof(struct IPV4Message) + sizeof(struct ICMPMessage);
    if (swap16(icmp->ID) == PING_ID && swap16(icmp->sequence) == PING_SEQ &&
        strncmp(dat, data, PING_SIZE) == 0) {
      printf("From %d.%d.%d.%d reply: bytes:%d time:%dms TTL:%d\n", ip0, ip1,
             ip2, ip3, PING_SIZE + 2 + 2, time, ipv4->timeToLive);
    }
  }
  printf("\nAll:4 packets Get:%d packets Lost:%d packets\n\n", get, lost);
}
void icmp_handler(void *base) {
  extern uint32_t ip;
  struct EthernetFrame_head *header = (struct EthernetFrame_head *)(base);
  struct IPV4Message *ipv4 =
      (struct IPV4Message *)(base + sizeof(struct EthernetFrame_head));
  struct ICMPMessage *icmp =
      (struct ICMPMessage *)(base + sizeof(struct EthernetFrame_head) +
                             sizeof(struct IPV4Message));
  if (icmp->type == 8 && icmp->code == 0) { // Ping请求
    // printf("ICMP PING Echo request\n");
    uint32_t size = (((ipv4->totalLength & 0xff00) >> 8) |
                     ((ipv4->totalLength & 0x00ff) << 8));
    size -= sizeof(struct IPV4Message);
    size -= sizeof(struct ICMPMessage);
    uint8_t *data = base + sizeof(struct EthernetFrame_head) +
                    sizeof(struct IPV4Message) + sizeof(struct ICMPMessage);
    uint32_t src_ip =
        ((ipv4->srcIP << 24) & 0xff000000) | ((ipv4->srcIP << 8) & 0x00ff0000) |
        ((ipv4->srcIP >> 8) & 0xff00) | ((ipv4->srcIP >> 24) & 0xff);
    uint8_t *packet =
        ICMP_Packet(0, 0, swap16(icmp->ID), swap16(icmp->sequence), data, size);
    IPV4ProviderSend(1, *(uint64_t *)&header->src_mac[0], src_ip, ip, packet,
                     size + sizeof(struct ICMPMessage));                 // 给答复
  } else if (icmp->type == 0 && icmp->code == 0) { // Ping答复
    // printf("ICMP PING Echo Reply\n");
    memcpy(ping_reply_data, base, 2048);
    ping_reply_flags = 0;
  }
}

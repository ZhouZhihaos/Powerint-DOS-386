#include <net.h>
// IPV4
void IPV4ProviderSend(uint8_t protocol, uint64_t dest_mac, uint32_t dest_ip,
                      uint32_t src_ip, uint8_t *data, uint32_t size,
                      uint16_t ident) {
  struct IPV4Message *res =
      (struct IPV4Message *)page_malloc(sizeof(struct IPV4Message) + size);
  uint8_t *dat = (uint8_t *)res;
  memcpy(dat + sizeof(struct IPV4Message), data, size);
  res->version = 4;
  res->headerLength = sizeof(struct IPV4Message) / 4;
  res->tos = 0;
  res->totalLength = sizeof(struct IPV4Message) + size;
  res->totalLength =
      ((res->totalLength & 0xff00) >> 8) | ((res->totalLength & 0x00ff) << 8);
  res->ident = ident;
  res->flagsAndOffset = 0;
  res->timeToLive = 64;
  res->protocol = protocol;
  res->dstIP = ((dest_ip << 24) & 0xff000000) | ((dest_ip << 8) & 0x00ff0000) |
               ((dest_ip >> 8) & 0xff00) | ((dest_ip >> 24) & 0xff);
  res->srcIP = ((src_ip << 24) & 0xff000000) | ((src_ip << 8) & 0x00ff0000) |
               ((src_ip >> 8) & 0xff00) | ((src_ip >> 24) & 0xff);
  res->checkSum = 0;
  res->checkSum = CheckSum((uint16_t *)dat, sizeof(struct IPV4Message));
  EtherFrameProviderSend(dest_mac, 0x0800, dat,
                         sizeof(struct IPV4Message) + size);
  page_free(dat, sizeof(struct IPV4Message) + size);
  return;
}
uint16_t CheckSum(uint16_t *data, uint32_t size) {
  uint32_t tmp = 0;
  for (int i = 0; i < size / 2; i++) {
    tmp += ((data[i] & 0xff00) >> 8) | ((data[i] & 0x00ff) << 8);
  }
  if (size % 2)
    tmp += ((uint16_t)((char *)data)[size - 1]) << 8;
  while (tmp & 0xffff0000)
    tmp = (tmp & 0xffff) + (tmp >> 16);
  return ((~tmp & 0xff00) >> 8) | ((~tmp & 0x00ff) << 8);
}
uint32_t IP2UINT32_T(uint8_t *ip) {
  uint8_t ip0, ip1, ip2, ip3;
  ip0 = strtol(ip, '.', 10);
  uint8_t t = ip0;
  while (t >= 10) {
    t /= 10;
    ip++;
  }
  ip1 = strtol(ip + 2, '.', 10);
  t = ip1;
  while (t >= 10) {
    t /= 10;
    ip++;
  }
  ip2 = strtol(ip + 4, '.', 10);
  t = ip2;
  while (t >= 10) {
    t /= 10;
    ip++;
  }
  ip3 = strtol(ip + 6, NULL, 10);
  return (uint32_t)((ip0 << 24) | (ip1 << 16) | (ip2 << 8) | ip3);
}

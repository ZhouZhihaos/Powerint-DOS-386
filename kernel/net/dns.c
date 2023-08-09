#include <net.h>
// DNS
uint32_t dns_parse_ip_result = 0;
uint32_t dns_parse_ip(uint8_t *name) {
  uint8_t *data =
      (uint8_t *)page_malloc(sizeof(struct DNS_Header) + strlen(name) + 2 +
                             sizeof(struct DNS_Question));
  struct DNS_Header *dns_header = (struct DNS_Header *)data;
  dns_header->ID = swap16(DNS_Header_ID);
  dns_header->QR = 0;     // 查询
  dns_header->Opcode = 0; // 标准查询
  dns_header->AA = 0;
  dns_header->TC = 0;
  dns_header->RD = 1; // 选择递归查询
  dns_header->RA = 0;
  dns_header->Z = 0;
  dns_header->RCODE = 0;
  dns_header->QDcount = swap16(1); // 1个Question
  dns_header->ANcount = 0;
  dns_header->NScount = 0;
  dns_header->ARcount = 0;
  dns_header->reserved = 0;
  uint8_t *new_name = data + sizeof(struct DNS_Header) - 1;
  name--;
  for (int i = 1, j = 0; i != strlen(name + 1) + 1; i++) {
    if (name[i] == '.') {
      name[j] = i - j - 1;
      j = i;
    } else if (i == strlen(name + 1)) {
      name[j] = i - j;
      j = i;
    }
  }
  memcpy(new_name, name, strlen(name) + 1);
  struct DNS_Question *dns_question =
      (struct DNS_Question *)(data + sizeof(struct DNS_Header) + strlen(name) +
                              1);
  dns_question->type = DNS_TYPE_A;
  dns_question->Class = DNS_CLASS_INET;
  extern uint32_t ip;
  udp_provider_send(DNS_SERVER_IP, ip, DNS_PORT, CHAT_CLIENT_PROT, data,
                  sizeof(struct DNS_Header) + strlen(name) + 1 +
                      sizeof(struct DNS_Question));
  dns_parse_ip_result = 0;
  while (dns_parse_ip_result == 0)
    ;
  return dns_parse_ip_result;
}
void dns_handler(void *base) {
  struct DNS_Header *dns_header =
      (struct DNS_Header *)(base + sizeof(struct EthernetFrame_head) +
                            sizeof(struct IPV4Message) +
                            sizeof(struct UDPMessage));
  if (swap16(dns_header->ID) == DNS_Header_ID) {
    uint8_t *p = (uint8_t *)(dns_header) + sizeof(struct DNS_Header);
    p += strlen(p) + sizeof(struct DNS_Question) - 1;
    struct DNS_Answer *dns_answer = (struct DNS_Answer *)p;
    dns_parse_ip_result = swap32(*(uint32_t *)&dns_answer->RData[0]);
  }
}

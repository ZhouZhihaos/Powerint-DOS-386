/* 网络粘合层 */
#include <dos.h>
extern uint8_t mac0, mac1, mac2, mac3, mac4, mac5;
unsigned long strtoul(const char* str, char** endptr, int base);
void Rtl8139Send(uint8_t* buffer, int size);
typedef struct {
  bool (*find)();
  void (*init)();
  void (*Send)(unsigned char* buffer, unsigned int size);
  char card_name[50];
  int use;  // 正在使用
  int flag;
} network_card;
bool pcnet_find_card();
network_card network_card_CTL[25];
static uint8_t* IP_Packet_Base[16] = {NULL, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, NULL};
static uint32_t Find_IP_Packet(uint16_t ident) {
  for (int i = 0; i != 16; i++) {
    if (IP_Packet_Base[i] != NULL) {
      struct IPV4Message* ipv4 =
          (struct IPV4Message*)(IP_Packet_Base[i] +
                                sizeof(struct EthernetFrame_head));
      if (swap16(ipv4->ident) == ident) {
        return i;
      }
    }
  }
  return -1;
}
static void IP_Assembling(struct IPV4Message* ipv4, unsigned char* RawData) {
  uint32_t i_p = Find_IP_Packet(swap16(ipv4->ident));
  struct IPV4Message* ipv4_p =
      (struct IPV4Message*)(IP_Packet_Base[i_p] +
                            sizeof(struct EthernetFrame_head));
  uint32_t size_p = swap16(ipv4_p->totalLength);
  ipv4_p->totalLength =
      swap16(swap16(ipv4->totalLength) + swap16(ipv4_p->totalLength) -
             sizeof(struct IPV4Message));
  IP_Packet_Base[i_p] = (uint8_t*)realloc((void*)IP_Packet_Base[i_p],
                                          swap16(ipv4_p->totalLength));
  memcpy(
      (void*)(IP_Packet_Base[i_p] + size_p),
      RawData + sizeof(struct EthernetFrame_head) + sizeof(struct IPV4Message),
      swap16(ipv4->totalLength) - sizeof(struct IPV4Message));
  return;
}
void init_networkCTL() {
  // printk("init_networkCTL");
  for (int i = 0; i < 25; i++) {
    // printk("Set %d\n",i);
    network_card_CTL[i].find = NULL;
    network_card_CTL[i].init = NULL;
    network_card_CTL[i].Send = NULL;
    network_card_CTL[i].use = 0;
    network_card_CTL[i].flag = 0;
  }
}

void register_network_card(network_card netCard) {
  for (int i = 0; i < 25; i++) {
    if (network_card_CTL[i].flag == 0) {
      network_card_CTL[i] = netCard;
      break;
    }
  }
}

void init_card() {
  // printk("init card\n");
  for (int i = 0; i < 25; i++) {
    if (network_card_CTL[i].flag == 0) {
      continue;
    }
    if (network_card_CTL[i].find()) {
      printf("Find --- %s\n", network_card_CTL[i].card_name);
      network_card_CTL[i].use = 1;
      network_card_CTL[i].init();
      extern uint32_t ip, gateway, submask, dns;

      ip = 0xFFFFFFFF;
      gateway = 0xFFFFFFFF;
      submask = 0xFFFFFFFF;
      dns = 0xFFFFFFFF;
      if (env_read("ip") == NULL) {
        dhcp_discovery(&mac0);
        while (gateway == 0xFFFFFFFF && submask == 0xFFFFFFFF &&
               dns == 0xFFFFFFFF && ip == 0xFFFFFFFF)
          ;
        
        char buf[100];
        sprintf(buf, "%x", ip);
        env_write("ip", buf);
        sprintf(buf, "%x", gateway);
        env_write("gateway", buf);
        sprintf(buf, "%x", submask);
        env_write("submask", buf);
        sprintf(buf, "%x", dns);
        env_write("dns", buf);
        
      } else {
        ip = strtoul(env_read("ip"),NULL,16);
        gateway = strtoul(env_read("gateway"),NULL,16);
        submask = strtoul(env_read("submask"),NULL,16);
        dns = strtoul(env_read("dns"),NULL,16);
      }
      
      for (uint8_t i = 1; i != 0; i++) {
        //printk("%d\n",i);
        IPParseMAC((ip & 0xffffff00) | i);
      }
      break;
    }
  }
}
void netcard_send(unsigned char* buffer, unsigned int size) {
  for (int i = 0; i < 25; i++) {
    if (network_card_CTL[i].use) {
      if (DriveSemaphoreTake(GetDriveCode("NETCARD_DRIVE"))) {
        // printf("Send....%s %d
        // %d\n",network_card_CTL[i].card_name,network_card_CTL[i].use,i);
        network_card_CTL[i].Send(buffer, size);
        DriveSemaphoreGive(GetDriveCode("NETCARD_DRIVE"));
        break;
      }
    }
  }
}

void Card_Recv_Handler(unsigned char* RawData) {
  struct EthernetFrame_head* header = (struct EthernetFrame_head*)(RawData);
  if (header->type == swap16(IP_PROTOCOL)) {  // IP数据报
    struct IPV4Message* ipv4 =
        (struct IPV4Message*)(RawData + sizeof(struct EthernetFrame_head));
    if (ipv4->version == 4) {
      if ((swap16(ipv4->flagsAndOffset) >> IP_MF) & 1) {
        if (Find_IP_Packet(swap16(ipv4->ident)) == -1) {
          for (int i = 0; i != 16; i++) {
            if (IP_Packet_Base[i] == NULL) {
              IP_Packet_Base[i] =
                  (uint8_t*)malloc(swap16(ipv4->totalLength) +
                                   sizeof(struct EthernetFrame_head));
              memcpy((void*)IP_Packet_Base[i], RawData,
                     swap16(ipv4->totalLength) +
                         sizeof(struct EthernetFrame_head));
              break;
            }
          }
        } else {
          IP_Assembling(ipv4, RawData);
        }
      } else if (!((swap16(ipv4->flagsAndOffset) >> IP_MF) & 1)) {
        uint32_t i_p = Find_IP_Packet(swap16(ipv4->ident));
        void* base = RawData;
        if (i_p != -1) {
          IP_Assembling(ipv4, RawData);
          base = (void*)IP_Packet_Base[i_p];
        }
        if (ipv4->protocol == ICMP_PROTOCOL) {  // ICMP
          icmp_handler(base);
        } else if (ipv4->protocol == UDP_PROTOCOL) {  // UDP
          udp_handler(base);
        } else if (ipv4->protocol == TCP_PROTOCOL) {  // TCP
          tcp_handler(base);
        }
        if (i_p != -1) {
          free((void*)IP_Packet_Base[i_p]);
          IP_Packet_Base[i_p] = NULL;
        }
      }
    }
  } else if (header->type == swap16(ARP_PROTOCOL)) {  // ARP
    arp_handler(RawData);
  }
}

void init_network() {
  // printk("init_network\n");
  network_card nc;
  strcpy(nc.card_name, "pcnet");
  nc.find = pcnet_find_card;
  nc.flag = 1;
  nc.init = init_pcnet_card;
  nc.Send = PcnetSend;
  nc.use = 0;
  register_network_card(nc);
  network_card rtl8139;
  strcpy(rtl8139.card_name, "rtl8139");
  rtl8139.find = rtl8139_find_card;
  rtl8139.flag = 1;
  rtl8139.init = init_rtl8139_card;
  rtl8139.Send = Rtl8139Send;
  rtl8139.use = 0;
  register_network_card(rtl8139);
}
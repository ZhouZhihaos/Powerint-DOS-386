#ifndef _NET_H_
#define _NET_H_
#include <define.h>
// 以太网帧
void EtherFrameProviderSend(uint64_t dest_mac, uint16_t type, uint8_t *buffer,
                            uint32_t size);
// ARP
uint8_t *ARP_Packet(uint64_t dest_mac, uint32_t dest_ip, uint64_t src_mac,
                    uint32_t src_ip, uint16_t command);
uint64_t IPParseMAC(uint32_t dstIP);
void arp_handler(void *base);
// IPV4
void IPV4ProviderSend(uint8_t protocol, uint64_t dest_mac, uint32_t dest_ip,
                      uint32_t src_ip, uint8_t *data, uint32_t size,
                      uint16_t ident);
uint16_t CheckSum(uint16_t *dat, uint32_t size);
uint32_t IP2UINT32_T(uint8_t *ip);
// ICMP
uint8_t *ICMP_Packet(uint8_t type, uint8_t code, uint16_t ID, uint16_t sequence,
                     uint8_t *data, uint32_t size);
void ICMPProviderSend(uint32_t destip, uint32_t srcip, uint8_t type,
                      uint8_t code, uint16_t ID, uint16_t sequence,
                      uint8_t *data, uint32_t size);
int ping(uint32_t dstIP);
void cmd_ping(char *cmdline);
void icmp_handler(void *base);
// UDP
uint8_t *UDP_Packet(uint16_t dest_port, uint16_t src_port, uint8_t *data,
                    uint32_t size);
void UDPProviderSend(uint32_t destip, uint32_t srcip, uint16_t dest_port,
                     uint16_t src_port, uint8_t *data, uint32_t size);
void udp_handler(void *base);
// DCHP
int dhcp_discovery(uint8_t *mac);
void dhcp_handler(void *base);
// DNS
uint32_t DNSParseIP(uint8_t *name);
void dns_handler(void *base);
// TCP
void TCPProviderSend(uint32_t dstIP, uint32_t srcIP, uint16_t dstPort,
                     uint16_t srcPort, uint32_t Sequence, uint32_t ackNum,
                     bool URG, bool ACK, bool PSH, bool RST, bool SYN, bool FIN,
                     bool ECE, bool CWR, uint8_t *data, uint32_t size);
void tcp_handler(void *base);
// Socket
struct Socket *Socket_Alloc(uint8_t protocol);
void Socket_Free(struct Socket *socket);
void Socket_Init(struct Socket *socket, uint32_t remoteIP, uint16_t remotePort,
                 uint32_t localIP, uint16_t localPort);
void Socket_Bind(struct Socket *socket, void (*Handler)());
struct Socket *Socket_Find(uint32_t dstIP, uint16_t dstPort, uint32_t srcIP,
                           uint16_t srcPort, uint8_t protocol);
struct SocketServer *
SocketServer_Alloc(void (*Handler)(struct Socket *socket, void *base),
                   uint32_t srcIP, uint16_t srcPort, uint8_t protocol);
void SocketServer_Free(struct SocketServer *server, uint8_t protocol);
uint32_t SocketServer_Status(struct SocketServer *server, uint8_t state);
// HTTP
bool IsHttpGetHeader(uint8_t* data, uint32_t size);
#endif

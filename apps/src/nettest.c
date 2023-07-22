#include <net.h>
#include <syscall.h>

int main() {
  if (ping(GetIP() & 0xffffff00 + 0x01) == -1) {
    printf("ICMP Packet error.");
    return 0;
  }
  socket_t socket = Socket_Alloc(UDP_PROTOCOL);
  Socket_Init(socket, GetIP() & 0xffffff00 + 0x01, 2115, GetIP(), 2115);
  unsigned char *buf = (unsigned char *)malloc(512);
  Socket_Recv(socket, buf, 512);
  printf("%s\n", buf);
  Socket_Free(socket);
  return 0;
}

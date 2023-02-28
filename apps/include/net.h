#ifndef __NET_H__
#define __NET_H__
#ifdef __cplusplus
extern "C" {
#endif
#define UDP_PROTOCOL 17
#define TCP_PROTOCOL 6
typedef int socket_t;
socket_t Socket_Alloc(unsigned int Protocol);
void Socket_Init(socket_t socket, unsigned int remoteIP,
                 unsigned short remotePort, unsigned int localIP,
                 unsigned short localPort);
void Socket_Free(socket_t socket);
void Socket_Send(socket_t socket, unsigned char *data, unsigned int size);
void Socket_Recv(socket_t socket, unsigned char *data, unsigned int size);
unsigned int GetIP();
int ping(unsigned int remoteIP);
#ifdef __cplusplus
}
#endif
#endif
#include <net.h>
// Socket
static struct Socket sockets[MAX_SOCKET_NUM];
static void Socket_UDP_Send(struct Socket *socket, uint8_t *data,
                            uint32_t size) {
  UDPProviderSend(socket->remoteIP, socket->localIP, socket->remotePort,
                  socket->localPort, data, size);
}
static void Socket_TCP_Send(struct Socket *socket, uint8_t *data,
                            uint32_t size) {
  for (int i = 0; i * socket->MSS < size; i++) {
    uint32_t s = ((int32_t)i * socket->MSS >= (int32_t)(size - socket->MSS))
                     ? (size - i * socket->MSS)
                     : socket->MSS;
    TCPProviderSend(socket->remoteIP, socket->localIP, socket->remotePort,
                    socket->localPort, socket->seqNum, socket->ackNum, 0, 1, 0,
                    0, 0, 0, 0, 0, data + i * socket->MSS, s);
    socket->seqNum += s;
    sleep(TCP_SEG_WAITTIME);
  }
}
static int Socket_TCP_Connect(struct Socket *socket) {
  // printf("send first shake.\n");
  // printf("socket->state: SOCKET_TCP_CLOSED -> SOCKET_TCP_SYN_SENT\n");
  socket->ackNum = 0;
  socket->seqNum = 0;
  socket->state = SOCKET_TCP_SYN_SENT;
  TCPProviderSend(socket->remoteIP, socket->localIP, socket->remotePort,
                  socket->localPort, socket->seqNum, socket->ackNum, 0, 0, 0, 0,
                  1, 0, 0, 0, 0, 0);
  socket->seqNum++;
  extern struct TIMERCTL timerctl;
  uint32_t time = timerctl.count;
  while (socket->state != SOCKET_TCP_ESTABLISHED) {
    if (timerctl.count - time > TCP_CONNECT_WAITTIME) {
      return -1;
    }
  }
  return 0;
}
static void Socket_TCP_Disconnect(struct Socket *socket) {
  // printf("send first wave.\n");
  // printf("socket->state: SOCKET_TCP_ESTABLISHED -> SOCKET_TCP_FIN_WAIT1\n");
  socket->state = SOCKET_TCP_FIN_WAIT1;
  TCPProviderSend(socket->remoteIP, socket->localIP, socket->remotePort,
                  socket->localPort, socket->seqNum, socket->ackNum, 0, 1, 0, 0,
                  0, 1, 0, 0, 0, 0);
  socket->seqNum++;
  while (socket->state != SOCKET_TCP_CLOSED)
    ;
}
static void Socket_TCP_Listen(struct Socket *socket) {
  // printf("Listening...\n");
  // printf("socket->state: SOCKET_TCP_CLOSED -> SOCKET_TCP_LISTEN\n");
  socket->state = SOCKET_TCP_LISTEN;
  while (socket->state != SOCKET_TCP_ESTABLISHED)
    ;
}
static void SocketServer_Send(struct SocketServer *server, uint8_t *data,
                              uint32_t size) {
  for (int i = 0; i != SOCKET_SERVER_MAX_CONNECT; i++) {
    if (server->socket[i]->state == SOCKET_TCP_ESTABLISHED) {
      server->socket[i]->Send(server->socket[i], data, size);
    }
  }
}
struct Socket *Socket_Alloc(uint8_t protocol) {
  for (int i = 0; i != MAX_SOCKET_NUM; i++) {
    if (sockets[i].state == SOCKET_FREE) {
      if (protocol == UDP_PROTOCOL) { // UDP
        sockets[i].state = SOCKET_ALLOC;
        sockets[i].protocol = UDP_PROTOCOL;
        sockets[i].Send = Socket_UDP_Send;
        sockets[i].Handler = NULL;
        return (struct Socket *)&sockets[i];
      } else if (protocol == TCP_PROTOCOL) { // TCP
        sockets[i].state = SOCKET_TCP_CLOSED;
        sockets[i].protocol = TCP_PROTOCOL;
        sockets[i].Send = Socket_TCP_Send;
        sockets[i].Connect = Socket_TCP_Connect;
        sockets[i].Disconnect = Socket_TCP_Disconnect;
        sockets[i].Listen = Socket_TCP_Listen;
        sockets[i].Handler = NULL;
        sockets[i].MSS = MSS_Default;
        return (struct Socket *)&sockets[i];
      }
    }
  }
  return -1;
}
void Socket_Free(struct Socket *socket) {
  for (int i = 0; i != MAX_SOCKET_NUM; i++) {
    if (&sockets[i] == socket) {
      sockets[i].state = SOCKET_FREE;
      return;
    }
  }
}
void Socket_Init(struct Socket *socket, uint32_t remoteIP, uint16_t remotePort,
                 uint32_t localIP, uint16_t localPort) {
  socket->remoteIP = remoteIP;
  socket->remotePort = remotePort;
  socket->localIP = localIP;
  socket->localPort = localPort;
}
void Socket_Bind(struct Socket *socket,
                 void (*Handler)(struct Socket *socket, void *base)) {
  socket->Handler = Handler;
}
struct Socket *Socket_Find(uint32_t dstIP, uint16_t dstPort, uint32_t srcIP,
                           uint16_t srcPort, uint8_t protocol) {
  for (int i = 0; i != MAX_SOCKET_NUM; i++) {
    if (srcIP == sockets[i].localIP && dstIP == sockets[i].remoteIP &&
        srcPort == sockets[i].localPort && dstPort == sockets[i].remotePort &&
        protocol == sockets[i].protocol && sockets[i].state != SOCKET_FREE) {
      return (struct Socket *)&sockets[i];
    } else if (srcIP == sockets[i].localIP && srcPort == sockets[i].localPort &&
               protocol == sockets[i].protocol &&
               sockets[i].state == SOCKET_TCP_LISTEN) {
      return (struct Socket *)&sockets[i];
    }
  }
  return -1;
}

struct SocketServer *
SocketServer_Alloc(void (*Handler)(struct Socket *socket, void *base),
                   uint32_t srcIP, uint16_t srcPort, uint8_t protocol) {
  struct SocketServer *server =
      (struct SocketServer *)malloc(sizeof(struct SocketServer));
  for (int i = 0; i < SOCKET_SERVER_MAX_CONNECT; i++) {
    if (protocol == TCP_PROTOCOL) {
      server->socket[i] = Socket_Alloc(TCP_PROTOCOL);
      Socket_Init(server->socket[i], 0, 0, srcIP, srcPort);
      Socket_Bind(server->socket[i], Handler);
      server->socket[i]->state = SOCKET_TCP_LISTEN;
    } else if (protocol == UDP_PROTOCOL) {
      server->socket[i] = Socket_Alloc(UDP_PROTOCOL);
      Socket_Init(server->socket[i], 0, 0, srcIP, srcPort);
      Socket_Bind(server->socket[i], Handler);
    }
  }
  server->Send = SocketServer_Send;
  return server;
}

void SocketServer_Free(struct SocketServer *server, uint8_t protocol) {
  for (int i = 0; i < SOCKET_SERVER_MAX_CONNECT; i++) {
    if (server->socket[i] != NULL &&
        server->socket[i]->state == SOCKET_TCP_ESTABLISHED) {
      if (protocol == TCP_PROTOCOL) {
        server->socket[i]->Disconnect(server->socket[i]);
      }
      Socket_Free(server->socket[i]);
    }
  }
  free(server);
}

uint32_t SocketServer_Status(struct SocketServer *server, uint8_t state) {
  uint32_t count = 0;
  for (int i = 0; i < SOCKET_SERVER_MAX_CONNECT; i++) {
    if (server->socket[i]->state == state) {
      count++;
    }
  }
  return count;
}

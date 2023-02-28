#include <dos.h>
#define MAX_FTP_CLIENT_NUM 32
static struct FTP_Client ftp_c[MAX_FTP_CLIENT_NUM];
static void FTP_Client_CMD_Handler(struct Socket *socket, void *base) {
  struct IPV4Message *ipv4 =
      (struct IPV4Message *)(base + sizeof(struct EthernetFrame_head));
  struct TCPMessage *tcp =
      (struct TCPMessage *)(base + sizeof(struct EthernetFrame_head) +
                            sizeof(struct IPV4Message));
  uint8_t *data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + (tcp->headerLength * 4);
  printf("Recv:%s\n",data);
  struct FTP_Client *ftp_c_ = FTP_Client_Find(socket);
  if (ftp_c_ != NULL) {
    printf("Not NULL\n");
    uint32_t length = swap16(ipv4->totalLength) - sizeof(struct IPV4Message) -
                      (tcp->headerLength * 4);
    free((void *)ftp_c_->recv_buf_cmd);
    ftp_c_->recv_buf_cmd = (uint8_t *)malloc(length);
    memcpy((void *)ftp_c_->recv_buf_cmd, (void *)data, length);
    ftp_c_->reply_code = (uint32_t)strtol(data, NULL, 10);
    ftp_c_->recv_flag_cmd = true;
    printf("reply_code:%d\n",ftp_c_->reply_code );
  } else {
    printf("NULL\n");
  }
  //printf("Reply: %s\n", ftp_c_->recv_buf_cmd);
}
static void FTP_Client_DAT_Handler(struct Socket *socket, void *base) {
  struct IPV4Message *ipv4 =
      (struct IPV4Message *)(base + sizeof(struct EthernetFrame_head));
  struct TCPMessage *tcp =
      (struct TCPMessage *)(base + sizeof(struct EthernetFrame_head) +
                            sizeof(struct IPV4Message));
  uint8_t *data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + (tcp->headerLength * 4);
  struct FTP_Client *ftp_c_ = FTP_Client_Find(socket);
  printf("DAT Handler!\n");
  if (ftp_c_ != NULL) {
    uint32_t pak_length = swap16(ipv4->totalLength) -
                          sizeof(struct IPV4Message) - (tcp->headerLength * 4);
    uint32_t mlc_length = *(uint32_t *)(ftp_c_->recv_buf_dat - 4);
    if (ftp_c_->recv_buf_dat != NULL) {
      void *ptr = realloc(ftp_c_->recv_buf_dat, pak_length + mlc_length);
      ftp_c_->recv_buf_dat = (uint8_t *)ptr;
      memcpy((void *)(ftp_c_->recv_buf_dat + mlc_length), (void *)data,
             pak_length);
      ftp_c_->recv_dat_size = pak_length + mlc_length;
    } else {
      ftp_c_->recv_buf_dat = (uint8_t *)malloc(pak_length);
      memcpy((void *)ftp_c_->recv_buf_dat, (void *)data, pak_length);
      ftp_c_->recv_dat_size = pak_length;
    }
    ftp_c_->recv_flag_dat = true;
  }
}
static int Login(struct FTP_Client *ftp_c_, uint8_t *user, uint8_t *pass) {
  char s[128];
  sprintf(s, "USER %s\r\n", user);
  ftp_c_->recv_flag_cmd = false;
  ftp_c_->socket_cmd->Send(ftp_c_->socket_cmd, s, strlen(s));
  printf("wait");
  while (ftp_c_->recv_flag_cmd != true)
    ;
  printf("ok");
  if (ftp_c_->reply_code != 331) {
    printf("Return n 331\n");
    return -1;
  }
  sprintf(s, "PASS %s\r\n", pass);
  ftp_c_->recv_flag_cmd = false;
  ftp_c_->socket_cmd->Send(ftp_c_->socket_cmd, s, strlen(s));
  printf("Wait 2\n");
  while (ftp_c_->recv_flag_cmd != true)
    ;
  printf("OK\n");
  if (ftp_c_->reply_code != 230) {
    printf("Return n 230\n");
    return -1;
  }
  return 0;
}
static int TransModeChoose(struct FTP_Client *ftp_c_, int mode) {
  if (mode == FTP_PORT_MODE) {
    ftp_c_->socket_dat->remotePort = 20;
    ftp_c_->socket_dat->state = SOCKET_TCP_LISTEN;
    char s[128];
    uint8_t i1, i2, i3, i4;
    i1 = (uint8_t)ftp_c_->socket_dat->localIP;
    i2 = (uint8_t)ftp_c_->socket_dat->localIP >> 8;
    i3 = (uint8_t)ftp_c_->socket_dat->localIP >> 16;
    i4 = (uint8_t)ftp_c_->socket_dat->localIP >> 24;
    sprintf(s, "PORT %d,%d,%d,%d,%d,%d\r\n", i1, i2, i3, i4,
            ftp_c_->socket_dat->localPort / 256,
            ftp_c_->socket_dat->localPort % 256);
    ftp_c_->recv_flag_cmd = false;
    ftp_c_->socket_cmd->Send(ftp_c_->socket_cmd, s, strlen(s));
    while (ftp_c_->recv_flag_cmd != true)
      ;
    while (ftp_c_->socket_dat->state != SOCKET_TCP_ESTABLISHED)
      ;
  } else if (mode == FTP_PASV_MODE) {
    ftp_c_->recv_flag_cmd = false;
    ftp_c_->socket_cmd->Send(ftp_c_->socket_cmd, "PASV\r\n", 6);
    while (ftp_c_->recv_flag_cmd != true)
      ;
    if (ftp_c_->reply_code != 227) {
      return -1;
    }
    uint8_t *p = strrchr(ftp_c_->recv_buf_cmd, '(');
    uint16_t port;
    for (int i = 0, spCount = 0; i != strlen(p); i++) {
      if (p[i] == ',') {
        spCount++;
      }
      if (spCount == 4) {
        char *ptr;
        uint8_t port1 = (uint8_t)strtol(&p[i + 1],&ptr, 10);
        uint8_t port2 = (uint8_t)strtol(ptr, NULL, 10);
        port = port1 * 256 + port2;
        printf("port1=%d\nport2=%d\n",port1,port2 );
        break;
      }
    }
    ftp_c_->socket_dat->remotePort = port;
    printf("Port = %d\n",port);
    if (ftp_c_->socket_dat->Connect(ftp_c_->socket_dat) == -1) {
      return -1;
    }
  }
  return 0;
}
static void Logout(struct FTP_Client *ftp_c_) {
  ftp_c_->socket_cmd->Send(ftp_c_->socket_cmd, "QUIT\r\n", 6);
  ftp_c_->is_login = false;
}
static int Download(struct FTP_Client *ftp_c_, uint8_t *path_pdos,
                    uint8_t *path_ftp, int mode) {
  printf("DOWNLOAD!!!\n");
  char s[50];
  sprintf(s, "SIZE %s\r\n", path_ftp);
  ftp_c_->recv_flag_cmd = false;
  ftp_c_->socket_cmd->Send(ftp_c_->socket_cmd, s, strlen(s));
  while (ftp_c_->recv_flag_cmd != true)
    ;
  uint32_t size = (uint32_t)strtol(ftp_c_->recv_buf_cmd + 4, NULL, 10);
  printf("SIZE=%d\n",size);
  if (ftp_c_->TransModeChoose(ftp_c_, mode) == -1) {
    printf("Error!!!\n");
    return -1;
  }
  if (fsz(path_pdos) == -1) {
    mkfile(path_pdos);
  }
  FILE *fp = fopen(path_pdos, "wb");
  sprintf(s, "RETR %s\r\n", path_ftp);
  ftp_c_->recv_flag_cmd = false;
  ftp_c_->socket_cmd->Send(ftp_c_->socket_cmd, s, strlen(s));
  printf("Wait recv flag.........\n");
  while (ftp_c_->recv_flag_cmd != true)
    ;
  printf("Wait file send.........\n");
  while (ftp_c_->recv_dat_size < size)
    ;
  printf("RECV:\n");
  for (int i = 0; i != size; i++) {
    printf("%c",ftp_c_->recv_buf_dat[i]);
    fputc(ftp_c_->recv_buf_dat[i], fp);
  }
  free((void *)ftp_c_->recv_buf_dat);
  return 0;
}
static uint8_t *Getlist(struct FTP_Client *ftp_c_, uint8_t *path, int mode) {
  if (ftp_c_->TransModeChoose(ftp_c_, mode) == -1) {
    return -1;
  }
  char s[50];
  sprintf(s, "LIST %s\r\n", path);
  ftp_c_->recv_flag_cmd = false;
  ftp_c_->recv_flag_dat = false;
  ftp_c_->socket_cmd->Send(ftp_c_->socket_cmd, s, strlen(s));
  while (ftp_c_->recv_flag_cmd != true)
    ;
  if (ftp_c_->reply_code != 150) {
    return (uint8_t *)NULL;
  }
  while (ftp_c_->recv_flag_dat != true)
    ;
  uint8_t *res = (uint8_t *)malloc(*(uint32_t *)(ftp_c_->recv_buf_dat - 4));
  memcpy((void *)res, (void *)ftp_c_->recv_buf_dat,
         *(uint32_t *)(ftp_c_->recv_buf_dat - 4));
  return res;
}
struct FTP_Client *FTP_Client_Alloc(uint32_t remoteIP, uint32_t localIP,
                                    uint16_t localPort) {
  for (int i = 0; i != MAX_FTP_CLIENT_NUM; i++) {
    if (ftp_c[i].using1 == false) {
      ftp_c[i].using1 = true;
      ftp_c[i].is_login = false;
      ftp_c[i].socket_cmd = Socket_Alloc(TCP_PROTOCOL);
      ftp_c[i].socket_dat = Socket_Alloc(TCP_PROTOCOL);
      ftp_c[i].socket_cmd->remoteIP = remoteIP;
      ftp_c[i].socket_dat->remoteIP = remoteIP;
      ftp_c[i].socket_cmd->remotePort = FTP_SERVER_COMMAND_PORT;
      ftp_c[i].socket_cmd->localIP = localIP;
      ftp_c[i].socket_dat->localIP = localIP;
      ftp_c[i].socket_cmd->localPort = localPort;
      ftp_c[i].socket_cmd->localPort = localPort + 1;
      ftp_c[i].Login = Login;
      ftp_c[i].TransModeChoose = TransModeChoose;
      ftp_c[i].Logout = Logout;
      ftp_c[i].Download = Download;
      ftp_c[i].Getlist = Getlist;
      ftp_c[i].recv_buf_cmd = (uint8_t *)malloc(128); // 吴用
      ftp_c[i].recv_buf_dat = (uint8_t *)NULL;
      ftp_c[i].recv_flag_cmd = false;
      ftp_c[i].recv_flag_dat = false;
      Socket_Bind(ftp_c[i].socket_cmd, FTP_Client_CMD_Handler);
      Socket_Bind(ftp_c[i].socket_dat, FTP_Client_DAT_Handler);
      if (ftp_c[i].socket_cmd->Connect(ftp_c[i].socket_cmd) == -1) {
        return (struct FTP_Client *)NULL;
      }
      while(ftp_c[i].recv_flag_cmd == false);
      if(ftp_c[i].reply_code != 220) {
        printf("Connet error\n");
        return NULL;
      }
      ftp_c[i].recv_flag_cmd = false;
      return &ftp_c[i];
    }
  }
  return (struct FTP_Client *)NULL;
}
struct FTP_Client *FTP_Client_Find(struct Socket *s) {
  for (int i = 0; i != MAX_FTP_CLIENT_NUM; i++) {
    if (ftp_c[i].using1 == false) {
      continue;
    } else {
      if (ftp_c[i].socket_cmd == s || ftp_c[i].socket_dat == s) {
        return &ftp_c[i];
      }
    }
  }
}
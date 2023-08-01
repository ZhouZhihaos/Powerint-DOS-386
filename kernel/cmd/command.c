// 命令行与命令处理
#include <cmd.h>
#include <dos.h>
#include <fcntl.h>
#include <mst.h>
void showPage(void);
void wav_player_test(void);
/* 一些函数或结构体声明 */
typedef struct {
  char path[50];
  char* buf;
} HttpFile;
List* httpFileList;
extern struct ide_device {
  unsigned char Reserved;       // 0 (Empty) or 1 (This Drive really exists).
  unsigned char Channel;        // 0 (Primary Channel) or 1 (Secondary Channel).
  unsigned char Drive;          // 0 (Master Drive) or 1 (Slave Drive).
  unsigned short Type;          // 0: ATA, 1:ATAPI.
  unsigned short Signature;     // Drive Signature
  unsigned short Capabilities;  // Features.
  unsigned int CommandSets;     // Command Sets Supported.
  unsigned int Size;            // Size in Sectors.
  unsigned char Model[41];      // Model in string.
} ide_devices[4];
unsigned char* ramdisk;
typedef struct BaseAddressRegister {
  int prefetchable;
  uint8_t* address;
  uint32_t size;
  int type;
} BaseAddressRegister;
BaseAddressRegister GetBaseAddressRegister(uint8_t bus,
                                           uint8_t device,
                                           uint8_t function,
                                           uint8_t bar);
/* vdisk的RW测试函数 */
void TestRead(char drive,
              unsigned char* buffer,
              unsigned int number,
              unsigned int lba) {
  // printk("TestRW:Read Lba %d,Read Sectors number %d\n", lba, number);
  memcpy(buffer, ramdisk + lba * 512, number * 512);
}
void TestWrite(char drive,
               unsigned char* buffer,
               unsigned int number,
               unsigned int lba) {
  // printk("TestRW:Write Lba %d,Write Sectors number %d\n", lba, number);
  memcpy(ramdisk + lba * 512, buffer, number * 512);
}
// socket测试例子
static void TCP_Socket_Handler(struct Socket* socket, void* base) {
  struct TCPMessage* tcp =
      (struct TCPMessage*)(base + sizeof(struct EthernetFrame_head) +
                           sizeof(struct IPV4Message));
  uint8_t* data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + (tcp->headerLength * 4);
  printf("\nTCP Recv from %d.%d.%d.%d:%d:%s\n",
         (uint8_t)(socket->remoteIP >> 24), (uint8_t)(socket->remoteIP >> 16),
         (uint8_t)(socket->remoteIP >> 8), (uint8_t)(socket->remoteIP),
         socket->remotePort, data);
}
static void UDP_Socket_Handler(struct Socket* socket, void* base) {
  uint8_t* data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
  printf("\nUDP Recv from %d.%d.%d.%d:%d:%s\n",
         (uint8_t)(socket->remoteIP >> 24), (uint8_t)(socket->remoteIP >> 16),
         (uint8_t)(socket->remoteIP >> 8), (uint8_t)(socket->remoteIP),
         socket->remotePort, data);
}
/* 如果开启了HTTP命令，那么接收到HTTP请求会调用这个函数 */
static unsigned char* html_file;
static void HTTP_Socket_Handler(struct Socket* socket, void* base) {
  /* 声明，获取各个协议的标头和数据 */
  struct IPV4Message* ipv4 =
      (struct IPV4Message*)(base + sizeof(struct EthernetFrame_head));
  struct TCPMessage* tcp =
      (struct TCPMessage*)(base + sizeof(struct EthernetFrame_head) +
                           sizeof(struct IPV4Message));
  uint16_t size = swap16(ipv4->totalLength) - sizeof(struct IPV4Message) -
                  (tcp->headerLength * 4);
  uint8_t* data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + (tcp->headerLength * 4);
  if (http_check(data, size).ok) {  // 是HTTP GetHeader
    /* 标头信息 */
    unsigned char head[500] = "HTTP/1.1 200 OK\r\n";
    unsigned char content_type[] = "Content-Type: text/html\r\n";
    unsigned char content_length[100];
    unsigned char date[100];
    printf("Is Http get header!\n");
    if (strlen(http_check(data, size).path) ==
        1) {             // 只有一个字符，那只能是"/"
      printf("root\n");  // 根目录
      HttpFile* f = (HttpFile*)FindForCount(1, httpFileList)
                        ->val;  // 第一个文件就是根目录
      printf("f->path = %s\n", f->path);
      html_file = (unsigned char*)f->buf;  // 设置html_file的地址
    } else {
      printf("Not root.\n");  // 不是根目录
      for (int i = 1; FindForCount(i, httpFileList) != NULL; i++) {
        HttpFile* f = (HttpFile*)FindForCount(i, httpFileList)->val;
        if (strcmp(f->path, http_check(data, size).path + 1) ==
            0) {  // 判断网站是否有这个文件
          html_file = (unsigned char*)f->buf;  // 有，直接返回
          goto OK;
        }
      }
      html_file =
          (unsigned char
               *)("<html><head><title>404 Not Found</title></head><body "
                  "bgcolor= white"
                  "><center><h1>404 Not "
                  "Found</h1></center><hr><center>Powerint DOS "
                  "HTTP Server</center></body></html>"); // 啊，没有呢，那就只能给404页面了
      strcpy((char*)head, "HTTP/1.1 404 Not Found\r\n");  // 顺便修改一下head
    }
  OK:

    sprintf((char*)content_length, "Content-Length: %d\r\n",
            strlen((char*)html_file));
    strcat((char*)head, (char*)content_type);
    strcat((char*)head, (char*)content_length);
    GetNowDate((char*)date);
    strcat((char*)head, (char*)date);
    strcat((char*)head, "\r\n");
    strcat((char*)head, "\r\n");
    printf("%s", (char*)head);
    // unsigned char *head = "HTTP/1.1 200 OK\r\n";
    unsigned char* packet = (unsigned char*)page_malloc(
        strlen((char*)head) + strlen((char*)html_file) +
        1);  // 声明最终的packet发送的数据
    memcpy((void*)packet, (void*)head, strlen((char*)head));  // HTTP 标头
    memcpy((void*)(packet + strlen((char*)head)), (void*)html_file,
           strlen((char*)html_file));  // html文件数据
    packet[strlen((char*)head) + strlen((char*)html_file) + 1] =
        0;  // 字符串结束符（为了下面调用的strlen函数）
    socket->Send(socket, packet,
                 strlen((char*)packet) + 1);  // 调用Socket API发送
  } else {
    printf("isn't http get header\n");  // 不是HTTP get header
  }
}
static void SocketServerLoop(
    struct SocketServer* server) {  // Socket Server（Http）的循环
  /* 检测哪个socket已经与客户端断开连接了，重新设置状态，不然无法连接其他客户端（一次性socket）
   */
  static bool flags[SOCKET_SERVER_MAX_CONNECT];
  memset((void*)flags, false, SOCKET_SERVER_MAX_CONNECT * sizeof(bool));
  while (1) {
    for (int i = 0; i < SOCKET_SERVER_MAX_CONNECT; i++) {
      if (server->socket[i]->state == SOCKET_TCP_CLOSED) {
        printf("server->socket[%d] close.\n", i);
        flags[i] = false;
        server->socket[i]->state = SOCKET_TCP_LISTEN;
      } else if (server->socket[i]->state == SOCKET_TCP_ESTABLISHED &&
                 flags[i] == false) {
        printf("server->socket[%d] connect.\n", i);
        flags[i] = true;
      }
    }
  }
}
/* 用UDP协议传输文件 */
static unsigned int fudp_size;
static unsigned char* fudp_buffer;
static void FUDP_Socket_Handler(struct Socket* socket, void* base) {
  /* 获取数据并拷贝到fudp_buffer中 */
  struct IPV4Message* ipv4 =
      (struct IPV4Message*)(base + sizeof(struct EthernetFrame_head));
  (void)(ipv4);
  struct UDPMessage* udp =
      (struct UDPMessage*)(base + sizeof(struct EthernetFrame_head) +
                           sizeof(struct IPV4Message));
  uint8_t* data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
  fudp_size = swap16(udp->length) - sizeof(struct UDPMessage);
  fudp_buffer = malloc(fudp_size);
  memcpy((void*)fudp_buffer, (void*)data, fudp_size);
}
/* 取绝对值 */
int abs(int n) {
  if (n >= 0) {
    return n;
  } else {
    return -n;
  }
}

void command_run(char* cmdline) {
  //命令解析器
  uint32_t addr;
  uint8_t c;
  char* p;
  if (cmdline[0] == 0) {
    return;
  }
  for (int i = 0; i < strlen(cmdline); i++) {
    if (cmdline[i] != ' ' && cmdline[i] != '\n' && cmdline[i] != '\r') {
      strcpy(cmdline, cmdline + i);
      goto CHECK_OK;
    }
  }
  //都是空格，直接返回
  return;
CHECK_OK:
  if (current_task()->line != cmdline) {
    strcpy(current_task()->line, cmdline);
  }
  if (strincmp("FORMAT ", cmdline, 7) == 0) {
    int res = vfs_format(cmdline[7], "PFS");
    if (res == 0) {
      printf("fatal error\n\n");
    } else if (res == 1) {
      printf("Format OK\n");
    }
  } else if (stricmp("FAT", cmdline) == 0) {
    struct TASK* task = current_task();
    int neline =
        current_task()->TTY->xsize / (get_dm(current_task()->nfs).type / 4 + 1);
    for (int i = 0, j = 0; i != get_dm(current_task()->nfs).FatMaxTerms; i++) {
      if (get_dm(current_task()->nfs).type == 12) {
        printf("%03x ", get_dm(task->nfs).fat[i]);
      } else if (get_dm(current_task()->nfs).type == 16) {
        printf("%04x ", get_dm(task->nfs).fat[i]);
      } else if (get_dm(current_task()->nfs).type == 32) {
        printf("%08x ", get_dm(task->nfs).fat[i]);
      }
      if (!((i + 1) % neline)) {
        printf("\b\n");
        j++;
      }
      if (j == current_task()->TTY->ysize - 1) {
        printf("Press any key to continue...");
        getch();
        printf("\n");
        j = 0;
      }
    }
    printf("\n");
  } else if (stricmp("SHOWPAGE", cmdline) == 0) {
    showPage();
  } else if (stricmp("CHAT", cmdline) == 0) {
    chat_cmd();
  } else if (stricmp("NETGOBANG", cmdline) == 0) {
    netgobang();
  } else if (strincmp("READPCI ", cmdline, 8) == 0) {
    char* endptr;
    uint32_t bus = strtol(cmdline + 8, &endptr, 16);
    printk("endptr = %s\n", endptr);
    uint32_t slot = strtol(endptr, &endptr, 16);

    uint32_t func = strtol(endptr, &endptr, 16);

    printf("looking for %02x %02x %02x\n", bus, slot, func);
    for (int i = 0; i < 6; i++) {
      printf("BAR%d:%08x ", i,
             GetBaseAddressRegister(bus, slot, func, i).address);
    }
    printf("\n");
  } else if (strincmp("CDISK ", cmdline, 6) == 0) {
    if (Get_Argc(cmdline) != 2) {
      printf("arg error.\n");
      return;
    }
    char name[50];
    char size[50];
    Get_Arg(name, cmdline, 1);
    Get_Arg(size, cmdline, 2);
    int isize = strtol(size, NULL, 10);
    vfs_createfile(name);
    char* b = malloc(isize);
    EDIT_FILE(name, b, isize, 0);
    free(b);
    // printf("Name=%s Size=%s\n",name,size);
  } else if (strincmp("HTTP ", cmdline, 5) == 0) {
    httpFileList = NewList();  // 创建文件链表
    for (int i = 1; i < Get_Argc(cmdline) + 1; i++) {
      char s[50];
      Get_Arg(s, cmdline, i);
      printf("#%d %s\n", i, s);
      FILE* fp = fopen(s, "rb");
      printf("fp=%08x\n", fp);
      HttpFile* f = page_malloc(sizeof(HttpFile));
      f->buf = (char*)fp->buffer;
      strcpy(f->path, s);
      AddVal((int)f, httpFileList);
      // f = FindForCount(i, httpFileList)->val;
      // printf("f->path = %s\n", f->path);
    }
    extern uint32_t ip;
    srand(time());
    uint16_t port = (uint16_t)80;
    struct SocketServer* server =
        SocketServer_Alloc(HTTP_Socket_Handler, ip, port, TCP_PROTOCOL);
    printf("SrcIP/Port:%d.%d.%d.%d:%d\n", (uint8_t)(ip >> 24),
           (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip), port);
    SocketServerLoop(server);
  } else if (stricmp("SOCKET", cmdline) == 0) {
    extern uint32_t ip;
    struct Socket* socket;
    struct SocketServer* server;
    srand(time());
    uint32_t dstIP = 0, srcIP = ip;
    uint16_t dstPort = 0, srcPort = (uint16_t)rand();
    printf("Protocol(UDP(0)/TCP(1)):");
    int p = getch() - '0';
    printf("\n");
    int m = 0;
    if (p) {
      printf("Mode(CLIENT(0)/SERVER(1)):");
      m = getch() - '0';
      printf("\n");
    }
    char buf[15];
    if (!m) {
      printf("DstIP:");
      input(buf, 15);
      dstIP = IP2UINT32_T((uint8_t*)buf);
      printf("DstPort:");
      input(buf, 15);
      dstPort = (uint16_t)strtol(buf, NULL, 10);
    }
    printf("Src IP:%d.%d.%d.%d\n", (uint8_t)(srcIP >> 24),
           (uint8_t)(srcIP >> 16), (uint8_t)(srcIP >> 8), (uint8_t)(srcIP));
    printf("Src Port:%d\n", srcPort);
    if (!m) {
      if (p) {  // TCP
        socket = socket_alloc(TCP_PROTOCOL);
        Socket_Bind(socket, TCP_Socket_Handler);
      } else if (!p) {  // UDP
        socket = socket_alloc(UDP_PROTOCOL);
        Socket_Bind(socket, UDP_Socket_Handler);
      }
      Socket_Init(socket, dstIP, dstPort, srcIP, srcPort);
    } else if (m) {
      if (p) {
        server = SocketServer_Alloc(TCP_Socket_Handler, srcIP, srcPort,
                                    TCP_PROTOCOL);
      } else if (!p) {
        server = SocketServer_Alloc(UDP_Socket_Handler, srcIP, srcPort,
                                    UDP_PROTOCOL);
      }
    }
    if (p && m) {
      while (SocketServer_Status(server, SOCKET_TCP_ESTABLISHED) == 0)
        ;
    } else if (p && !m) {
      if (socket->Connect(socket) == -1) {
        printf("Connect failed.\n");
        return;
      }
    }
    if (!m) {
      printf(
          "Connect %d.%d.%d.%d:%d done.\n", (uint8_t)(socket->remoteIP >> 24),
          (uint8_t)(socket->remoteIP >> 16), (uint8_t)(socket->remoteIP >> 8),
          (uint8_t)(socket->remoteIP), socket->remotePort);
    }
    char* inp = (char*)page_malloc(1024);
    while (1) {
      if (socket->state == SOCKET_TCP_CLOSED && !m) {
        if (p) {
          socket->Disconnect(socket);
        }
        socket_free(socket);
        page_free((void*)inp, 1024);
        return;
      }
      if (p) {
        printf("TCP ");
      } else if (!p) {
        printf("UDP ");
      }
      if (!m) {
        printf("Send to %d.%d.%d.%d:%d:", (uint8_t)(socket->remoteIP >> 24),
               (uint8_t)(socket->remoteIP >> 16),
               (uint8_t)(socket->remoteIP >> 8), (uint8_t)(socket->remoteIP),
               socket->remotePort);
      } else if (m) {
        printf("Send to %d client(s):",
               SocketServer_Status(server, SOCKET_TCP_ESTABLISHED));
      }
      input(inp, 1024);
      if (stricmp(inp, "exit") == 0) {
        if (!m) {
          if (p) {
            socket->Disconnect(socket);
          }
          socket_free(socket);
        } else if (m) {
          SocketServer_Free(server, TCP_PROTOCOL);
        }
        page_free((void*)inp, 1024);
        return;
      }
      if (!m) {
        socket->Send(socket, (uint8_t*)inp, strlen(inp));
      } else if (m) {
        server->Send(server, (uint8_t*)inp, strlen(inp));
      }
    }
  } else if (stricmp("ARP", cmdline) == 0) {
    extern uint32_t ARP_write_pointer;
    extern uint64_t ARP_mac_address[MAX_ARP_TABLE];
    extern uint32_t ARP_ip_address[MAX_ARP_TABLE];
    if (ARP_write_pointer == 0) {
      return;
    }
    for (int i = 0; i != ARP_write_pointer; i++) {
      printf("IP: %d.%d.%d.%d -> MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
             (uint8_t)(ARP_ip_address[i] >> 24),
             (uint8_t)(ARP_ip_address[i] >> 16),
             (uint8_t)(ARP_ip_address[i] >> 8), (uint8_t)(ARP_ip_address[i]),
             (uint8_t)(ARP_mac_address[i]), (uint8_t)(ARP_mac_address[i] >> 8),
             (uint8_t)(ARP_mac_address[i] >> 16),
             (uint8_t)(ARP_mac_address[i] >> 24),
             (uint8_t)(ARP_mac_address[i] >> 32),
             (uint8_t)(ARP_mac_address[i] >> 40));
    }
  } else if (strincmp("FUDP ", cmdline, 5) == 0) {
    extern uint32_t ip;
    uint32_t dstIP = 0;
    uint16_t dstPort = 0;
    srand(time());
    uint32_t srcIP = ip;
    uint16_t srcPort = (uint16_t)rand();
    char buf[15];
    printf("Src IP:%d.%d.%d.%d\n", (uint8_t)(srcIP >> 24),
           (uint8_t)(srcIP >> 16), (uint8_t)(srcIP >> 8), (uint8_t)(srcIP));
    printf("Src Port:%d\n", srcPort);
    printf("Choose Mode(Send(0)/Recv(1)):");
    input(buf, 15);
    int m = (int)strtol(buf, NULL, 10);
    printf("DstIP:");
    input(buf, 15);
    dstIP = IP2UINT32_T((uint8_t*)buf);
    printf("DstPort:");
    input(buf, 15);
    dstPort = (uint16_t)strtol(buf, NULL, 10);
    struct Socket* socket;
    socket = socket_alloc(UDP_PROTOCOL);
    Socket_Init(socket, dstIP, dstPort, srcIP, srcPort);
    if (m) {
      Socket_Bind(socket, FUDP_Socket_Handler);
      printf("Waiting %d.%d.%d.%d:%d to send file...",
             (uint8_t)(socket->remoteIP >> 24),
             (uint8_t)(socket->remoteIP >> 16),
             (uint8_t)(socket->remoteIP >> 8), (uint8_t)(socket->remoteIP),
             socket->remotePort);
      fudp_buffer = NULL;
      while (fudp_buffer == NULL)
        ;
      printf("OK.\n");
      char* save_file_name = (char*)malloc(100);
      Get_Arg(save_file_name, cmdline, 1);
      if (fsz(save_file_name) == -1) {
        vfs_createfile(save_file_name);
      }
      FILE* fp = fopen(save_file_name, "wb");
      fseek(fp, 0, 0);
      for (int i = 0; i != fudp_size; i++) {
        fputc(fudp_buffer[i], fp);
      }
      fclose(fp);
      free(save_file_name);
      free(fudp_buffer);
    } else if (!m) {
      char* send_file_name = (char*)malloc(100);
      Get_Arg(send_file_name, cmdline, 1);
      if (fsz(send_file_name) == -1) {
        printf("File not find!\n\n");
        return;
      }
      FILE* fp = fopen(send_file_name, "r");
      socket->Send(socket, fp->buffer, fp->fileSize);
      fclose(fp);
      free(send_file_name);
    }
    socket_free(socket);
  } else if (strincmp("NSLOOKUP ", cmdline, 9) == 0) {
    uint8_t* dns = (uint8_t*)page_malloc(strlen(cmdline + 9) + 1);
    memcpy(dns + 1, cmdline + 9, strlen(cmdline + 9));
    uint32_t ip = dns_parse_ip(dns + 1);
    printf("DNS: %s -> IP: %d.%d.%d.%d\n", cmdline + 9, (uint8_t)(ip >> 24),
           (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip));
    page_free(dns, strlen(cmdline + 9) + 1);
  } else if (strincmp("PING ", cmdline, 5) == 0) {
    cmd_ping(cmdline + 5);
  } else if (stricmp("IPCONFIG", cmdline) == 0) {
    extern uint32_t gateway, submask, dns, ip, dhcp_ip;
    printf("DCHP: %d.%d.%d.%d\n", (uint8_t)(dhcp_ip >> 24),
           (uint8_t)(dhcp_ip >> 16), (uint8_t)(dhcp_ip >> 8),
           (uint8_t)(dhcp_ip));
    printf("IP: %d.%d.%d.%d\n", (uint8_t)(ip >> 24), (uint8_t)(ip >> 16),
           (uint8_t)(ip >> 8), (uint8_t)(ip));
    printf("DNS: %d.%d.%d.%d\n", (uint8_t)(dns >> 24), (uint8_t)(dns >> 16),
           (uint8_t)(dns >> 8), (uint8_t)(dns));
    printf("Subnet Mask: %d.%d.%d.%d\n", (uint8_t)(submask >> 24),
           (uint8_t)(submask >> 16), (uint8_t)(submask >> 8),
           (uint8_t)(submask));
    printf("Gateway: %d.%d.%d.%d\n", (uint8_t)(gateway >> 24),
           (uint8_t)(gateway >> 16), (uint8_t)(gateway >> 8),
           (uint8_t)(gateway));
  } else if (stricmp("CATCH", cmdline) == 0) {
    int a = 0;
    disableExp();                // 关闭蓝屏
    ClearExpFlag();              // 清除err标志
    SetCatchEip(get_eip());      // 重定位Catch后返回的EIP
    if (GetExpFlag()) {          // 是否产生了异常？
      printf("error catch!\n");  // 产生了
      ClearExpFlag();            // 清除标志
      EnableExp();               // 测试完成，开启蓝屏
    } else {
      printf("Try to calc 5 / 0\n");  // 没有异常，尝试计算
      printf("%d\n", 5 / a);          // 输出结果
    }
  } else if (stricmp("FORK", cmdline) == 0) {
    // printf("FTP Test!\n");

    // extern uint32_t ip;
    // struct FTP_Client *ftp_c =
    //     FTP_Client_Alloc(IP2UINT32_T((uint8_t *)"192.168.0.106"), ip, 25565);
    // printf("Alloc OK!\n");
    // if (ftp_c->Login(ftp_c, (uint8_t *)"anonymous", (uint8_t *)"anonymous")
    // ==
    //     -1) {
    //   printf("Login Faild\n");
    //   return;
    // }
    // printf("Login OK!!!\n");

    // ftp_c->Download(ftp_c, (uint8_t *)"c.py", (uint8_t *)"/c.py",
    //                 FTP_PASV_MODE);
    // ftp_c->Logout(ftp_c);
    // SubTask(GetTask(2));

    // io_cli();
    // struct TASK* task = AddUserTask("UsrTask", 1, 1146 * 8, usertasktest,
    // 1145*8,1145*8,(unsigned int)malloc(16*1024)+16*1024); task->tss.ss0 =
    // 1*8; task->tss.esp0 = malloc(16*1024)+16*1024; io_sti();
  } else if(stricmp("SHOW_HEAP",cmdline) == 0) {
    show_heap(current_task()->mm);
  } else if (strincmp("MOUNT ", cmdline, 6) == 0) {
    int c = mount(cmdline + 6);
    printf("mount file in %c:\\\n", c);
  } else if (strincmp("UNMOUNT ", cmdline, 8) == 0) {
    unmount(cmdline[8]);
  } else if (stricmp("SB16", cmdline) == 0) {
    wav_player_test();
  } else if (stricmp("DISKLS", cmdline) == 0) {
    extern vdisk vdisk_ctl[26];

    for (int i = 0; i < 26; i++) {
      if (vdisk_ctl[i].flag) {
        printf("%c:\\ => TYPE: %s\n", i + ('A'), vdisk_ctl[i].DriveName);
      }
    }
  } else if (stricmp("ADD", cmdline) == 0) {
    if (running_mode == POWERINTDOS) {
      AddShell_TextMode();
    } else if (running_mode == HIGHTEXTMODE) {
      AddShell_HighTextMode();
    }
  } else if (strincmp("SWITCH ", cmdline, 7) == 0) {
    if (running_mode == POWERINTDOS) {
      SwitchShell_TextMode(strtol(cmdline + 7, NULL, 10));
    } else if (running_mode == HIGHTEXTMODE) {
      SwitchShell_HighTextMode(strtol(cmdline + 7, NULL, 10));
    }
  } else if (stricmp("VBETEST", cmdline) == 0) {
    cmd_vbetest();
  } else if (stricmp("GET_BUILD_INFO", cmdline) == 0) {
    printf("Build Time: %s %s\n", __DATE__, __TIME__);
    return;
  } else if (strincmp("DIR", cmdline, 3) == 0) {
    char* args[1];
    args[0] = (char*)malloc(256);
    Get_Arg(args[0], cmdline, 1);
    cmd_dir(args);
    free(args[0]);
    return;
  } else if (stricmp("NTPTIME", cmdline) == 0) {
    uint32_t ts = ntp_get_server_time(NTPServer2);
    uint32_t year, mon, day, hour, min, sec;
    UnNTPTimeStamp(ts, &year, &mon, &day, &hour, &min, &sec);
    printf("NTPTime:%04d\\%02d\\%02d %02d:%02d:%02d\n", year, mon, day, hour,
           min, sec);
    return;
  } else if (stricmp("TL", cmdline) == 0) {
    cmd_tl();
    return;
  } else if (strincmp("MD5S ", cmdline, 5) == 0) {
    unsigned char r[16];
    printf("\"%s\" = ", cmdline + 5);
    md5s(cmdline + 5, strlen(cmdline + 5), (char*)r);
    for (int i = 0; i < 16; i++)
      printf("%02x", r[i]);
    printf("\n");
  } else if (strincmp("MD5F ", cmdline, 5) == 0) {
    unsigned char r[16];
    printf("\"%s\" = ", cmdline + 5);
    md5f(cmdline + 5, r);
    for (int i = 0; i < 16; i++)
      printf("%02x", r[i]);
    printf("\n");
  } else if (strincmp("KILL ", cmdline, 5) == 0) {
    cmdline += 5;
    for (int i = 0; get_task(i) != 0; i++) {
      if (strtol(cmdline, NULL, 10) == i) {
        if (get_running_task_num() == 1) {
          printf("Cannot kill the last task.\n");
          return;
        }
        if (!get_task(i)->app) {
          printf("Cannot kill the system task.\n");
          return;
        }
        task_delete(get_task(i));
        return;
      }
    }
    printf("No such task.\n");
    return;
  } else if (strincmp(cmdline, "CMDEDIT ", 8) == 0) {
    char file[50] = {0};
    char* file_buf = malloc(500);
    Get_Arg(file, cmdline, 1);
    Get_Arg(file_buf, cmdline, 2);
    EDIT_FILE(file, file_buf, strlen(file_buf), 0);
    free(file_buf);
  } else if (strincmp("TYPE ", cmdline, 5) == 0) {
    type_deal(cmdline);
    return;
  } else if (stricmp("CLS", cmdline) == 0) {
    clear();
    return;
  } else if (stricmp("PAUSE", cmdline) == 0) {
    printf("Press any key to continue. . .");
    getch();
    printf("\n");
  } else if (stricmp("VER", cmdline) == 0) {
    printf("Powerint DOS 386 Version %s\n", VERSION);
    print("Copyright (C) 2021-2022 zhouzhihao & min0911\n");
    print("THANKS Link TOOLS BY Kawai\n\n");
    printf("C Build tools by GNU C Compiler\n");
    printf("ASM Build tools by NASM\n\n");
    printf("I love you Kawai\n");
    return;
  } else if (stricmp("TIME", cmdline) == 0) {
    char* time = "The current time is:00:00:00";
    io_out8(0x70, 0);
    c = io_in8(0x71);
    time[27] = (c & 0x0f) + 0x30;
    time[26] = (c >> 4) + 0x30;
    io_out8(0x70, 2);
    c = io_in8(0x71);
    time[24] = (c & 0x0f) + 0x30;
    time[23] = (c >> 4) + 0x30;
    io_out8(0x70, 4);
    c = io_in8(0x71);
    time[21] = (c & 0x0f) + 0x30;
    time[20] = (c >> 4) + 0x30;
    print(time);
    print("\n\n");
    return;
  } else if (stricmp("DATE", cmdline) == 0) {
    char* date = "The current date is:2000\\00\\00,";
    io_out8(0x70, 9);
    c = io_in8(0x71);
    date[23] = (c & 0x0f) + 0x30;
    date[22] = (c >> 4) + 0x30;
    io_out8(0x70, 8);
    c = io_in8(0x71);
    date[26] = (c & 0x0f) + 0x30;
    date[25] = (c >> 4) + 0x30;
    io_out8(0x70, 7);
    c = io_in8(0x71);
    date[29] = (c & 0x0f) + 0x30;
    date[28] = (c >> 4) + 0x30;
    print(date);
    io_out8(0x70, 6);
    c = io_in8(0x71);
    if (c == 1)
      print("Sunday");
    if (c == 2)
      print("Monday");
    if (c == 3)
      print("Tuesday");
    if (c == 4)
      print("Wednesday");
    if (c == 5)
      print("Thursday");
    if (c == 6)
      print("Friday");
    if (c == 7)
      print("Saturday");
    print("\n\n");
    return;
  } else if (stricmp("PCILS", cmdline) == 0) {
    pci_list();
  } else if (strincmp("ECHO ", cmdline, 5) == 0) {
    print(cmdline + 5);
    print("\n");
    return;
  } else if (strincmp("MKDIR ", cmdline, 6) == 0) {
    vfs_createdict(cmdline + 6);
  } else if (strincmp("POKE ", cmdline, 5) == 0) {
    addr = (ascii2num(cmdline[5]) >> 28) + (ascii2num(cmdline[6]) >> 24);
    addr = addr + (ascii2num(cmdline[7]) >> 20) + (ascii2num(cmdline[8]) >> 16);
    addr = addr + (ascii2num(cmdline[9]) >> 12) + (ascii2num(cmdline[10]) >> 8);
    addr = addr + (ascii2num(cmdline[11]) >> 4) + ascii2num(cmdline[12]);
    p = (char*)addr;
    c = (ascii2num(cmdline[14]) >> 4) + ascii2num(cmdline[15]);
    p[0] = c;
    print("\n");
    return;
  } else if (strincmp("VISIT ", cmdline, 6) == 0) {
    addr = (ascii2num(cmdline[5]) >> 28) + (ascii2num(cmdline[6]) >> 24);
    addr = addr + (ascii2num(cmdline[7]) >> 20) + (ascii2num(cmdline[8]) >> 16);
    addr = addr + (ascii2num(cmdline[9]) >> 12) + (ascii2num(cmdline[10]) >> 8);
    addr = addr + (ascii2num(cmdline[11]) >> 4) + ascii2num(cmdline[12]);
    p = (char*)addr;
    c = p[0];
    printchar(num2ascii(c >> 4));
    printchar(num2ascii(c & 0x0f));
    print("\n");
    return;
  } else if (stricmp("PCINFO", cmdline) == 0) {
    pcinfo();
  } else if (stricmp("MEM", cmdline) == 0) {
    mem();
  } else if (strincmp("BEEP ", cmdline, 5) == 0) {
    int point, notes, dup;
    point = ascii2num(*(char*)(cmdline + 5));
    notes = ascii2num(*(char*)(cmdline + 7));
    dup = ascii2num(*(char*)(cmdline + 9));
    beep(point, notes, dup);
  } else if (stricmp("REBOOT", cmdline) == 0) {
    env_save();
    io_out8(0xcf9, 0x0e);
  } else if (stricmp("HALT", cmdline) == 0) {
    env_save();
    running_mode = POWERINTDOS;
    acpi_shutdown();
    SwitchToText8025_BIOS();
    clear();
    printf("\nYou can turn off your computer safely.");
    io_cli();
    while (1)
      ;
  } else if (strincmp("COLOR ", cmdline, 6) == 0) {
    struct TASK* task = current_task();
    unsigned char c = (ascii2num(cmdline[6]) << 4) + ascii2num(cmdline[7]);
    Text_Draw_Box(0, 0, task->TTY->xsize, task->TTY->ysize, c);
    task->TTY->color = c;
  } else if (strincmp("MKFILE ", cmdline, 7) == 0) {
    vfs_createfile(cmdline + 7);
    return;
  } else if (strincmp("DEL ", cmdline, 4) == 0) {
    if (vfs_delfile(cmdline + 4) == 0) {
      printf("File not find.\n\n");
    }
    return;
  } else if (strincmp("DELDIR ", cmdline, 7) == 0) {
    if (vfs_deldir(cmdline + 7) == 0) {
      printf("Directory tree not find.\n\n");
    }
    return;
  } else if (strincmp("DELTREE ", cmdline, 8) == 0) {
    if (vfs_deldir(cmdline + 7) == 0) {
      printf("Directory tree not find.\n\n");
    }
    return;
  } else if (strincmp("FONT ", cmdline, 5) == 0) {
    Set_Font(cmdline + 5);
  } else if (strincmp("CD ", cmdline, 3) == 0) {
    if (vfs_change_path(cmdline + 3) == 0) {
      printf("Invalid directory.\n\n");
    }
  } else if (strincmp("RENAME ", cmdline, 7) == 0) {
    if (Get_Argc(cmdline) < 2) {
      printf("Usage: RENAME <src_name> <dst_name>\n");
      return;
    }
    char* src_name = (char*)malloc(100);
    char* dst_name = (char*)malloc(100);
    Get_Arg(src_name, cmdline, 1);
    Get_Arg(dst_name, cmdline, 2);
    if (vfs_renamefile(src_name, dst_name) == 0) {
      printf("File not find.\n\n");
    }
    free(src_name);
    free(dst_name);
  } else if (strincmp("ATTRIB ", cmdline, 7) == 0) {
    char* filename = (char*)malloc(100);
    Get_Arg(filename, cmdline, 1);
    if (fsz(filename) == -1) {
      printf("File not find.\n\n");
      free(filename);
      return;
    }
    char* type = (char*)malloc(100);
    Get_Arg(type, cmdline, 2);
    if (stricmp("READ-ONLY", type) == 0) {
      vfs_attrib(filename, RDO);
    } else if (stricmp("HIDE", type) == 0) {
      vfs_attrib(filename, HID);
    } else if (stricmp("SYSTEM-FILE", type) == 0) {
      vfs_attrib(filename, SYS);
    } else if (stricmp("FILE", type) == 0) {
      vfs_attrib(filename, FLE);
    } else {
      printf("Undefined type.\n\n");
    }
    free(filename);
    free(type);
  } else if (strincmp("PATH ", cmdline, 5) == 0) {
    char* buf = (char*)malloc(1024);
    Get_Arg(buf, cmdline, 1);
    EDIT_FILE("tskdrv:\\path.sys", buf, strlen(buf), 0);
    Path_Addr = (uint32_t)buf;
  } else if (strincmp("RDRV ", cmdline, 5) == 0) {
    if (!vfs_check_mount(cmdline[5])) {
      if (!vfs_mount_disk(cmdline[5], cmdline[5])) {
        printf("Disk not ready!\n");
      } else {
        vfs_change_disk(cmdline[5]);
      }
    } else {
      vfs_unmount_disk(cmdline[5]);
      if (!vfs_mount_disk(cmdline[5], cmdline[5])) {
        printf("Disk not ready!\n");
      } else {
        vfs_change_disk(cmdline[5]);
      }
    }
  } else if(strincmp(cmdline,"set ",4) == 0) {
    char buf[100]; // 变量名称
    char buf1[100]; // 值
    Get_Arg(buf,cmdline,1);
    Get_Arg(buf1,cmdline,2);
    env_write(buf,buf1);
  } 
  else if(stricmp(cmdline,"env") == 0) {
    extern MST_Object* env;
    char *s = MST_build_to_string(env);
    printf("%s\n",s);
    free(s);
  }
  else if (cmdline[1] == ':' && cmdline[2] == '\0') {
    if (!vfs_check_mount(cmdline[0])) {
      if (!vfs_mount_disk(cmdline[0], cmdline[0])) {
        printf("Disk not ready!\n");
      } else {
        vfs_change_disk(cmdline[0]);
      }
    } else {
      vfs_change_disk(cmdline[0]);
    }
  } else {
    if (cmd_app(cmdline) == 0) {
      if (run_bat(cmdline) == 0) {
        print("Bad Command!\n\n");
        return;
      }
    }
  }
}

void show_heap(memory* mem) {
  freeinfo* finf = mem->freeinf;
  while (finf) {
    for (int i = 0; i < FREE_MAX_NUM; i++) {
      if (finf->f[i].start == 0 && finf->f[i].end == 0) {
        break;
      }
      printf("START: %08x END: %08x SIZE: %08x Bytes\n", finf->f[i].start, finf->f[i].end,
             finf->f[i].end - finf->f[i].start);
    }
    finf = finf->next;
  }
}

void pci_list() {
  extern int PCI_ADDR_BASE;
  unsigned char* pci_drive = (unsigned char*)PCI_ADDR_BASE;
  //输出PCI表的内容
  for (int line = 0;; pci_drive += 0x110 + 4, line++) {
    if (pci_drive[0] == 0xff)
      PCI_ClassCode_Print((struct PCI_CONFIG_SPACE_PUCLIC*)(pci_drive + 12));
    else
      break;
  }
}

void cmd_dir(char** args) {
  vfs_file* file = vfs_fileinfo(args[0]);
  if (file != NULL) {
    printf("%s  %d  %04d-%02d-%02d %02d:%02d  ", file->name, file->size,
           file->year, file->month, file->day, file->hour, file->minute);
    if (file->type == FLE) {
      printf("FILE");
    } else if (file->type == RDO) {
      printf("READ-ONLY");
    } else if (file->type == HID) {
      printf("HIDE");
    } else if (file->type == SYS) {
      printf("SYSTEM-FILE");
    }
    printf("\n");
    free(file);
  } else {
    List* list_of_file = vfs_listfile(args[0]);
    for (int i = 1; FindForCount(i, list_of_file) != NULL; i++) {
      vfs_file* d = (vfs_file*)FindForCount(i, list_of_file)->val;
      int color = now_tty()->color;
      if (d->type == DIR) {
        now_tty()->color = 0x0a;
        printf("%s ", d->name);
        // free(d->name);
        now_tty()->color = color;
      } else if (d->type == RDO) {
        now_tty()->color = 0x0e;
        printf("%s ", d->name);
        now_tty()->color = color;
      } else if (d->type == HID) {
        now_tty()->color = 0x08;
        printf("%s ", d->name);
        now_tty()->color = color;
      } else if (d->type == SYS) {
        now_tty()->color = 0x0c;
        printf("%s ", d->name);
        now_tty()->color = color;
      } else {
        printf("%s ", d->name);
        // free(d->name);
      }
      free(d);
    }
    DeleteList(list_of_file);
    printf("\n");
  }
  return;
}

void type_deal(char* cmdline) {
  // type命令的实现
  char* name = cmdline + 5;

  int size = vfs_filesize(cmdline + 5);
  if (size == -1) {
    print(name);
    print(" not found!\n\n");
  } else {
    FILE* fp = fopen(name, "r");
    char* p = (char*)fp->buffer;
    // for (i = 0; i != finfo->size; i++) {
    //   printchar(p[i]);
    // }
    print(p);
    print("\n");
    fclose(fp);
  }
  return;
}

void pcinfo() {
  char cpu[100] = {0};
  int cpuid[3] = {get_cpu1(), get_cpu3(), get_cpu2()};
  //根据CPUID信息打印出来
  cpu[0] = cpuid[0] & 0xff;
  cpu[1] = (cpuid[0] >> 8) & 0xff;
  cpu[2] = (cpuid[0] >> 16) & 0xff;
  cpu[3] = (cpuid[0] >> 24) & 0xff;
  cpu[4] = cpuid[1] & 0xff;
  cpu[5] = (cpuid[1] >> 8) & 0xff;
  cpu[6] = (cpuid[1] >> 16) & 0xff;
  cpu[7] = (cpuid[1] >> 24) & 0xff;
  cpu[8] = cpuid[2] & 0xff;
  cpu[9] = (cpuid[2] >> 8) & 0xff;
  cpu[10] = (cpuid[2] >> 16) & 0xff;
  cpu[11] = (cpuid[2] >> 24) & 0xff;
  cpu[12] = 0;
  printf("CPU:%s ", cpu);
  char cpu1[100] = {0};
  getCPUBrand(cpu1);
  printf("Ram Size:%dMB\n", memsize / (1024 * 1024));
  return;
}
void mem() {
  int free = 0;
  for (int i = 0; i != 1024 * 768; i++) {
    extern struct PAGE_INFO* pages;
    if (pages[i].flag == 0)
      free++;
  }
  printf("free vpages:%d free kpages:%d\nfree:%dKB\n", free,
         free - (1024 * 768 - memsize / (4 * 1024)),
         (free - (1024 * 768 - memsize / (4 * 1024))) * 4);
  return;
}

void cmd_tl() {
  // tl：tasklist
  // 显示当前运行的任务
  extern int tasknum;  //任务数量（定义在task.c）
  for (int i = 0; i != tasknum + 1; i++) {
    struct TASK* task = get_task(i);
    printf("Task %d: Name:%s,Level:%d,Sleep:%d,GDT address:%d*8,Type:", i,
           task->name, task->level, task->sleep, task->sel / 8);
    if (task->is_child == 1) {
      printf("Thread\n");
    } else {
      printf("Task\n");
    }
  }
}
void cmd_vbetest() {
  get_all_mode();
}

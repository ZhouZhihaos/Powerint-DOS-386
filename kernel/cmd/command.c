// 命令行与命令处理
#include <cmd.h>
#include <dos.h>
#include <fcntl.h>
#include "../zlib/zlib.h"

void showPage(void);
void show_photo(char* path, vram_t* vr, int xsize);
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

void usertasktest() {
  while (1) {
    // do nothing
  }
}

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
/* GUI BMP32VIEW/JPGVIEW */
static vram_t* buf_view_window;
static struct SHEET* sht_view_window;
static int sheet_view_free_flag;
static void close_view_window() {
  sheet_free(sht_view_window);
  page_free(buf_view_window, 1029 * 792 * sizeof(color_t));
  sheet_view_free_flag = 0;
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
  if (IsHttpGetHeader(data, size).ok) {  // 是HTTP GetHeader
    /* 标头信息 */
    unsigned char head[500] = "HTTP/1.1 200 OK\r\n";
    unsigned char content_type[] = "Content-Type: text/html\r\n";
    unsigned char content_length[100];
    unsigned char date[100];
    printf("Is Http get header!\n");
    if (strlen(IsHttpGetHeader(data, size).path) ==
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
        if (strcmp(f->path, IsHttpGetHeader(data, size).path + 1) ==
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
  if (NowTask()->line != cmdline) {
    strcpy(NowTask()->line, cmdline);
  }
  if (strincmp("FORMAT ", cmdline, 7) == 0) {
    int res = format(cmdline[7]);
    if (res == -1) {
      printf("Couldn't find tskdrv:\\boot.bin to format disk.\n\n");
    } else if (res == 0) {
      printf("Format OK.\n\n");
    } else if (res == 1) {
      printf("Couldn't find Disk.\n");
    }
  } else if (stricmp("FAT", cmdline) == 0) {
    struct TASK* task = NowTask();
    for (int i = 0; i != 3072; i++) {
      printf("%03x ", drive_ctl.drives[task->drive_number].fat[i]);
      if (!((i + 1) % (24 * 20))) {
        printf("Press any key to continue...");
        getch();
        printf("\n");
      }
    }
    printf("\n");
  } else if (stricmp("SHOWPAGE", cmdline) == 0) {
    showPage();
  } else if (strincmp("DESKTOP ", cmdline, 8) == 0 &&
             running_mode == POWERDESKTOP) {
    if (fsz(cmdline + 8) == -1) {
      return;
    }
    extern struct SHEET* sht_back;
    show_photo(cmdline + 8, sht_back->buf, sht_back->bxsize);
    sheet_refresh(sht_back, 0, 0, sht_back->bxsize, sht_back->bysize);
  } else if (stricmp("PCNET", cmdline) == 0) {
    init_pcnet_card();
  } else if (stricmp("CHAT", cmdline) == 0 && running_mode == POWERINTDOS) {
    chat_cmd();
  } else if (stricmp("CHAT", cmdline) == 0 && running_mode == POWERDESKTOP) {
    chat_gui();
  } else if (stricmp("NETGOBANG", cmdline) == 0) {
    netgobang();
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
      HttpFile* f = page_kmalloc(sizeof(HttpFile));
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
        socket = Socket_Alloc(TCP_PROTOCOL);
        Socket_Bind(socket, TCP_Socket_Handler);
      } else if (!p) {  // UDP
        socket = Socket_Alloc(UDP_PROTOCOL);
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
        Socket_Free(socket);
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
          Socket_Free(socket);
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
    socket = Socket_Alloc(UDP_PROTOCOL);
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
        mkfile(save_file_name);
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
    Socket_Free(socket);
  } else if (strincmp("NSLOOKUP ", cmdline, 9) == 0) {
    uint8_t* dns = (uint8_t*)page_malloc(strlen(cmdline + 9) + 1);
    memcpy(dns + 1, cmdline + 9, strlen(cmdline + 9));
    uint32_t ip = DNSParseIP(dns + 1);
    printf("DNS: %s -> IP: %d.%d.%d.%d\n", cmdline + 9, (uint8_t)(ip >> 24),
           (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip));
    page_free(dns, strlen(cmdline + 9) + 1);
  } else if (strincmp("PING ", cmdline, 5) == 0) {
    cmd_ping(cmdline + 5);
  } else if (strincmp("IPCONFIG", cmdline, 6) == 0) {
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
  } else if (strincmp("MOUNT ", cmdline, 6) == 0) {
    int c = mount(cmdline + 6);
    printf("mount file in %c:\\\n", c);
  } else if (strincmp("UNMOUNT ", cmdline, 8) == 0) {
    unmount(cmdline[8]);
  } else if (stricmp("SB16", cmdline) == 0) {
    // wav_player_test();
  } else if (stricmp("DISKLS", cmdline) == 0) {
    extern vdisk vdisk_ctl[255];

    for (int i = 0; i < 255; i++) {
      if (vdisk_ctl[i].flag) {
        printf("%c:\\ => TYPE: %s\n", i + ('A'), vdisk_ctl[i].DriveName);
      }
    }
  } else if (stricmp("ADD", cmdline) == 0) {
    if (running_mode == POWERINTDOS) {
      AddShell_TextMode();
    } else if (running_mode == HIGHTEXTMODE) {
      AddShell_HighTextMode();
    } else if (running_mode == POWERDESKTOP) {
      AddShell_GraphicMode();
    }
  } else if (strincmp("SWITCH ", cmdline, 7) == 0) {
    if (running_mode == POWERINTDOS) {
      SwitchShell_TextMode(strtol(cmdline + 7, NULL, 10));
    } else if (running_mode == HIGHTEXTMODE) {
      SwitchShell_HighTextMode(strtol(cmdline + 7, NULL, 10));
    }
  } else if (stricmp("VBETEST", cmdline) == 0) {
    cmd_vbetest();
  } else if (strincmp("BMPVIEW32 ", cmdline, 10) == 0) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO* vbeinfo = (struct VBEINFO*)VBEINFO_ADDRESS;
      if (set_mode(1024, 768, 32) != 0) {
        printf("Can't enable 1024x768x32 VBE mode.\n\n");
        return;
      }
      BMPVIEW32(cmdline + 10, (unsigned char*)vbeinfo->vram, 1024);
      getch();
      SwitchToText8025_BIOS();
      clear();
      return;
    } else if (running_mode == POWERDESKTOP) {
      if (fsz(cmdline + 10) == -1) {
        printf("Can't find file %s!\n\n", cmdline + 10);
        return;
      }
      sheet_view_free_flag = 1;
      extern struct SHTCTL* shtctl;
      buf_view_window = (vram_t*)page_malloc(1029 * 792 * sizeof(color_t));
      sht_view_window = MakeWindow(50, 50, 1029, 792, "bmpview32", shtctl,
                                   buf_view_window, close_view_window);
      sheet_updown(sht_view_window, shtctl->top - 1);
      BMPVIEW32(cmdline + 10, (unsigned char*)(buf_view_window + 24 * 1029 + 3),
                1029);
      sheet_refresh(sht_view_window, 0, 0, 1029, 792);
      while (sheet_view_free_flag)
        ;
    }
  } else if (stricmp("GET_BUILD_INFO", cmdline) == 0) {
    printf("Build Time: %s %s\n", __DATE__, __TIME__);
    return;
  } else if (strincmp("JPGVIEW ", cmdline, 8) == 0) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO* vbeinfo = (struct VBEINFO*)VBEINFO_ADDRESS;
      if (set_mode(1024, 768, 32) != 0) {
        printf("Can't enable 1024x768x32 VBE mode.\n\n");
        return;
      }
      jpgview32(cmdline + 8, (unsigned char*)vbeinfo->vram, 1024);
      getch();
      SwitchToText8025_BIOS();
      clear();
      return;
    } else if (running_mode == POWERDESKTOP) {
      if (fsz(cmdline + 8) == -1) {
        printf("Can't find file %s!\n\n", cmdline + 8);
        return;
      }
      sheet_view_free_flag = 1;
      extern struct SHTCTL* shtctl;
      buf_view_window = (vram_t*)page_malloc(1029 * 792 * sizeof(color_t));
      sht_view_window = MakeWindow(50, 50, 1029, 792, "jpgview", shtctl,
                                   buf_view_window, close_view_window);
      sheet_updown(sht_view_window, shtctl->top - 1);
      show_photo(cmdline + 8, buf_view_window + 24 * 1029 + 3, 1029);
      sheet_refresh(sht_view_window, 0, 0, 1029, 792);
      while (sheet_view_free_flag)
        ;
    }
  } else if (strincmp("PRAVIEW ", cmdline, 8) == 0 &&
             running_mode == POWERINTDOS) {
    char* path = malloc(strlen(cmdline) - 7);
    strcpy(path, cmdline + 8);
    struct VBEINFO* vbeinfo = (struct VBEINFO*)VBEINFO_ADDRESS;
    if (set_mode(1024, 768, 32) != 0) {
      printf("Can't enable 1024x768x32 VBE mode.\n\n");
      return;
    }
    pra_view_32((unsigned char*)path, (unsigned char*)vbeinfo->vram, 1024);
    getch();
    SwitchToText8025_BIOS();
    clear();
    free(path);
    return;
  } else if (stricmp("DIR", cmdline) == 0) {
    cmd_dir();
    return;
  } else if (stricmp("NTPTIME", cmdline) == 0) {
    uint32_t ts = GetNTPServerTime(NTPServer2);
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
    for (int i = 0; GetTask(i) != 0; i++) {
      if (strtol(cmdline, NULL, 10) == i) {
        if (Get_Running_Task_Num() == 1) {
          printf("Cannot kill the last task.\n");
          return;
        }
        if (!GetTask(i)->app) {
          printf("Cannot kill the system task.\n");
          return;
        }
        SubTask(GetTask(i));
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
  } else if (stricmp("PRASHELL", cmdline) == 0) {
    PraShell();
  } else if (strincmp("ECHO ", cmdline, 5) == 0) {
    print(cmdline + 5);
    print("\n");
    return;
  } else if (strincmp("MKDIR ", cmdline, 6) == 0) {
    vfs_createdict(cmdline + 6);
  } else if (stricmp("TREE", cmdline) == 0) {
    // tree(NowTask()->directory);
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
  } else if (strincmp("BMPVIEW ", cmdline, 8) == 0 &&
             running_mode == POWERINTDOS) {
    bmpview(cmdline + 8);
    char c;
    for (;;) {
      c = input_char_inSM();
      if (c == 0x01) {
        SwitchToText8025_BIOS();
        break;
      }
    }
    return;
  } else if (strincmp("BEEP ", cmdline, 5) == 0) {
    int point, notes, dup;
    point = ascii2num(*(char*)(cmdline + 5));
    notes = ascii2num(*(char*)(cmdline + 7));
    dup = ascii2num(*(char*)(cmdline + 9));
    beep(point, notes, dup);
  } else if (stricmp("REBOOT", cmdline) == 0) {
    io_out8(0xcf9, 0x0e);
  } else if (stricmp("HALT", cmdline) == 0) {
    running_mode = POWERINTDOS;
    acpi_shutdown();
    SwitchToText8025_BIOS();
    clear();
    printf("\nYou can turn off your computer safely.");
    io_cli();
    while (1)
      ;
  } else if (strincmp("ZIP ", cmdline, 4) == 0) {
    if (Get_Argc(cmdline) < 2) {
      printf("Usage: ZIP <infile> <outfile>\n\n");
      return;
    }
    char* asm1 = page_malloc(100);
    char* out = page_malloc(100);
    Get_Arg(asm1, cmdline, 1);
    Get_Arg(out, cmdline, 2);
    compress_one_file(asm1, out);
  } else if (strincmp("UZIP ", cmdline, 5) == 0) {
    if (Get_Argc(cmdline) < 2) {
      printf("Usage: UZIP <infile> <outfile>\n\n");
      return;
    }
    char* asm1 = page_malloc(100);
    char* out = page_malloc(100);
    Get_Arg(asm1, cmdline, 1);
    Get_Arg(out, cmdline, 2);
    decompress_one_file(asm1, out);
  } else if (strincmp("COLOR ", cmdline, 6) == 0) {
    struct TASK* task = NowTask();
    unsigned char c = (ascii2num(cmdline[6]) << 4) + ascii2num(cmdline[7]);
    Text_Draw_Box(0, 0, task->TTY->xsize, task->TTY->ysize, c);
    task->TTY->color = c;
  } else if (strincmp("CASM ", cmdline, 5) == 0) {
    if (Get_Argc(cmdline) < 2) {
      printf("Usage: CASM <asmfile> <outfile>\n\n");
      return;
    }
    char* asm1 = page_malloc(100);
    char* out = page_malloc(100);
    Get_Arg(asm1, cmdline, 1);
    Get_Arg(out, cmdline, 2);
    if (fsz(asm1) == -1) {
      printf("%s not find!\n\n", asm1);
      return;
    }
    if (fsz(out) == -1) {
      mkfile(out);
    }
    FILE* fp_asm = fopen(asm1, "wb");
    FILE* fp_out = fopen(out, "wb");
    compile_file(fp_asm, fp_out);
    fclose(fp_asm);
    fclose(fp_out);
    FILE* fp = fopen(out, "r");
    for (int i = 0; i != fp->fileSize; i++) {
      printf("%02x ", (unsigned char)fp->buffer[i]);
    }
    printf("\n");
    fclose(fp);
    page_free(asm1, 100);
    page_free(out, 100);
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
    // else if (strincmp("EDIT ", cmdline, 5) == 0)
    // {
    // 	edit(cmdline);
    // 	return;
    // }
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
    // char* filename = (char*)malloc(100);
    // Get_Arg(filename, cmdline, 1);
    // if (fsz(filename) == -1) {
    //   printf("File not find.\n\n");
    //   free(filename);
    //   return;
    // }
    // char* type = (char*)malloc(100);
    // Get_Arg(type, cmdline, 2);
    // if (stricmp("READ-ONLY", type) == 0) {
    //   attrib(filename, 0x01);
    // } else if (stricmp("HIDE", type) == 0) {
    //   attrib(filename, 0x02);
    // } else if (stricmp("SYSTEM-FILE", type) == 0) {
    //   attrib(filename, 0x04);
    // } else if (stricmp("FILE", type) == 0) {
    //   attrib(filename, 0x20);
    // } else {
    //   printf("Undefined type.\n\n");
    // }
    // free(filename);
    // free(type);
  } else if (strincmp("PATH ", cmdline, 5) == 0) {
    char* buf = (char*)malloc(1024);
    Get_Arg(buf, cmdline, 1);
    EDIT_FILE("tskdrv:\\path.sys", buf, strlen(buf), 0);
  } else if (cmdline[1] == ':' && cmdline[2] == '\0') {
    if (!vfs_change_disk(cmdline[0])) {
      if (!vfs_mount_disk(cmdline[0], cmdline[0])) {
        printf("Disk not ready!\n");
      } else {
        vfs_change_disk(cmdline[0]);
      }
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

void print_date(unsigned short _date, unsigned short _time) {
  //打印日期
  unsigned short year = _date & 65024;
  year = year >> 9;
  unsigned short month = _date & 480;
  month = month >> 5;
  unsigned short day = _date & 31;

  unsigned short hour = _time & 63488;
  hour = hour >> 11;
  unsigned short minute = _time & 2016;
  minute = minute >> 5;
  printf("%04d-%02d-%02d %02d:%02d", (year + 1980), month, day, hour, minute);
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

static void dir(struct TASK* task, struct FILEINFO* finfo, bool hide) {}
void cmd_dir() {
  // DIR命令的实现
  // struct TASK* task = NowTask();
  // char arg[20], arg2[20];
  // char* cmp = arg;
  // bool hide = true;
  // arg[0] = '\0';
  // arg2[0] = '\0';
  // Get_Arg(arg, task->line, 1);
  // Get_Arg(arg2, task->line, 2);
  // if (strcmp(arg, "-h") == 0 || strcmp(arg2, "-h") == 0) {
  //   hide = false;
  //   if (strcmp(arg, "-h") == 0) {
  //     cmp = arg2;
  //   }
  // }
  // if (strcmp(cmp, "-s") == 0) {
  //   struct List* list_dir = NewList();
  //   dir(task, task->directory, hide);
  //   AddVal((int)task->directory, list_dir);
  //   for (int k = 1; FindForCount(k, list_dir) != NULL; k++) {
  //     struct FILEINFO* finfo = (struct FILEINFO*)FindForCount(k,
  //     list_dir)->val; for (int i = 0; i !=
  //     drive_ctl.drives[task->drive_number].RootMaxFiles;
  //          i++) {
  //       if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
  //         if (strncmp((char*)finfo[i].name, ".       ", 8) == 0 ||
  //             strncmp((char*)finfo[i].name, "..      ", 8) == 0) {
  //           continue;
  //         }
  //         for (int j = 1; FindForCount(j, (struct List*)drive_ctl
  //                                             .drives[task->drive_number]
  //                                             .directory_clustno_list) !=
  //                                             NULL;
  //              j++) {
  //           struct List* list = FindForCount(
  //               j, (struct List*)drive_ctl.drives[task->drive_number]
  //                      .directory_clustno_list);
  //           if (list->val == finfo[i].clustno) {
  //             list = FindForCount(
  //                 j, (struct List*)drive_ctl.drives[task->drive_number]
  //                        .directory_list);
  //             dir(task, (struct FILEINFO*)list->val, hide);
  //             AddVal(list->val, list_dir);
  //             break;
  //           }
  //         }
  //       }
  //       if (finfo[i].name[0] == 0) {
  //         break;
  //       }
  //     }
  //   }
  // } else if (cmp[0] == '\0' || cmp[0] == ' ') {
  //   dir(task, task->directory, hide);
  // } else {
  //   struct List* list = Get_wildcard_File_Address(cmp);
  //   if (list != (struct List*)NULL) {
  //     void* tm = malloc(32 * 256);
  //     for (int i = 1; FindForCount(i, list) != NULL; i++) {
  //       struct FILEINFO* finfo = (struct FILEINFO*)FindForCount(i,
  //       list)->val; memcpy(tm + (i - 1) * 32, (void*)finfo, 32);
  //     }
  //     dir(task, (struct FILEINFO*)tm, hide);
  //     free(tm);
  //     return;
  //   }
  //   struct FILEINFO* finfo = Get_File_Address(cmp);
  //   if (finfo != 0) {
  //     char t = finfo[1].name[0];
  //     finfo[1].name[0] = '\0';
  //     dir(task, finfo, hide);
  //     finfo[1].name[0] = t;
  //     return;
  //   }
  //   finfo = Get_dictaddr(cmp);
  //   if (finfo != 0) {
  //     for (int i = 1;
  //          FindForCount(i, (struct List*)drive_ctl.drives[task->drive_number]
  //                              .directory_clustno_list) != NULL;
  //          i++) {
  //       struct List* list =
  //           FindForCount(i, (struct
  //           List*)drive_ctl.drives[task->drive_number]
  //                               .directory_clustno_list);
  //       if (list->val == finfo->clustno) {
  //         list =
  //             FindForCount(i, (struct
  //             List*)drive_ctl.drives[task->drive_number]
  //                                 .directory_list);
  //         dir(task, (struct FILEINFO*)list->val, hide);
  //         return;
  //       }
  //     }
  //   }
  //   printf("Arg error or file or directory not find!\n");
  // }
  List* list_of_file = vfs_listfile("");
  for (int i = 1; FindForCount(i, list_of_file) != NULL; i++) {
    vfs_dict* d = (vfs_dict*)FindForCount(i, list_of_file)->val;
    if (d->type == DIR) {
      int color = now_tty()->color;
      now_tty()->color = 0x0a;
      printf("%s ", d->name);
      free(d->name);
      now_tty()->color = color;
    } else {
      printf("%s ", d->name);
      // free(d->name);
    }
    free(d);
  }
  DeleteList(list_of_file);
  printf("\n");
  return;
}

void tree(struct FILEINFO* directory) {
  struct TASK* task = NowTask();
  struct FILEINFO* finfo = directory;
  struct List* list_ = NewList();
  int directory_num = 0;
  int root_file_num = 0;
  for (; directory[root_file_num].name[0] != '\0'; root_file_num++)
    ;
  for (; directory[root_file_num].type != 0x10; root_file_num--)
    ;
  printf("|-%c:\\%s>\n", task->drive, task->path);
  for (;;) {
    for (int i = 0; finfo[i].name[0] != '\0'; i++) {
      if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
        if (strncmp((char*)finfo[i].name, ".       ", 8) == 0 ||
            strncmp((char*)finfo[i].name, "..      ", 8) == 0) {
          continue;
        } else {
          for (int j = 1; FindForCount(j, (struct List*)drive_ctl
                                              .drives[task->drive_number]
                                              .directory_clustno_list) != NULL;
               j++) {
            struct List* list = FindForCount(
                j, (struct List*)drive_ctl.drives[task->drive_number]
                       .directory_clustno_list);
            if (list->val == finfo[i].clustno) {
              list = FindForCount(
                  j, (struct List*)drive_ctl.drives[task->drive_number]
                         .directory_list);
              printf("|-");
              for (int k = 0; k != directory_num; k++) {
                printf("    ");
              }
              printf("|---");
              for (int k = 0; finfo[i].name[k] != ' '; k++) {
                printf("%c", finfo[i].name[k]);
              }
              printf("\n");
              AddVal((int)(finfo + i), list_);
              finfo = (struct FILEINFO*)list->val;
              i = 0;
              directory_num++;
              break;
            }
          }
        }
      }
    }
    if (finfo == directory + root_file_num + 1) {
      break;
    }
    struct FILEINFO* last =
        (struct FILEINFO*)FindForCount(directory_num, list_)->val;
    finfo = last + 1;
    DeleteVal(directory_num, list_);
    directory_num--;
  }
  return;
}

void type_deal(char* cmdline) {
  // type命令的实现
  char* name = cmdline + 5;

  int size = vfs_filesize(cmdline+5);
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
    struct TASK* task = GetTask(i);
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

int compress_one_file(char* infilename, char* outfilename) {
  int num_read = 0;
  char* buffer;
  char* end;
  char inbuffer[128];
  int fg = 0;
  int p = 0;
  int sz;
  unsigned long total_read = 0, total_wrote = 0;
  (void)(total_wrote);
  (void)(fg);
  (void)(end);
  mkfile(outfilename);
  FILE* infile = fopen(infilename, "rb");
  buffer = malloc(infile->fileSize);
  sz = infile->fileSize;
  end = buffer + infile->fileSize;
  fread(buffer, infile->fileSize, 1, infile);
  fclose(infile);
  gzFile outfile = gzopen(outfilename, "wb");
  if (!infile || !outfile) {
    return -1;
  }
  printk("OPEN OK.\n");
  while (1) {
    if (p >= sz) {
      break;
    } else if (sz - p >= 128) {
      printk("=128\n");
      memcpy(inbuffer, buffer, 128);
      buffer += 128;
      p += 128;
      num_read = 128;
    } else {
      printk("sz-p=%d\n", sz - p);
      memcpy(inbuffer, buffer, sz - p);
      num_read = sz - p;
      buffer += sz - p;
      p += sz - p;
    }

    total_read += num_read;
    gzwrite(outfile, inbuffer, num_read);
    memset(inbuffer, 0, 128);
    printk("total_read=%d\n", total_read);
  }
  printk("%d\n", total_read);
  printk("\nfclose(infile)\n");
  // fclose(infile);
  printk("gzclose(outfile)\n");
  gzclose(outfile);
  printk("ALL DONE\n");
  return 0;
}
int decompress_one_file(char* infilename, char* outfilename) {
  int num_read = 0;
  static char buffer[128] = {0};
  unsigned char* buffer2;
  unsigned int p = 0;
  int sz = 0;
  mkfile(outfilename);
  gzFile infile = gzopen(infilename, "rb");
  // FILE* outfile = fopen(outfilename, "wb");
  while ((num_read = gzread(infile, buffer, sizeof(buffer))) > 0) {
    // fwrite(buffer, 1, num_read, outfile);
    // printf("p=%d,num_read=%d\n", p, num_read);
    // for (int i = 0; i < num_read; i++) {
    //   buffer2[p++] = buffer[i];
    //   printk("buffer2[%d]=%c\n", p - 1, buffer[i]);
    //   printk("Now buffer2[%d] = %c\n", p - 1, buffer2[p - 1]);
    // }
    sz += num_read;

    // memset(buffer, 0, 128);
  }
  gzseek(infile, 0, SEEK_SET);
  buffer2 = page_kmalloc(sz);
  while ((num_read = gzread(infile, buffer, sizeof(buffer))) > 0) {
    for (int i = 0; i < num_read; i++) {
      buffer2[p++] = buffer[i];
    }
  }
  gzclose(infile);
  // fclose(outfile);
  char buffer3[60];
  sprintf(buffer3, "del %s", outfilename);
  del(buffer3);
  mkfile(outfilename);
  for (int i = 0; i < sz; i++) {
    printk("%02x ", buffer2[i]);
  }
  EDIT_FILE(outfilename, (char*)buffer2, sz, 0);
  printk("size=%d\n", sz);
  // printk("%s\n", buffer2);
  page_kfree((int)buffer2, sz);
  return 0;
}

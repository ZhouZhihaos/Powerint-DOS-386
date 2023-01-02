// 命令行与命令处理
#include <cmd.h>
#include <dos.h>
#include <fcntl.h>
#include "../zlib/zlib.h"
int disk_Size(char drive);
#define _NSETJMP 10
int getReadyDisk();  // init.c
typedef long jmp_buf[_NSETJMP];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

typedef struct {
  uint32_t eax, ebx, ecx, edx, esi, edi, esp, eip, ret
} jmpbuf;
typedef struct {
  char path[50];
  char* buf;
} HttpFile;
unsigned char cons_color = 0x0f;  // 颜色
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
jmp_buf j;
unsigned char * ramdisk;
void TestRead(char drive,
              unsigned char* buffer,
              unsigned int number,
              unsigned int lba) {
  //printk("TestRW:Read Lba %d,Read Sectors number %d\n", lba, number);
  memcpy(buffer,ramdisk + lba * 512,number*512);
}
void TestWrite(char drive,
               unsigned char* buffer,
               unsigned int number,
               unsigned int lba) {
 // printk("TestRW:Write Lba %d,Write Sectors number %d\n", lba, number);
  memcpy(ramdisk + lba * 512,buffer,number*512);
}
void sjtest1() {
  printk("exception raised\n");
  longjmp(j, 3); /* jump to exception handler */
  printk("this line should never appear\n");
}
void sjtest() {
  int ret = setjmp(j);
  if (ret == 0) {
    printk("\''setjmp\'' is initializing \''j\''\n");
    sjtest1();
    printk("this line should never appear\n");
  } else {
    printk("''setjmp'' was just jumped into\n");
    /* this code is the exception handler */
  }
}
static void send_task() {
  SendIPCMessage(2, "Hello Task_sr2! I'm Send.",
                 strlen("Hello Task_sr2! I'm Send."),
                 synchronous);  // 给Task_sr2发信息（同步）
  while (1)
    ;
}
static vram_t* buf_view_window;
static struct SHEET* sht_view_window;
static int sheet_view_free_flag;
static void close_view_window() {
  sheet_free(sht_view_window);
  page_free((int)buf_view_window, 1029 * 792 * sizeof(color_t));
  sheet_view_free_flag = 0;
}
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
static unsigned char* html_file;
static void HTTP_Socket_Handler(struct Socket* socket, void* base) {
  struct IPV4Message* ipv4 =
      (struct IPV4Message*)(base + sizeof(struct EthernetFrame_head));
  struct TCPMessage* tcp =
      (struct TCPMessage*)(base + sizeof(struct EthernetFrame_head) +
                           sizeof(struct IPV4Message));
  uint16_t size = swap16(ipv4->totalLength) - sizeof(struct IPV4Message) -
                  (tcp->headerLength * 4);
  uint8_t* data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + (tcp->headerLength * 4);
  if (IsHttpGetHeader(data, size).ok) {
    unsigned char head[500] = "HTTP/1.1 200 OK\r\n";
    unsigned char content_type[] = "Content-Type: text/html\r\n";
    unsigned char content_length[100];
    unsigned char date[100];
    printf("Is Http get header!\n");
    if (strlen(IsHttpGetHeader(data, size).path) == 1) {
      printf("root\n");
      HttpFile* f = FindForCount(1, httpFileList)->val;
      printf("f->path = %s\n", f->path);
      html_file = f->buf;
    } else {
      printf("Not root.\n");
      for (int i = 1; FindForCount(i, httpFileList) != NULL; i++) {
        HttpFile* f = FindForCount(i, httpFileList)->val;
        // TODO: this
        if (strcmp(f->path, IsHttpGetHeader(data, size).path + 1) == 0) {
          html_file = f->buf;
          goto OK;
        }
      }
      html_file =
          "<html><head><title>404 Not Found</title></head><body bgcolor= white"
          "><center><h1>404 Not Found</h1></center><hr><center>Powerint DOS "
          "HTTP Server</center></body></html>";
      strcpy(head, "HTTP/1.1 404 Not Found\r\n");
    }
  OK:

    sprintf(content_length, "Content-Length: %d\r\n", strlen(html_file));
    strcat(head, content_type);
    strcat(head, content_length);
    GetNowDate(date);
    strcat(head, date);
    strcat(head, "\r\n");
    strcat(head, "\r\n");
    printf("%s", head);
    // unsigned char *head = "HTTP/1.1 200 OK\r\n";
    unsigned char* packet =
        (unsigned char*)page_malloc(strlen(head) + strlen(html_file) + 1);
    memcpy((void*)packet, (void*)head, strlen(head));
    memcpy((void*)(packet + strlen(head)), (void*)html_file, strlen(html_file));
    packet[strlen(head) + strlen(html_file) + 1] = NULL;
    socket->Send(socket, packet, strlen(packet) + 1);
  } else {
    printf("isn't http get header\n");
  }
}
static void SocketServerLoop(struct SocketServer* server) {
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
static unsigned int fudp_size;
static unsigned char* fudp_buffer;
static void FUDP_Socket_Handler(struct Socket* socket, void* base) {
  struct IPV4Message* ipv4 =
      (struct IPV4Message*)(base + sizeof(struct EthernetFrame_head));
  struct UDPMessage* udp =
      (struct UDPMessage*)(base + sizeof(struct EthernetFrame_head) +
                           sizeof(struct IPV4Message));
  uint8_t* data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
  fudp_size = swap16(udp->length) - sizeof(struct UDPMessage);
  fudp_buffer = malloc(fudp_size);
  memcpy((void*)fudp_buffer, (void*)data, fudp_size);
}

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
  int flag = 0;
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

  strtoupper(cmdline);  //转换成大写
  if (cmdline != line) {
    strcpy(line, cmdline);
  }
  if (strcmp("WIN", cmdline) == 0) {
    //进入图形化环境
    if (running_mode == POWERDESKTOP)
      return;
    running_mode = POWERDESKTOP;
    graphic();
    running_mode = POWERINTDOS;
    return;
  } else if (strncmp("FORMAT ", cmdline, 7) == 0) {
    // A,B盘——软盘
    // C盘——IDE/SATA硬盘主分区
    // D,E,F...盘——IDE/USB/SATA存储介质/分区
    FILE* fp = fopen("tskdrv:\\boot.bin", "r");
    void* read_in = page_malloc(fp->size);
    fread(read_in, fp->size, 1, fp);
    if (!(cmdline[7] - 'A')) {
      printf("3K FloppyDisk: %d bytes\n", 2880 * 512);
      printf("INT 13H DriveNumber: 0\n");
      printf("RootDictFiles: 224\n");
      printf(
          "drive_ctl.drives[%d].ClustnoBytes: 512 "
          "bytes\n",
          NowTask()->drive_number);
      *(unsigned char*)(&read_in[BPB_SecPerClus]) = 1;
      *(unsigned short*)(&read_in[BPB_RootEntCnt]) = 224;
      *(unsigned short*)(&read_in[BPB_TotSec16]) = 2880;
      *(unsigned int*)(&read_in[BPB_TotSec32]) = 2880;
      *(unsigned char*)(&read_in[BS_DrvNum]) = 0;
      write_floppy_for_ths(0, 0, 1, read_in, 1);
      unsigned int* fat = (unsigned int*)page_malloc(9 * 512);
      fat[0] = 0x00fffff0;
      write_floppy_for_ths(0, 0, 2, fat, 9);
      write_floppy_for_ths(0, 0, 11, fat, 9);
      page_free((void*)fat, 9 * 512);
      void* null_sec = page_malloc(512);
      for (int i = 0; i < 224 * 32 / 512; i++) {
        write_floppy_for_ths(0, 0, 20 + i, null_sec, 1);
      }
      page_free(null_sec, 512);
    } else if (cmdline[7] != 'B') {
      // struct IDEHardDiskInfomationBlock* info = drivers_idehdd_info();
      if (!DiskReady(cmdline[7]) && !have_vdisk(cmdline[7])) {
        printf("Couldn't find Disk.\n");
        return;
      } else if (DiskReady(cmdline[7])) {
        printf("IDE HardDisk ID:%s\n", ide_devices[cmdline[7] - 'C'].Model);
      }

      printf("Disk: %d bytes\n", disk_Size(cmdline[7]));
      printf("RootDictFiles: %d\n",
             14 * (((disk_Size(cmdline[7]) / 4096) / 512 + 1) * 512) / 32);
      printf("ClustnoBytes: %d bytes\n",
             ((disk_Size(cmdline[7]) / 4096) / 512 + 1) * 512);
      *(unsigned char*)(&read_in[BPB_SecPerClus]) =
          ((disk_Size(cmdline[7]) / 4096) / 512 + 1);
      *(unsigned short*)(&read_in[BPB_RootEntCnt]) =
          14 * (((disk_Size(cmdline[7]) / 4096) / 512 + 1) * 512) / 32;
      // printk("Sectors:%d\n", ide_devices[cmdline[7] - 'C'].Size /power
      if (disk_Size(cmdline[7]) / 512 > 65535) {
        *(unsigned short*)(&read_in[BPB_TotSec16]) = 0;
      } else {
        *(unsigned short*)(&read_in[BPB_TotSec16]) =
            disk_Size(cmdline[7]) / 512;
      }
      *(unsigned int*)(&read_in[BPB_TotSec32]) = disk_Size(cmdline[7]) / 512;
      *(unsigned char*)(&read_in[BS_DrvNum]) = cmdline[7] - 'C' + 0x80;
      Disk_Write(0, 1, (unsigned short*)read_in, cmdline[7]);
      unsigned int* fat = (unsigned int*)page_malloc(9 * 512);
      fat[0] = 0x00fffff0;
      Disk_Write(1, 9, (unsigned short*)fat, cmdline[7]);
      Disk_Write(10, 9, (unsigned short*)fat, cmdline[7]);
      page_free((void*)fat, 9 * 512);
      void* null_sec = page_malloc(512);
      clean((char*)null_sec, 512);
      for (int i = 0;
           i < 14 * (((disk_Size(cmdline[7]) / 4096) / 512 + 1) * 512) / 32 *
                   32 / 512;
           i++) {
        Disk_Write(19, 1, (unsigned short*)null_sec, cmdline[7]);
      }
      page_free(null_sec, 512);
      // page_free((void*)info, 256 * sizeof(short));
    }
    page_free(read_in, fp->size);
    fclose(fp);
  } else if (strcmp("FAT", cmdline) == 0) {
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
  } else if (strcmp("SHOWPAGE", cmdline) == 0) {
    showPage();
  } else if (strncmp("DESKTOP ", cmdline, 8) == 0 &&
             running_mode == POWERDESKTOP) {
    if (fsz(cmdline + 8) == -1) {
      return;
    }
    extern struct SHEET* sht_back;
    show_photo(cmdline + 8, sht_back->buf, sht_back->bxsize);
    sheet_refresh(sht_back, 0, 0, sht_back->bxsize, sht_back->bysize);
  } else if (strcmp("PCNET", cmdline) == 0) {
    init_pcnet_card();
  } else if (strcmp("CHAT", cmdline) == 0 && running_mode == POWERINTDOS) {
    chat_cmd();
  } else if (strcmp("CHAT", cmdline) == 0 && running_mode == POWERDESKTOP) {
    chat_gui();
  } else if (strcmp("NETGOBANG", cmdline) == 0) {
    netgobang();
  } else if (strncmp("HTTP ", cmdline, 5) == 0) {
    httpFileList = NewList();  // 创建文件链表
    for (int i = 1; i < Get_Argc(cmdline) + 1; i++) {
      char s[50];
      Get_Arg(s, cmdline, i);
      printf("#%d %s\n", i, s);
      FILE* fp = fopen(s, "rb");
      printf("fp=%08x\n", fp);
      HttpFile* f = page_kmalloc(sizeof(HttpFile));
      f->buf = fp->buf;
      strcpy(f->path, s);
      AddVal(f, httpFileList);
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
  } else if (strcmp("SOCKET", cmdline) == 0) {
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
      dstIP = IP2UINT32_T(buf);
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
      if (strcmp(inp, "exit") == 0 || strcmp(inp, "EXIT") == 0) {
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
        socket->Send(socket, inp, strlen(inp));
      } else if (m) {
        server->Send(server, inp, strlen(inp));
      }
    }
  } else if (strcmp("ARP", cmdline) == 0) {
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
  } else if (strncmp("FUDP ", cmdline, 5) == 0) {
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
    dstIP = IP2UINT32_T(buf);
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
      socket->Send(socket, fp->buf, fp->size);
      fclose(fp);
      free(send_file_name);
    }
    Socket_Free(socket);
  } else if (strncmp("NSLOOKUP ", cmdline, 9) == 0) {
    uint8_t* dns = (uint8_t*)page_malloc(strlen(cmdline + 9) + 1);
    memcpy(dns + 1, cmdline + 9, strlen(cmdline + 9));
    uint32_t ip = DNSParseIP(dns + 1);
    printf("DNS: %s -> IP: %d.%d.%d.%d\n", cmdline + 9, (uint8_t)(ip >> 24),
           (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip));
    page_free(dns, strlen(cmdline + 9) + 1);
  } else if (strncmp("PING ", cmdline, 5) == 0) {
    cmd_ping(cmdline + 5);
  } else if (strncmp("IPCONFIG", cmdline, 6) == 0) {
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
  } else if (strcmp("IPCTEST", cmdline) == 0) {
    unsigned char* stack = (unsigned char*)page_kmalloc(4 * 1024);
    struct TASK* send = AddTask("IPC", 3, 2 * 8, (int)send_task, 1 * 8, 1 * 8,
                                stack + 4 * 1024);
    sleep(2000);
    __SubTask(send);
    page_kfree((void*)stack, 4 * 1024);
  } else if (strcmp("FORK", cmdline) == 0) {
    // uint8_t* a = 0x12345678;
    // if (fork() == 0) {
    //   printf("I'm child process! --- %08x\n",a);
    //   SubTask(NowTask());
    // } else {
    //   printf("I'm parent process! --- %08x\n", a);
    //   free(a);
    // }
    // ChangeLevel(NowTask(),3); //  把当然任务的优先级别改成3（也就是shell的）
    vdisk vd;
    vd.flag = 1;
    vd.Read = TestRead;
    vd.Write = TestWrite;
    vd.size = 32*1024*1024;
    ramdisk = malloc(32*1024*1024);
    char c = register_vdisk(vd);
    printf("Mount Vdisk in %c:\\\n", c);
  } else if (strcmp("SB16", cmdline) == 0) {
    wav_player_test();
  } else if (strcmp("DISKLS", cmdline) == 0) {
    extern vdisk vdisk_ctl[255];
    for (int i = 0; i < getReadyDisk(); i++) {
      printf("%c:\\ => TYPE: IDE DRIVE\n", 'C' + i);
    }
    for (int i = 0; i < 255; i++) {
      if (vdisk_ctl[i].flag) {
        printf("%c:\\ => TYPE: VIRTUAL DISK\n", i + ('C' + getReadyDisk()));
      }
    }
  } else if (strncmp("SWITCH ", cmdline, 7) == 0 &&
             running_mode == POWERDESKTOP) {
    io_cli();  // 禁止中断产生
    extern struct VBEINFO* vbinfo;
    SDraw_Box(vbinfo->vram, 0, 0, vbinfo->xsize, vbinfo->ysize, COL_000000,
              vbinfo->xsize);
    int xsize = strtol(cmdline + 7, ' ', 10);
    int t = xsize, length = 1;
    while (t >= 10) {
      t /= 10;
      length++;
    }
    int ysize = strtol(cmdline + 7 + length, NULL, 10);
    if (set_mode(xsize, ysize, 32) != 0) {  // 彳亍不彳亍
      printf("Can't enable %dx%dx32 VBE mode.\n\n", xsize, ysize);
      return;
    }
    io_cli();
    struct VBEINFO* vbinfo0 = (struct VBEINFO*)VBEINFO_ADDRESS;
    extern struct SHTCTL* shtctl;
    extern struct SHEET* sht_back;
    extern vram_t* buf_back;
    extern struct SHEET* sht_mouse;
    // 不定 -> 定值
    vbinfo->vram = vbinfo0->vram;
    vbinfo->xsize = vbinfo0->xsize;
    vbinfo->ysize = vbinfo0->ysize;
    // shtctl更改
    shtctl->vram = (vram_t*)vbinfo->vram;
    shtctl->xsize = vbinfo->xsize;
    shtctl->ysize = vbinfo->ysize;
    // sht_back（背景）更改 重新malloc缓冲区+初始化并刷新
    page_free((void*)buf_back,
              sht_back->bxsize * sht_back->bysize * sizeof(color_t));
    buf_back =
        (vram_t*)page_malloc(vbinfo->xsize * vbinfo->ysize * sizeof(color_t));
    sht_back->buf = buf_back;
    sht_back->bxsize = vbinfo->xsize;
    sht_back->bysize = vbinfo->ysize;
    init_screen(buf_back, vbinfo->xsize, vbinfo->ysize);
    sheet_refresh(sht_back, 0, 0, vbinfo->xsize, vbinfo->ysize);
    for (int i = 1; shtctl->sheets0[i].flags != 0;
         i++) {  // 把每一个窗口的坐标设置成(0,0) 并且刷新（除sht_back）
      sheet_slide((struct SHEET*)&shtctl->sheets0[i], 0, 0);
      sheet_refresh((struct SHEET*)&shtctl->sheets0[i], 0, 0,
                    shtctl->sheets0[i].bxsize, shtctl->sheets0[i].bysize);
    }
    sheet_slide(sht_mouse, vbinfo->xsize / 2 - 8,
                vbinfo->ysize / 2 - 8);  // 鼠标重新居中
    io_sti();
  } else if (strcmp("VBETEST", cmdline) == 0) {
    cmd_vbetest();
  } else if (strncmp("BMPVIEW32 ", cmdline, 10) == 0) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO* vbeinfo = (struct VBEINFO*)VBEINFO_ADDRESS;
      if (set_mode(1024, 768, 32) != 0) {
        printf("Can't enable 1024x768x32 VBE mode.\n\n");
        return;
      }
      BMPVIEW32(cmdline + 10, vbeinfo->vram, 1024);
      getch();
      SwitchToText8025_BIOS();
      Set_Font(font);
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
                                   buf_view_window, (int)close_view_window);
      sheet_updown(sht_view_window, shtctl->top - 1);
      BMPVIEW32(cmdline + 10, buf_view_window + 24 * 1029 + 3, 1029);
      sheet_refresh(sht_view_window, 0, 0, 1029, 792);
      while (sheet_view_free_flag)
        ;
    }
  } else if (strcmp("GET_BUILD_INFO", cmdline) == 0) {
    printf("Build Time: %s %s\n", __DATE__, __TIME__);
    return;
  } else if (strncmp("JPGVIEW ", cmdline, 8) == 0) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO* vbeinfo = (struct VBEINFO*)VBEINFO_ADDRESS;
      if (set_mode(1024, 768, 32) != 0) {
        printf("Can't enable 1024x768x32 VBE mode.\n\n");
        return;
      }
      jpgview32(cmdline + 8, vbeinfo->vram, 1024);
      getch();
      SwitchToText8025_BIOS();
      Set_Font(font);
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
                                   buf_view_window, (int)close_view_window);
      sheet_updown(sht_view_window, shtctl->top - 1);
      show_photo(cmdline + 8, buf_view_window + 24 * 1029 + 3, 1029);
      sheet_refresh(sht_view_window, 0, 0, 1029, 792);
      while (sheet_view_free_flag)
        ;
    }
  } else if (strncmp("PRAVIEW ", cmdline, 8) == 0 &&
             running_mode == POWERINTDOS) {
    char* path = malloc(strlen(cmdline) - 7);
    strcpy(path, cmdline + 8);
    struct VBEINFO* vbeinfo = (struct VBEINFO*)VBEINFO_ADDRESS;
    if (set_mode(1024, 768, 32) != 0) {
      printf("Can't enable 1024x768x32 VBE mode.\n\n");
      return;
    }
    pra_view_32(path, vbeinfo->vram, 1024);
    getch();
    SwitchToText8025_BIOS();
    Set_Font(font);
    clear();
    free(path);
    return;
  } else if (strcmp("DIR", cmdline) == 0) {
    cmd_dir();
    return;
  } else if (strcmp("NTPTIME", cmdline) == 0) {
    uint32_t ts = GetNTPServerTime(NTPServer2);
    uint32_t year, mon, day, hour, min, sec;
    UnNTPTimeStamp(ts, &year, &mon, &day, &hour, &min, &sec);
    printf("NTPTime:%04d\\%02d\\%02d %02d:%02d:%02d\n", year, mon, day, hour,
           min, sec);
    return;
  } else if (strcmp("TL", cmdline) == 0) {
    cmd_tl();
    return;
  } else if (strncmp("MD5S ", cmdline, 5) == 0) {
    unsigned char r[16];
    printf("\"%s\" = ", cmdline + 5);
    md5s(cmdline + 5, strlen(cmdline + 5), r);
    for (int i = 0; i < 16; i++)
      printf("%02x", r[i]);
    printf("\n");
  } else if (strncmp("MD5F ", cmdline, 5) == 0) {
    unsigned char r[16];
    printf("\"%s\" = ", cmdline + 5);
    md5f(cmdline + 5, r);
    for (int i = 0; i < 16; i++)
      printf("%02x", r[i]);
    printf("\n");
  } else if (strncmp("KILL ", cmdline, 5) == 0) {
    cmdline += 5;
    for (int i = 0; GetTask(i) != 0; i++) {
      if (strtol(cmdline, NULL, 10) == i) {
        if (Get_Running_Task_Num() == 1) {
          printf("Cannot kill the last task.\n");
          return;
        }
        if (GetTask(i)->level == 1) {
          printf("Cannot kill the system task.\n");
          return;
        }
        GetTask(i)->running = 0;
        return;
      }
    }
    printf("No such task.\n");
    return;
  } else if (strncmp(cmdline, "CMDEDIT ", 8) == 0) {
    char file[50] = {0};
    char* file_buf = malloc(500);
    Get_Arg(file, cmdline, 1);
    Get_Arg(file_buf, cmdline, 2);
    EDIT_FILE(file, file_buf, strlen(file_buf), 0);
    free(file_buf);
  } else if (strcmp("CPPTEST", cmdline) == 0) {
    cpptest();
  } else if (strncmp("TYPE ", cmdline, 5) == 0) {
    type_deal(cmdline);
    return;
  } else if (strcmp("CLS", cmdline) == 0) {
    clear();
    return;
  } else if (strcmp("PAUSE", cmdline) == 0) {
    printf("Press any key to continue. . .");
    getch();
    printf("\n");
  } else if (strcmp("VER", cmdline) == 0) {
    printf("Powerint DOS 386 Version %s\n", VERSION);
    print("Copyright (C) 2021-2022 zhouzhihao & min0911\n");
    print("THANKS Link TOOLS BY Kawai\n\n");
    printf("C Build tools by GNU C Compiler\n");
    printf("ASM Build tools by NASM\n\n");
    printf("I love you Kawai\n");
    return;
  } else if (strcmp("TIME", cmdline) == 0) {
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
  } else if (strcmp("DATE", cmdline) == 0) {
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
  } else if (strcmp("PCILS", cmdline) == 0) {
    pci_list();
  } else if (strcmp("PRASHELL", cmdline) == 0) {
    PraShell();
  } else if (strncmp("ECHO ", cmdline, 5) == 0) {
    print(cmdline + 5);
    print("\n");
    return;
  } else if (strncmp("MKDIR ", cmdline, 6) == 0) {
    if (NowTask()->change_dict_times == 0) {
      mkdir(cmdline + 6, 0);
    } else {
      struct FILEINFO* finfo =
          dict_search(".", NowTask()->directory,
                      drive_ctl.drives[NowTask()->drive_number].RootMaxFiles);
      mkdir(cmdline + 6, finfo->clustno);
    }
  } else if (strncmp("POKE ", cmdline, 5) == 0) {
    addr =
        ascii2num(cmdline[5]) * 0x10000000 + ascii2num(cmdline[6]) * 0x1000000;
    addr = addr + ascii2num(cmdline[7]) * 0x100000 +
           ascii2num(cmdline[8]) * 0x10000;
    addr =
        addr + ascii2num(cmdline[9]) * 0x1000 + ascii2num(cmdline[10]) * 0x100;
    addr = addr + ascii2num(cmdline[11]) * 0x10 + ascii2num(cmdline[12]);
    p = addr;
    c = ascii2num(cmdline[14]) * 0x10 + ascii2num(cmdline[15]);
    p[0] = c;
    print("\n");
    return;
  } else if (strncmp("VISIT ", cmdline, 6) == 0) {
    addr =
        ascii2num(cmdline[6]) * 0x10000000 + ascii2num(cmdline[7]) * 0x1000000;
    addr = addr + ascii2num(cmdline[8]) * 0x100000 +
           ascii2num(cmdline[9]) * 0x10000;
    addr =
        addr + ascii2num(cmdline[10]) * 0x1000 + ascii2num(cmdline[11]) * 0x100;
    addr = addr + ascii2num(cmdline[12]) * 0x10 + ascii2num(cmdline[13]);
    p = addr;
    c = p[0];
    printchar(num2ascii(c >> 4));
    printchar(num2ascii(c & 0x0f));
    print("\n");
    return;
  } else if (strcmp("PCINFO", cmdline) == 0) {
    pcinfo();
  } else if (strcmp("MEM", cmdline) == 0) {
    mem();
  } else if (strncmp("BMPVIEW ", cmdline, 8) == 0 &&
             running_mode == POWERINTDOS) {
    bmpview(cmdline + 8);
    char c;
    for (;;) {
      c = input_char_inSM();
      if (c == 0x01) {
        SwitchToText8025_BIOS();
        Set_Font(font);
        break;
      }
    }
    return;
  } else if (strncmp("CHVIEW ", cmdline, 7) == 0) {
    chview(cmdline + 7);
  } else if (strncmp("BEEP ", cmdline, 5) == 0) {
    int point, notes, dup;
    point = ascii2num(*(char*)(cmdline + 5));
    notes = ascii2num(*(char*)(cmdline + 7));
    dup = ascii2num(*(char*)(cmdline + 9));
    beep(point, notes, dup);
  } else if (strcmp("MOUSE", cmdline) == 0 && running_mode == POWERINTDOS) {
    struct FIFO8* kfifo;
    struct FIFO8* mfifo;
    char* kbuf;
    char* mbuf;
    unsigned char* stack;
    if (flag == 0) {
      struct TASK* mouse_task;
      stack = page_malloc(64 * 1024);
      mouse_task = AddTask("Mouse", 2, 2 * 8, (int)mouseinput, 1 * 8, 1 * 8,
                           stack + 64 * 1024);
      kfifo = (struct FIFO8*)page_malloc(sizeof(struct FIFO8));
      mfifo = (struct FIFO8*)page_malloc(sizeof(struct FIFO8));
      kbuf = (char*)page_malloc(32);
      mbuf = (char*)page_malloc(128);
      fifo8_init(kfifo, 32, kbuf);
      fifo8_init(mfifo, 128, mbuf);
      TaskSetFIFO(mouse_task, kfifo, mfifo);
      flag = 1;
    } else {
      page_free(stack, 64 * 1024);
      page_free(kfifo, sizeof(struct FIFO8));
      page_free(mfifo, sizeof(struct FIFO8));
      page_free(kbuf, 32);
      page_free(mbuf, 128);
      SubTask(GetTaskForName("Mouse"));
      flag = 0;
    }
  } else if (strcmp("REBOOT", cmdline) == 0) {
    io_out8(0xcf9, 0x0e);
  } else if (strcmp("HALT", cmdline) == 0) {
    running_mode = POWERINTDOS;
    acpi_shutdown();
    SwitchToText8025_BIOS();
    clear();
    printf("\nYou can turn off your computer safely.");
    io_cli();
    while (1)
      ;
  } else if (strncmp("PAK ", cmdline, 4) == 0) {
    int i;
    for (i = 4; cmdline[i] != ' '; i++)
      ;
    pak(cmdline + 4, cmdline + i + 1);
  } else if (strncmp("UNPAK ", cmdline, 6) == 0) {
    unpak(cmdline + 6);
  } else if (strncmp("ZIP ", cmdline, 4) == 0) {
    if (Get_Argc(cmdline) < 2) {
      printf("Usage: ZIP <infile> <outfile>\n\n");
      return;
    }
    char* asm1 = page_malloc(100);
    char* out = page_malloc(100);
    Get_Arg(asm1, cmdline, 1);
    Get_Arg(out, cmdline, 2);
    compress_one_file(asm1, out);
  } else if (strncmp("UZIP ", cmdline, 5) == 0) {
    if (Get_Argc(cmdline) < 2) {
      printf("Usage: UZIP <infile> <outfile>\n\n");
      return;
    }
    char* asm1 = page_malloc(100);
    char* out = page_malloc(100);
    Get_Arg(asm1, cmdline, 1);
    Get_Arg(out, cmdline, 2);
    decompress_one_file(asm1, out);
  } else if (strncmp("COLOR ", cmdline, 6) == 0) {
    cons_color = (ascii2num(cmdline[6]) << 4) + ascii2num(cmdline[7]);
    int i;
    for (i = 0; i != 160 * 25; i += 2) {
      *(char*)(0xb8000 + i + 1) = cons_color;
    }
  } else if (strncmp("CASM ", cmdline, 5) == 0) {
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
    for (int i = 0; i != fp->size; i++) {
      printf("%02x ", (unsigned char)fp->buf[i]);
    }
    printf("\n");
    fclose(fp);
    page_free(asm1, 100);
    page_free(out, 100);
  } else if (strncmp("MKFILE ", cmdline, 7) == 0) {
    mkfile(cmdline + 7);
    return;
  } else if (strncmp("DEL ", cmdline, 4) == 0) {
    del(cmdline);
    return;
  }
  // else if (strncmp("EDIT ", cmdline, 5) == 0)
  // {
  // 	edit(cmdline);
  // 	return;
  // }
  else if (strncmp("FONT ", cmdline, 5) == 0) {
    Set_Font(cmdline + 5);
  } else if (strncmp("CD ", cmdline, 3) == 0) {
    changedict(cmdline + 3);
  } else if (strcmp("LINEAR FUNCTION", cmdline) == 0) {
    int k, b;
    printf("y = kx + b\n");
    printf("k = ");
    char* in = (char*)malloc(100);
    input(in, 100);
    k = (int)strtol(in, NULL, 10);
    printf("b = ");
    input(in, 100);
    b = (int)strtol(in, NULL, 10);
    printf("y = %dx + %d\n", k, b);
    printf("Press any key to continue...");
    getch();
    free((void*)in);
    int ox = 1024 / 2, oy = 768 / 2;
    if (set_mode(1024, 768, 32) != 0) {
      printf("Can't enable 1024x768x32 VBE mode.\n\n");
      return;
    }
    struct VBEINFO* vbinfo = (struct VBEINFO*)VBEINFO_ADDRESS;
    double i = 1.0 / (double)(abs(k));
    for (double x = -(1024 / 2); x < 1024 / 2; x += i) {
      double y = k * x + b;
      SDraw_Px(vbinfo->vram, (int)(ox + x), 768 - (int)(oy + y), COL_FFFFFF,
               1024);
    }
    for (;;)
      ;
  } else if (strcmp("QUADRATIC FUNCTION", cmdline) == 0) {
    int a, b, c;
    printf("y = ax^2 + bx + c\n");
    printf("a = ");
    char* in = (char*)malloc(100);
    input(in, 100);
    a = (int)strtol(in, NULL, 10);
    printf("b = ");
    input(in, 100);
    b = (int)strtol(in, NULL, 10);
    printf("c = ");
    input(in, 100);
    c = (int)strtol(in, NULL, 10);
    printf("y = %dx^2 + %dx + %d\n", a, b, c);
    printf("Press any key to continue...");
    getch();
    free((void*)in);
    int ox = 1024 / 2, oy = 768 / 2;
    if (set_mode(1024, 768, 32) != 0) {
      printf("Can't enable 1024x768x32 VBE mode.\n\n");
      return;
    }
    struct VBEINFO* vbinfo = (struct VBEINFO*)VBEINFO_ADDRESS;
    double i = 1.0 / (double)(abs(a * a * b));
    for (double x = -(1024 / 2); x < 1024 / 2; x += i) {
      double y = a * x * x + b * x + c;
      if ((int)(oy + y) > 768 || (int)(oy + y) < 0) {
        continue;
      }
      SDraw_Px(vbinfo->vram, (int)(ox + x), 768 - (int)(oy + y), COL_FFFFFF,
               1024);
    }
    for (;;)
      ;
  } else if (strncmp("RENAME ", cmdline, 7) == 0) {
    if (Get_Argc(cmdline) < 2) {
      printf("Usage: RENAME <src_name> <dst_name>\n");
      return;
    }
    char* src_name = (char*)malloc(100);
    char* dst_name = (char*)malloc(100);
    Get_Arg(src_name, cmdline, 1);
    Get_Arg(dst_name, cmdline, 2);
    rename(src_name, dst_name);
    free(src_name);
    free(dst_name);
  } else if (cmdline[1] == ':') {
    int* eipa = &cmdline;
    printk("====%08x\n", eipa[-1]);
    struct TASK* task = NowTask();
    int drive_number = *cmdline - 0x41;
    if (1) {
      if (drive_ctl.drives[drive_number].ADR_DISKIMG != NULL) {
        free(drive_ctl.drives[drive_number].ADR_DISKIMG);
        free(drive_ctl.drives[drive_number].fat);
        free(drive_ctl.drives[drive_number].FatClustnoFlags);
        free(drive_ctl.drives[drive_number].root_directory);
      }
      void* boot_sector = page_malloc(512);
      Disk_Read(0, 1, boot_sector, *cmdline);

      if (!*(unsigned char*)(boot_sector)) {
        printf("Device not ready.\n");
        return;
      }
      task->drive_number = drive_number;
      task->drive = *cmdline;
      task->directory = drive_ctl.drives[task->drive_number].root_directory;
      strcpy(task->path, "");
      task->change_dict_times = 0;
      drive_ctl.drives[task->drive_number].SectorBytes =
          *(unsigned short*)(boot_sector + BPB_BytsPerSec);
      drive_ctl.drives[task->drive_number].RootMaxFiles =
          *(unsigned short*)(boot_sector + BPB_RootEntCnt);
      drive_ctl.drives[task->drive_number].ClustnoBytes =
          drive_ctl.drives[task->drive_number].SectorBytes *
          *(unsigned char*)(boot_sector + BPB_SecPerClus);
      drive_ctl.drives[task->drive_number].RootDictAddress =
          (*(unsigned char*)(boot_sector + BPB_NumFATs) *
               *(unsigned short*)(boot_sector + BPB_FATSz16) +
           *(unsigned short*)(boot_sector + BPB_RsvdSecCnt)) *
          drive_ctl.drives[task->drive_number].SectorBytes;
      drive_ctl.drives[task->drive_number].FileDataAddress =
          drive_ctl.drives[task->drive_number].RootDictAddress +
          drive_ctl.drives[task->drive_number].RootMaxFiles * 32;
      if (*(unsigned short*)(boot_sector + BPB_TotSec16) != 0) {
        drive_ctl.drives[task->drive_number].imgTotalSize =
            *(unsigned short*)(boot_sector + BPB_TotSec16) *
            drive_ctl.drives[task->drive_number].SectorBytes;
      } else {
        drive_ctl.drives[task->drive_number].imgTotalSize =
            *(unsigned int*)(boot_sector + BPB_TotSec32) *
            drive_ctl.drives[task->drive_number].SectorBytes;
      }
      drive_ctl.drives[task->drive_number].Fat1Address =
          *(unsigned short*)(boot_sector + BPB_RsvdSecCnt) *
          drive_ctl.drives[task->drive_number].SectorBytes;
      drive_ctl.drives[task->drive_number].Fat2Address =
          drive_ctl.drives[task->drive_number].Fat1Address +
          *(unsigned short*)(boot_sector + BPB_FATSz16) *
              drive_ctl.drives[task->drive_number].SectorBytes;
      uint32_t sec = drive_ctl.drives[task->drive_number].FileDataAddress /
                     drive_ctl.drives[task->drive_number].SectorBytes;
      drive_ctl.drives[task->drive_number].ADR_DISKIMG =
          malloc(drive_ctl.drives[task->drive_number].FileDataAddress);

      Disk_Read(0, sec, drive_ctl.drives[task->drive_number].ADR_DISKIMG,
                task->drive);

      drive_ctl.drives[task->drive_number].fat = malloc(3072 * sizeof(int));
      drive_ctl.drives[task->drive_number].FatClustnoFlags =
          malloc(3072 * sizeof(char));
      read_fat(drive_ctl.drives[task->drive_number].ADR_DISKIMG +
                   drive_ctl.drives[task->drive_number].Fat1Address,
               drive_ctl.drives[task->drive_number].fat,
               drive_ctl.drives[task->drive_number].FatClustnoFlags);
      drive_ctl.drives[task->drive_number].root_directory =
          (struct FILEINFO*)malloc(
              drive_ctl.drives[task->drive_number].RootMaxFiles * 32);
      memcpy((void*)drive_ctl.drives[task->drive_number].root_directory,
             (void*)drive_ctl.drives[task->drive_number].ADR_DISKIMG +
                 drive_ctl.drives[task->drive_number].RootDictAddress,
             drive_ctl.drives[task->drive_number].RootMaxFiles * 32);
      task->directory = drive_ctl.drives[task->drive_number].root_directory;
      drive_ctl.drives[task->drive_number].directory_list = NewList();
      drive_ctl.drives[task->drive_number].directory_clustno_list = NewList();
      struct FILEINFO* finfo = task->directory;

      for (int i = 0; i != drive_ctl.drives[task->drive_number].RootMaxFiles;
           i++) {
        if (finfo[i].type == 0x10 && finfo[i].name[0] != 0xe5) {
          AddVal(finfo[i].clustno,
                 drive_ctl.drives[task->drive_number].directory_clustno_list);
          void* directory_alloc =
              malloc(drive_ctl.drives[task->drive_number].ClustnoBytes);
          uint32_t sec1 =
              (drive_ctl.drives[task->drive_number].FileDataAddress +
               (finfo[i].clustno - 2) *
                   drive_ctl.drives[task->drive_number].ClustnoBytes) /
              drive_ctl.drives[task->drive_number].SectorBytes;
          Disk_Read(sec1,
                    drive_ctl.drives[task->drive_number].ClustnoBytes /
                        drive_ctl.drives[task->drive_number].SectorBytes,
                    directory_alloc, task->drive);
          AddVal(directory_alloc,
                 drive_ctl.drives[task->drive_number].directory_list);
        }
        if (finfo[i].name[0] == NULL) {
          break;
        }
      }

      for (int i = 1;
           FindForCount(
               i, drive_ctl.drives[task->drive_number].directory_list) != NULL;
           i++) {
        struct List* list = FindForCount(
            i, drive_ctl.drives[task->drive_number].directory_list);
        finfo = (struct FILEINFO*)list->val;
        for (int j = 0;
             j != drive_ctl.drives[task->drive_number].ClustnoBytes / 32; j++) {
          if (finfo[j].type == 0x10 && finfo[j].name[0] != 0xe5 &&
              strncmp(".", finfo[j].name, 1) != 0 &&
              strncmp("..", finfo[j].name, 2) != 0) {
            AddVal(finfo[j].clustno,
                   drive_ctl.drives[task->drive_number].directory_clustno_list);
            void* directory_alloc =
                page_malloc(drive_ctl.drives[task->drive_number].ClustnoBytes);
            uint32_t sec1 =
                (drive_ctl.drives[task->drive_number].FileDataAddress +
                 (finfo[j].clustno - 2) *
                     drive_ctl.drives[task->drive_number].ClustnoBytes) /
                drive_ctl.drives[task->drive_number].SectorBytes;
            Disk_Read(sec1,
                      drive_ctl.drives[task->drive_number].ClustnoBytes /
                          drive_ctl.drives[task->drive_number].SectorBytes,
                      directory_alloc, task->drive);
            AddVal(directory_alloc,
                   drive_ctl.drives[task->drive_number].directory_list);
          }
          if (finfo[j].name[0] == NULL) {
            break;
          }
        }
      }

      page_free(boot_sector, 512);
    } else {
      task->drive_number = drive_number;
      task->drive = *cmdline;
      task->directory = drive_ctl.drives[task->drive_number].root_directory;
      strcpy(task->path, "");
      task->change_dict_times = 0;
    }
    printk("=%08x\n", eipa[-1]);

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
  unsigned char* pci_drive = PCI_ADDR_BASE;
  //输出PCI表的内容
  for (int line = 0;; pci_drive += 0x110 + 4, line++) {
    if (pci_drive[0] == 0xff)
      PCI_ClassCode_Print((struct PCI_CONFIG_SPACE_PUCLIC*)(pci_drive + 12));
    else
      break;
  }
}

void cmd_dir() {
  // DIR命令的实现
  struct TASK* task = NowTask();
  struct FILEINFO* finfo = task->directory;
  extern int MaxLine;
  int i, j, k, line = 0;
  int FileSize = 0;  //所有文件的大小
  char s[30];
  for (i = 0; i != 30; i++) {
    s[i] = 0;
  }
  print("FILENAME   EXT    LENGTH       TYPE   DATE\n");
  for (i = 0; i < drive_ctl.drives[task->drive_number].RootMaxFiles;
       i++, line++) {
    if (line == MaxLine) {
      print("Press any key to continue...");
      getch();
      print("\n");
      line = 0;
    }
    if (finfo[i].name[0] == 0x00) {
      break;
    }
    if (finfo[i].name[0] != 0xe5) {
      if ((finfo[i].type & 0x18) == 0 || finfo[i].type == 0x10) {
        for (j = 0; j < 8; j++) {
          s[j] = finfo[i].name[j];
        }
        s[9] = finfo[i].ext[0];
        s[10] = finfo[i].ext[1];
        s[11] = finfo[i].ext[2];

        if (s[0] != '+') {
          for (k = 0; k < 12; ++k) {
            if (k == 9) {
              print("   ");
            }
            if (s[k] == '\n') {
              print("   ");
            } else {
              printchar(s[k]);
            }
          }
          print("    ");
          print(itoa(finfo[i].size));
          gotoxy(31, get_y());
          if ((finfo[i].type & 0x18) == 0) {
            print("<FILE> ");
          }
          if (finfo[i].type == 0x10) {
            print("<DIR>  ");
          }
          print_date(finfo[i].date, finfo[i].time);
          print("\n");
          FileSize += finfo[i].size;
        }
      }
    }
  }
  print("\n");
  printf("    %d byte(s)\n", FileSize);
  printf("    %d byte(s) free\n",
         drive_ctl.drives[task->drive_number].imgTotalSize - FileSize);
  //&s = 0;
  return;
}
void type_deal(char* cmdline) {
  // type命令的实现
  char* name;
  int i;
  struct FILEINFO* finfo;
  for (i = 0; i < strlen(cmdline); i++) {
    name[i] = cmdline[i + 5];
  }
  finfo = Get_File_Address(name);
  if (finfo == 0) {
    print(name);
    print(" not found!\n\n");
  } else {
    FILE* fp = fopen(name, "r");
    char* p = fp->buf;
    for (i = 0; i != finfo->size; i++) {
      printchar(p[i]);
    }
    print("\n");
    fclose(fp);
  }
  return;
}
static vram_t* buf_text_window;
static struct SHEET* sht_text_window;
static void close_text_window() {
  sheet_free(sht_text_window);
  page_free((int)buf_text_window, 800 * 600 * sizeof(color_t));
}
void chview(char* filename) {
  struct FILEINFO* finfo = Get_File_Address(filename);
  FILE* fp = fopen(filename, "r");
  char* p = fp->buf;
  if (finfo == 0) {
    print("Can't find file ");
    print(filename);
    print("\n");
    return;
  }
  if (running_mode == POWERINTDOS) {
    SwitchToHighTextMode();  //切换到高分辨率文本模式
    HighPutStr(p, finfo->size);
    getch();
    Close_High_Text_Mode();
    SwitchToText8025_BIOS();
    Set_Font(font);
  } else if (running_mode == POWERDESKTOP) {
    extern struct SHTCTL* shtctl;
    buf_text_window = (vram_t*)page_malloc(800 * 600 * sizeof(color_t));
    sht_text_window = MakeWindow(50, 50, 800, 600, "chview", shtctl,
                                 buf_text_window, (int)close_text_window);
    sheet_updown(sht_text_window, shtctl->top - 1);
  }
  fclose(fp);
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

void set_cons_color(char color) {
  cons_color = color;
}

char get_cons_color() {
  //获取颜色
  return cons_color;
}
int mx = 39, my = 12;
void mouseinput() {
  char mousecopy[80];
  int i, mx1 = mx, my1 = my, bufx = mx * 8, bufy = my * 16;
  mouse_ready(&mdec);
  while (1) {
    if (fifo8_status(TaskGetMousefifo(NowTask())) == 0) {
      io_stihlt();
    } else {
      *(char*)(0xb8000 + my1 * 160 + mx1 * 2 + 1) = 0x07;
      *(char*)(0xb8000 + my * 160 + mx * 2 + 1) = 0x70;
      i = fifo8_get(TaskGetMousefifo(NowTask()));
      // io_sti();
      if (mouse_decode(&mdec, i) != 0) {
        mx1 = mx;  // 更新mx1,my1
        my1 = my;
        bufx += mdec.x;
        bufy += mdec.y;
        mx = bufx / 8;   // 计算位置
        my = bufy / 16;  // 计算位置
        if (bufx > 79 * 8) {
          bufx = 79 * 8;
        } else if (bufx < 0) {
          bufx = 0;
        }
        if (bufy > 24 * 16) {
          bufy = 24 * 16;
        } else if (bufy < 0) {
          bufy = 0;
        }
      }
    }
  }
  mouse_sleep(&mdec);  //让鼠标暂时停止不向FIFO缓冲区写入数据
  mx = mx1;
  my = my1;
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
  mkfile(outfilename);
  FILE* infile = fopen(infilename, "rb");
  buffer = malloc(infile->size);
  sz = infile->size;
  end = buffer + infile->size;
  fread(buffer, infile->size, 1, infile);
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
    // fwrite(buffer, 1, num_read, outfile);
    printk("p=%d,num_read=%d\n", p, num_read);
    for (int i = 0; i < num_read; i++) {
      buffer2[p++] = buffer[i];
      printk("buffer2[%d]=%c\n", p - 1, buffer[i]);
      printk("Now buffer2[%d] = %c\n", p - 1, buffer2[p - 1]);
    }

    // memset(buffer, 0, 128);
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
  EDIT_FILE(outfilename, buffer2, sz, 0);
  printk("size=%d\n", sz);
  // printk("%s\n", buffer2);
  page_kfree(buffer2, sz);
  return 0;
}

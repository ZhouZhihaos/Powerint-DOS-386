// 命令行与命令处理
#include <cmd.h>
#include <dos.h>
unsigned int dictaddr = 0x00002600; // 目前目录的地址
unsigned char cons_color = 0x0f;    // 颜色
unsigned int change_dict_times = 0; // 目录改变次数
static void send_task() {
  SendIPCMessage(2, "Hello Task_sr2! I'm Send.",
                 strlen("Hello Task_sr2! I'm Send."),
                 synchronous); // 给Task_sr2发信息（同步）
  while (1)
    ;
}
static vram_t *buf_view_window;
static struct SHEET *sht_view_window;
static int sheet_view_free_flag;
static void close_view_window() {
  sheet_free(sht_view_window);
  page_free((int)buf_view_window, 1029 * 792 * sizeof(color_t));
  sheet_view_free_flag = 0;
}
static void TCP_Socket_Handler(struct Socket *socket, void *base) {
  struct TCPMessage *tcp =
      (struct TCPMessage *)(base + sizeof(struct EthernetFrame_head) +
                            sizeof(struct IPV4Message));
  uint8_t *data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + (tcp->headerLength * 4);
  printf("\nTCP Recv from %d.%d.%d.%d:%d:%s\n",
         (uint8_t)(socket->remoteIP >> 24), (uint8_t)(socket->remoteIP >> 16),
         (uint8_t)(socket->remoteIP >> 8), (uint8_t)(socket->remoteIP),
         socket->remotePort, data);
}
static void UDP_Socket_Handler(struct Socket *socket, void *base) {
  uint8_t *data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
  printf("\nUDP Recv from %d.%d.%d.%d:%d:%s\n",
         (uint8_t)(socket->remoteIP >> 24), (uint8_t)(socket->remoteIP >> 16),
         (uint8_t)(socket->remoteIP >> 8), (uint8_t)(socket->remoteIP),
         socket->remotePort, data);
}
static unsigned char *html_file;
static void HTTP_Socket_Handler(struct Socket *socket, void *base) {
  struct IPV4Message *ipv4 =
      (struct IPV4Message *)(base + sizeof(struct EthernetFrame_head));
  struct TCPMessage *tcp =
      (struct TCPMessage *)(base + sizeof(struct EthernetFrame_head) +
                            sizeof(struct IPV4Message));
  uint16_t size = swap16(ipv4->totalLength) - sizeof(struct IPV4Message) -
                  (tcp->headerLength * 4);
  uint8_t *data = base + sizeof(struct EthernetFrame_head) +
                  sizeof(struct IPV4Message) + (tcp->headerLength * 4);
  if (IsHttpGetHeader(data, size)) {
    unsigned char *head = "HTTP/1.0 200 OK\r\n\r\n";
    unsigned char *packet =
        (unsigned char *)page_malloc(strlen(head) + strlen(html_file) + 1);
    memcpy((void *)packet, (void *)head, strlen(head));
    memcpy((void *)(packet + strlen(head)), (void *)html_file,
           strlen(html_file));
    packet[strlen(head) + strlen(html_file) + 1] = NULL;
    socket->Send(socket, packet, strlen(packet) + 1);
  }
}
static void SocketServerLoop(struct SocketServer *server) {
  static bool flags[SOCKET_SERVER_MAX_CONNECT];
  memset((void *)flags, false, SOCKET_SERVER_MAX_CONNECT * sizeof(bool));
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

void command_run(char *cmdline) {
  //命令解析器
  uint32_t addr;
  uint8_t c;
  char *p;
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

  strtoupper(cmdline); //转换成大写
  if (strcmp("WIN", cmdline) == 0) {
    //进入图形化环境
    if (running_mode == POWERDESKTOP)
      return;
    running_mode = POWERDESKTOP;
    graphic();
    running_mode = POWERINTDOS;
    return;
  } else if (strncmp("DESKTOP ", cmdline, 8) == 0) {
    if (!fopen(cmdline + 8)) {
      return;
    }
    extern struct SHEET *sht_back;
    show_photo(cmdline + 8, sht_back->buf, sht_back->bxsize);
    sheet_refresh(sht_back, 0, 0, sht_back->bxsize, sht_back->bysize);
  } else if (strcmp("PCNET", cmdline) == 0) {
    init_pcnet_card();
  } else if (strcmp("CHAT", cmdline) == 0 && running_mode == POWERINTDOS) {
    chat_cmd();
  } else if (strcmp("CHAT", cmdline) == 0 && running_mode == POWERDESKTOP) {
    chat_gui();
  } else if (strncmp("HTTP ", cmdline, 5) == 0) {
    html_file = fopen(cmdline + 5);
    extern uint32_t ip;
    srand(time());
    uint16_t port = (uint16_t)rand();
    struct SocketServer *server =
        SocketServer_Alloc(HTTP_Socket_Handler, ip, port, TCP_PROTOCOL);
    printf("SrcIP/Port:%d.%d.%d.%d:%d\n", (uint8_t)(ip >> 24),
           (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip), port);
    SocketServerLoop(server);
  } else if (strcmp("SOCKET", cmdline) == 0) {
    extern uint32_t ip;
    struct Socket *socket;
    struct SocketServer *server;
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
      if (p) { // TCP
        socket = Socket_Alloc(TCP_PROTOCOL);
        Socket_Bind(socket, TCP_Socket_Handler);
      } else if (!p) { // UDP
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
    char *inp = (char *)page_malloc(1024);
    while (1) {
      if (socket->state == SOCKET_TCP_CLOSED && !m) {
        if (p) {
          socket->Disconnect(socket);
        }
        Socket_Free(socket);
        page_free((void *)inp, 1024);
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
        page_free((void *)inp, 1024);
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
  } else if (strncmp("NSLOOKUP ", cmdline, 9) == 0) {
    uint8_t *dns = (uint8_t *)page_malloc(strlen(cmdline + 9) + 1);
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
  } else if (strcmp("FORK", cmdline) == 0) {
    if (fork() == 0) {
      printf("I'm child process! --- %d\n", Get_Tid(NowTask()));
      SubTask(NowTask());
    } else {
      printf("I'm parent process! --- %d\n", Get_Tid(NowTask()));
    }
  } else if (strcmp("SB16", cmdline) == 0) {
    wav_player_test();
  } else if (strncmp("SWITCH ", cmdline, 7) == 0 &&
             running_mode == POWERDESKTOP) {
    io_cli(); // 禁止中断产生
    extern struct VBEINFO *vbinfo;
    SDraw_Box(vbinfo->vram, 0, 0, vbinfo->xsize, vbinfo->ysize, COL_000000,
              vbinfo->xsize);
    int xsize = strtol(cmdline + 7, ' ', 10);
    int t = xsize, length = 1;
    while (t >= 10) {
      t /= 10;
      length++;
    }
    int ysize = strtol(cmdline + 7 + length, NULL, 10);
    if (set_mode(xsize, ysize, 32) != 0) { // 彳亍不彳亍
      printf("Can't enable %dx%dx32 VBE mode.\n\n", xsize, ysize);
      return;
    }
    io_cli();
    struct VBEINFO *vbinfo0 = (struct VBEINFO *)VBEINFO_ADDRESS;
    extern struct SHTCTL *shtctl;
    extern struct SHEET *sht_back;
    extern vram_t *buf_back;
    extern struct SHEET *sht_mouse;
    // 不定 -> 定值
    vbinfo->vram = vbinfo0->vram;
    vbinfo->xsize = vbinfo0->xsize;
    vbinfo->ysize = vbinfo0->ysize;
    // shtctl更改
    shtctl->vram = (vram_t *)vbinfo->vram;
    shtctl->xsize = vbinfo->xsize;
    shtctl->ysize = vbinfo->ysize;
    // sht_back（背景）更改 重新malloc缓冲区+初始化并刷新
    page_free((void *)buf_back,
              sht_back->bxsize * sht_back->bysize * sizeof(color_t));
    buf_back =
        (vram_t *)page_malloc(vbinfo->xsize * vbinfo->ysize * sizeof(color_t));
    sht_back->buf = buf_back;
    sht_back->bxsize = vbinfo->xsize;
    sht_back->bysize = vbinfo->ysize;
    init_screen(buf_back, vbinfo->xsize, vbinfo->ysize);
    sheet_refresh(sht_back, 0, 0, vbinfo->xsize, vbinfo->ysize);
    for (int i = 1; shtctl->sheets0[i].flags != 0;
         i++) { // 把每一个窗口的坐标设置成(0,0) 并且刷新（除sht_back）
      sheet_slide((struct SHEET *)&shtctl->sheets0[i], 0, 0);
      sheet_refresh((struct SHEET *)&shtctl->sheets0[i], 0, 0,
                    shtctl->sheets0[i].bxsize, shtctl->sheets0[i].bysize);
    }
    sheet_slide(sht_mouse, vbinfo->xsize / 2 - 8,
                vbinfo->ysize / 2 - 8); // 鼠标重新居中
    io_sti();
  } else if (strcmp("VBETEST", cmdline) == 0) {
    //对VBE测试
    get_all_mode();
    cmd_vbetest();
  } else if (strncmp("BMPVIEW32 ", cmdline, 10) == 0) {
    if (running_mode == POWERINTDOS) {
      struct VBEINFO *vbeinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
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
      if (fopen(cmdline + 10) == 0) {
        printf("Can't find file %s!\n\n", cmdline + 10);
        return;
      }
      sheet_view_free_flag = 1;
      extern struct SHTCTL *shtctl;
      buf_view_window = (vram_t *)page_malloc(1029 * 792 * sizeof(color_t));
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
      struct VBEINFO *vbeinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
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
      if (fopen(cmdline + 8) == 0) {
        printf("Can't find file %s!\n\n", cmdline + 8);
        return;
      }
      sheet_view_free_flag = 1;
      extern struct SHTCTL *shtctl;
      buf_view_window = (vram_t *)page_malloc(1029 * 792 * sizeof(color_t));
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
    struct VBEINFO *vbeinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
    if (set_mode(1024, 768, 32) != 0) {
      printf("Can't enable 1024x768x32 VBE mode.\n\n");
      return;
    }
    pra_view_32(cmdline + 8, vbeinfo->vram, 1024);
    getch();
    SwitchToText8025_BIOS();
    Set_Font(font);
    clear();
    return;
  } else if (strcmp("DIR", cmdline) == 0) {
    cmd_dir();
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
        SubTask(GetTask(i));
        return;
      }
    }
    printf("No such task.\n");
    return;
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
    char *time = "The current time is:00:00:00";
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
    char *date = "The current date is:2000\\00\\00,";
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
    if (change_dict_times == 0) {
      mkdir(cmdline + 6, 0);
    } else {
      struct FILEINFO *finfo =
          dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
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
    point = ascii2num(*(char *)(cmdline + 5));
    notes = ascii2num(*(char *)(cmdline + 7));
    dup = ascii2num(*(char *)(cmdline + 9));
    beep(point, notes, dup);
  } else if (strcmp("MOUSE", cmdline) == 0 && running_mode == POWERINTDOS) {
    struct FIFO8 *kfifo;
    struct FIFO8 *mfifo;
    char *kbuf;
    char *mbuf;
    unsigned char *stack;
    if (flag == 0) {
      struct TASK *mouse_task;
      stack = page_malloc(64 * 1024);
      mouse_task = AddTask("Mouse", 2, 2 * 8, (int)mouseinput, 1 * 8, 1 * 8,
                           stack + 64 * 1024);
      kfifo = (struct FIFO8 *)page_malloc(sizeof(struct FIFO8));
      mfifo = (struct FIFO8 *)page_malloc(sizeof(struct FIFO8));
      kbuf = (char *)page_malloc(32);
      mbuf = (char *)page_malloc(128);
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
    save_all_floppy();
    io_out8(0xcf9, 0x0e);
  } else if (strcmp("HALT", cmdline) == 0) {
    // 先保存，再调用ACPI进行关机
    save_all_floppy();
    acpi_shutdown();
  } else if (strncmp("PAK ", cmdline, 4) == 0) {
    int i;
    for (i = 4; cmdline[i] != ' '; i++)
      ;
    pak(cmdline + 4, cmdline + i + 1);
  } else if (strncmp("UNPAK ", cmdline, 6) == 0) {
    unpak(cmdline + 6);
  } else if (strncmp("COLOR ", cmdline, 6) == 0) {
    cons_color = (ascii2num(cmdline[6]) << 4) + ascii2num(cmdline[7]);
    int i;
    for (i = 0; i != 160 * 25; i += 2) {
      *(char *)(0xb8000 + i + 1) = cons_color;
    }
  } else if (strncmp("CASM ", cmdline, 5) == 0) {
    if (Get_Argc(cmdline) < 2) {
      printf("Usage: CASM <asmfile> <outfile>\n\n");
      return;
    }
    char *asm1 = page_malloc(100);
    char *out = page_malloc(100);
    Get_Arg(asm1, cmdline, 1);
    Get_Arg(out, cmdline, 2);
    if (fopen(asm1) == 0) {
      printf("%s not find!\n\n", asm1);
      return;
    }
    if (fopen(out) == 0)
      mkfile(out);
    FILE *fp_asm = fopen_for_FILE(asm1, "wb");
    FILE *fp_out = fopen_for_FILE(out, "wb");
    compile_file(fp_asm, fp_out);
    fclose(fp_asm);
    fclose(fp_out);
    struct FILEINFO *finfo = Get_File_Address(out);
    char *p = fopen(out);
    for (int i = 0; i != finfo->size; i++)
      printf("%02x ", (unsigned char)p[i]);
    printf("\n");
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
  } else if (*(char *)(0x7dfd) == 'A') {
    if (strcmp("SETUP", cmdline) == 0) {
      setup();
      return;
    } else if (cmd_app(cmdline) == 0) {
      if (run_bat(cmdline) == 0) {
        print("Bad Command!\n\n");
        return;
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
  unsigned char *pci_drive = PCI_ADDR_BASE;
  //输出PCI表的内容
  for (int line = 0;; pci_drive += 0x110 + 4, line++) {
    if (pci_drive[0] == 0xff)
      PCI_ClassCode_Print((struct PCI_CONFIG_SPACE_PUCLIC *)(pci_drive + 12));
    else
      break;
  }
}

void cmd_dir() {
  // DIR命令的实现
  struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + dictaddr);
  extern int MaxLine;
  int i, j, k, line = 0;
  int FileSize = 0; //所有文件的大小
  char s[30];
  for (i = 0; i != 30; i++) {
    s[i] = 0;
  }
  print("FILENAME   EXT    LENGTH       TYPE   DATE\n");
  for (i = 0; i < 224; i++, line++) {
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
  printf("    %d byte(s) free\n", 2880 * 512 - FileSize);
  //&s = 0;
  return;
}
void type_deal(char *cmdline) {
  // type命令的实现
  char *name;
  int i;
  struct FILEINFO *finfo;
  for (i = 0; i < strlen(cmdline); i++) {
    name[i] = cmdline[i + 5];
  }
  finfo = Get_File_Address(name);
  if (finfo == 0) {
    print(name);
    print(" not found!\n\n");
  } else {
    char *p = fopen(name);
    for (i = 0; i != finfo->size; i++) {
      printchar(p[i]);
    }
    print("\n");
  }
  return;
}

void edit(char *cmdline) {
  char *name;
  int i;
  struct FILEINFO *finfo;
  for (i = 0; i < strlen(cmdline); i++) {
    name[i] = cmdline[i + 5];
  }
  finfo = Get_File_Address(name);
  if (finfo == 0) {
    print(name);
    print(" not found!\n\n");
    return;
  }
  finfo->size = 0;
  char *p = fopen(name);
  for (;; finfo->size++) {
    p[finfo->size] = getch();
    printchar(p[finfo->size]);
    if (p[finfo->size] == 0x0a) {
      break;
    }
  }
  int size = finfo->size;
  int Bmp = dictaddr;
  char BMP_PATH[224 * 13 + 1];
  for (i = 0; i < 224 * 13; i++) {
    BMP_PATH[i] = 0;
  }
  strcpy(BMP_PATH, path);
  while (dictaddr != 0x2600) {
    if (dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224) !=
        0) {
      struct FILEINFO *finfo_this_dict_clust =
          dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
      struct FILEINFO *finfo_this_dict = clust_sech(
          finfo_this_dict_clust->clustno,
          (struct FILEINFO *)(ADR_DISKIMG + Get_dictaddr("../")), 224);
      finfo_this_dict->size += size;
    }
    changedict("..");
  }
  dictaddr = Bmp;
  strcpy(path, BMP_PATH);
  return;
}

vram_t *buf_text_window;
struct SHEET *sht_text_window;

void close_text_window() {
  sheet_free(sht_text_window);
  page_free((int)buf_text_window, 800 * 600 * sizeof(color_t));
}

void chview(char *filename) {
  struct FILEINFO *finfo = Get_File_Address(filename);
  char *p = fopen(filename);
  if (p == 0) {
    print("Can't find file ");
    print(filename);
    print("\n");
    return;
  }
  if (running_mode == POWERINTDOS) {
    SwitchToHighTextMode(); //切换到高分辨率文本模式
    HighPutStr(p, finfo->size);
    getch();
    Close_High_Text_Mode();
    SwitchToText8025_BIOS();
    Set_Font(font);
  } else if (running_mode == POWERDESKTOP) {
    extern struct SHTCTL *shtctl;
    buf_text_window = (vram_t *)page_malloc(800 * 600 * sizeof(color_t));
    sht_text_window = MakeWindow(50, 50, 800, 600, "chview", shtctl,
                                 buf_text_window, (int)close_text_window);
    sheet_updown(sht_text_window, shtctl->top - 1);
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
    extern char *pages;
    if (pages[i] == 0)
      free++;
  }
  printf("free vpages:%d free kpages:%d\nfree:%dKB\n", free,
         free - (1024 * 768 - memsize / (4 * 1024)),
         (free - (1024 * 768 - memsize / (4 * 1024))) * 4);
  return;
}

void set_cons_color(char color) { cons_color = color; }

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
      *(char *)(0xb8000 + my1 * 160 + mx1 * 2 + 1) = 0x07;
      *(char *)(0xb8000 + my * 160 + mx * 2 + 1) = 0x70;
      i = fifo8_get(TaskGetMousefifo(NowTask()));
      // io_sti();
      if (mouse_decode(&mdec, i) != 0) {
        mx1 = mx; // 更新mx1,my1
        my1 = my;
        bufx += mdec.x;
        bufy += mdec.y;
        mx = bufx / 8;  // 计算位置
        my = bufy / 16; // 计算位置
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
  mouse_sleep(&mdec); //让鼠标暂时停止不向FIFO缓冲区写入数据
  mx = mx1;
  my = my1;
  return;
}

void cmd_tl() {
  // tl：tasklist
  // 显示当前运行的任务
  extern int tasknum; //任务数量（定义在task.c）
  for (int i = 0; i != tasknum + 1; i++) {
    printf("Task %d: Name:%s,Level:%d,Sleep:%d,GDT address:%d*8\n", i,
           GetTask(i)->name, GetTask(i)->level, GetTask(i)->sleep,
           GetTask(i)->sel / 8);
  }
}
void cmd_vbetest() {
  int result = check_vbe_mode(0x101, (struct VBEINFO *)VBEINFO_ADDRESS);
  if (result == -1) {
    printf("VBE Error: You Computer does not support VBE mode!\n");
  } else {
    printf("VBE OK: You Computer supports VBE mode!\n");
    regs16_t r;
    r.ax = 0x4f00;
    r.es = 0x07e0;
    r.di = 0x0000;
    INT(0x10, &r);
    VESAControllerInfo *info = (VESAControllerInfo *)0x07e00;
    printf("VBE OemString: %s\n", rmfarptr2ptr(info->oemString));
    printf("VBE Version: %04x\n", info->Version);
    printf("Video Memory: %d MB\n\n", info->totalMemory * 64 / 1024);
  }
}
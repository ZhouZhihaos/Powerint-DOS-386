#include <dos.h>
static uint8_t chat_data[2048];
static uint8_t chat_flags = 1;
static struct Socket *socket;
static void Chat_Handler(struct Socket *socket, void *base) {
  memcpy(chat_data, base, 2048);
  chat_flags = 0;
}
static void out_put_task_main_cmd() {
  while (1) {
    clean(chat_data, 2048);
    chat_flags = 1;
    while (chat_flags)
      ;
    struct UDPMessage *udp =
        (struct UDPMessage *)(chat_data + sizeof(struct EthernetFrame_head) +
                              sizeof(struct IPV4Message));
    uint8_t *dat = chat_data + sizeof(struct EthernetFrame_head) +
                   sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
    char *p = malloc(swap16(udp->length) - 8 + 1);
    memcpy(p, dat, swap16(udp->length) - 8);
    p[swap16(udp->length) - 8] = 0;
    printf("\nFrom 118.31.248.215 Server ### %s\n", p);
    free(p);
  }
}
void chat_cmd() {
  extern uint32_t ip;
  printf("Welcome to Powerint DOS 386 Chat!\n");
  printf("Connect 118.31.248.215...\n");
  if (ping(CHAT_SERVER_IP) == -1) { // 网卡正常撒？
    printf("The ICMP packet is not from 118.31.248.215.\n");
    return;
  }
  printf("Connect 118.31.248.215 done.\n");
  printf("Connect Server...\n");
  socket = Socket_Alloc(UDP_PROTOCOL);
  Socket_Init(socket, CHAT_SERVER_IP, CHAT_SERVER_PROT, ip, CHAT_CLIENT_PROT);
  Socket_Bind(socket, Chat_Handler);
  clean(chat_data, 2048);
  socket->Send(socket, "CONNECT ", strlen("CONNECT "));
  while (chat_flags)
    ;
  uint8_t *dat = chat_data + sizeof(struct EthernetFrame_head) +
                 sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
  if (strcmp(dat, "OK") != 0) {
    printf("The UDP packet is not from 118.31.248.215.\n");
    return;
  }
  printf("Connect Server done.\n");
  char *inp = page_malloc(1024);
  inp[0] = 'S';
  inp[1] = 'E';
  inp[2] = 'N';
  inp[3] = 'D';
  inp[4] = ' ';
  unsigned int stack = (unsigned int)page_malloc(64 * 1024);
  struct TASK *out_put_task =
      AddTask("out_put_task", 2, 2 * 8, (int)out_put_task_main_cmd, 1 * 8,
              1 * 8, stack + 64 * 1024);
  for (;;) {
    printf("To 118.31.248.215 Server ### ");
    input(&inp[5], 1019);
    socket->Send(socket, inp, strlen(inp));
  }
}
static listBox_t *listbox;
static TextBox *textbox;
static Button *button;
static vram_t *buf_win;
static struct SHEET *sht_win;
static void out_put_task_main_gui() {
  while (1) {
    clean(chat_data, 2048);
    chat_flags = 1;
    while (chat_flags)
      ;
    struct UDPMessage *udp =
        (struct UDPMessage *)(chat_data + sizeof(struct EthernetFrame_head) +
                              sizeof(struct IPV4Message));
    uint8_t *dat = chat_data + sizeof(struct EthernetFrame_head) +
                   sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
    char *p = malloc(swap16(udp->length) - 8 + 1);
    memcpy(p, dat, swap16(udp->length) - 8 + 1);
    p[swap16(udp->length) - 8] = 0;
    AddItem(listbox, p);
    free(p);
  }
}
static void send() {
  extern uint32_t ip;
  char *inp = page_malloc(1024);
  inp[0] = 'S';
  inp[1] = 'E';
  inp[2] = 'N';
  inp[3] = 'D';
  inp[4] = ' ';
  memcpy(&inp[5], textbox->text, strlen(textbox->text));
  inp[5 + strlen(textbox->text)] = NULL;
  socket->Send(socket, inp, strlen(inp));
  page_free(inp, 1024);
}
void chat_gui() {
  extern uint32_t ip;
  printf("Welcome to Powerint DOS 386 Chat!\n");
  printf("Connect 118.31.248.215...\n");
  if (ping(CHAT_SERVER_IP) == -1) { // 网卡正常撒？
    printf("The ICMP packet is not from 118.31.248.215.\n");
    return;
  }
  printf("Connect 118.31.248.215 done.\n");
  printf("Connect Server...\n");
  socket = Socket_Alloc(UDP_PROTOCOL);
  Socket_Init(socket, CHAT_SERVER_IP, CHAT_SERVER_PROT, ip, CHAT_CLIENT_PROT);
  Socket_Bind(socket, Chat_Handler);
  clean(chat_data, 2048);
  socket->Send(socket, "CONNECT ", strlen("CONNECT "));
  while (chat_flags)
    ;
  uint8_t *dat = chat_data + sizeof(struct EthernetFrame_head) +
                 sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
  if (strcmp(dat, "OK") != 0) {
    printf("The UDP packet is not from 118.31.248.215.\n");
    return;
  }

  extern struct SHTCTL *shtctl;
  buf_win = (vram_t *)page_malloc(800 * 600 * sizeof(color_t));
  sht_win = MakeWindow(150, 150, 852, 430, "Chat", shtctl, buf_win, NULL);
  PutChineseStr0(buf_win, 852, 6, 21, COL_000000,
                 "接收消息栏（来自              服务器）下框");
  SDraw_Box(buf_win, 6 + 8 * 16, 21, 6 + 14 * 8 + 8 * 16, 21 + 16, COL_C6C6C6,
            852);
  Sputs(buf_win, "118.31.248.215", 6 + 8 * 16, 21, COL_000000, 852);
  listbox = MakeListBox(sht_win, 6, 60, 815, 360);
  PutChineseStr0(buf_win, 852, 500, 21, COL_000000,
                 "发送消息栏（发向              服务器）上框");
  SDraw_Box(buf_win, 500 + 8 * 16, 21, 500 + 14 * 8 + 8 * 16, 21 + 16,
            COL_C6C6C6, 852);
  Sputs(buf_win, "118.31.248.215", 500 + 8 * 16, 21, COL_000000, 852);
  textbox = MakeTextBox(6, 40, 800, 16, sht_win);
  button = MakeButton(806, 13, 40, 16, sht_win, "Send", send);

  unsigned int stack = (unsigned int)page_malloc(64 * 1024);
  struct TASK *out_put_task =
      AddTask("out_put_task", 2, 2 * 8, (int)out_put_task_main_gui, 1 * 8,
              1 * 8, stack + 64 * 1024);
}

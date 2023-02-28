#include <dos.h>
static uint8_t chat_data[2048]; // 接收网卡发来的数据包
static uint8_t chat_flags = 1;  // 网卡发来数据包了吗？
static struct Socket *socket;   // socket API需要用到这个结构体
static void Chat_Handler(struct Socket *socket, void *base) { // 有数据接收时会调用这个函数
  memcpy(chat_data, base, 2048);  // 拷贝数据
  chat_flags = 0;   // 已经接收完成
}
static void out_put_task_main_cmd() { // 这是一个关于接收数据并输出的进程
  while (1) {
    clean((char *)chat_data, 2048); // 清空缓冲区
    chat_flags = 1; // 等待接收
    while (chat_flags) // 如果上面handler设置chat_flags为0，那么就会跳出循环，所以这里就是等待数据包发送
      ;
    struct UDPMessage *udp =
        (struct UDPMessage *)(chat_data + sizeof(struct EthernetFrame_head) +
                              sizeof(struct IPV4Message)); // 获取到udp协议头
    uint8_t *dat = chat_data + sizeof(struct EthernetFrame_head) +
                   sizeof(struct IPV4Message) + sizeof(struct UDPMessage); // 获取数据包
    char *p = malloc(swap16(udp->length) - 8 + 1); // malloc一个新的缓冲区存放数据
    memcpy(p, dat, swap16(udp->length) - 8); // 复制数据
    p[swap16(udp->length) - 8] = 0; // 设置字符串结束符（因为下面要直接print）
    printf("\nFrom 118.31.248.215 Server ### %s\n", p); // 打印提示符+数据
    free(p); // 释放指针
  }
}
void chat_cmd() { // 用于接收用户想要发送的内容
  extern uint32_t ip; // 系统IP地址
  /* 打印提示符 */
  printf("Welcome to Powerint DOS 386 Chat!\n");
  printf("Connect 118.31.248.215...\n");
  if (ping(CHAT_SERVER_IP) == -1) { // 网卡正常撒？
    printf("The ICMP packet is not from 118.31.248.215.\n"); // 连接失败
    return;
  }
  printf("Connect 118.31.248.215 done.\n"); // 到这里说明连接成功
  printf("Connect Server...\n"); // 现在通过UDP协议连接服务器
  socket = Socket_Alloc(UDP_PROTOCOL); // 向SocketAPI分配一个socket位置，让我们通过Socket API连接到服务器（声明使用UDP协议）
  Socket_Init(socket, CHAT_SERVER_IP, CHAT_SERVER_PROT, ip, CHAT_CLIENT_PROT); // 对Socket进行初始化（设置服务器的IP、端口，以及自身的IP、端口）
  Socket_Bind(socket, Chat_Handler); // 绑定回调函数
  clean((char *)chat_data, 2048); // 清空缓冲区
  socket->Send(socket, (uint8_t *)"CONNECT ", strlen("CONNECT ")); // 告诉服务器我们已经连接
  while (chat_flags) // 等待服务器回复
    ;
  uint8_t *dat = chat_data + sizeof(struct EthernetFrame_head) +
                 sizeof(struct IPV4Message) + sizeof(struct UDPMessage); // 获取数据
  if (strcmp((char *)dat, "OK") != 0) { // 如果服务器是chat程序的正确服务端，那么将会返回OK字样
    printf("The UDP packet is not from 118.31.248.215.\n"); // 啊哦，不是
    return;
  }
  printf("Connect Server done.\n"); // 到这里说明是了
  char *inp = page_malloc(1024); // 设置输入缓冲区
  /* 设置声明指令（5个字节） */
  inp[0] = 'S';
  inp[1] = 'E';
  inp[2] = 'N';
  inp[3] = 'D';
  inp[4] = ' ';
  unsigned int stack = (unsigned int)page_malloc(64 * 1024); // 声明接收服务程序的task的栈空间
  struct TASK *out_put_task =
      AddTask("out_put_task", 2, 2 * 8, (int)out_put_task_main_cmd, 1 * 8,
              1 * 8, stack + 64 * 1024); // 创建进程
  (void)(out_put_task);
  for (;;) {
    printf("To 118.31.248.215 Server ### "); // 提示符信息
    input(&inp[5], 1019); // 输入，&inp[5]是因为上面声明了指令（五个字节），同样，1019是通过1024-5得到的
    socket->Send(socket, (uint8_t *)inp, strlen(inp)); // 通过socket API发送
  }
}
/* 声明用到的GUI控件 */
static listBox_t *listbox;
static TextBox *textbox;
static Button *button;
static vram_t *buf_win;
static struct SHEET *sht_win; // 窗口图层
static void out_put_task_main_gui() { // GUI模式下的接收
  while (1) {
    clean((char *)chat_data, 2048);
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
    AddItem(listbox, p, NULL); // 直接加入到listBox控件中
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
  memcpy(&inp[5], textbox->text, strlen(textbox->text)); // 这里获取textbox的输入
  inp[5 + strlen(textbox->text)] = 0;
  socket->Send(socket, (uint8_t *)inp, strlen(inp));
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
  clean((char *)chat_data, 2048);
  socket->Send(socket, (uint8_t *)"CONNECT ", strlen("CONNECT "));
  while (chat_flags)
    ;
  uint8_t *dat = chat_data + sizeof(struct EthernetFrame_head) +
                 sizeof(struct IPV4Message) + sizeof(struct UDPMessage);
  if (strcmp((char *)dat, "OK") != 0) {
    printf("The UDP packet is not from 118.31.248.215.\n");
    return;
  }

  extern struct SHTCTL *shtctl;
  buf_win = (vram_t *)page_malloc(800 * 600 * sizeof(color_t));  // 创建窗口的显示缓冲区
  sht_win = MakeWindow(150, 150, 852, 430, "Chat", shtctl, buf_win, NULL); // 创建窗口
  PutChineseStr0(buf_win, 852, 6, 21, COL_000000,
                 (uint8_t *)"接收消息栏（来自              服务器）下框"); // 输出汉字提示符
  SDraw_Box(buf_win, 6 + 8 * 16, 21, 6 + 14 * 8 + 8 * 16, 21 + 16, COL_C6C6C6,
            852);
  Sputs(buf_win, "118.31.248.215", 6 + 8 * 16, 21, COL_000000, 852);
  listbox = MakeListBox(sht_win, 6, 60, 815, 360);
  PutChineseStr0(buf_win, 852, 500, 21, COL_000000,
                 (uint8_t *)"发送消息栏（发向              服务器）上框");
  SDraw_Box(buf_win, 500 + 8 * 16, 21, 500 + 14 * 8 + 8 * 16, 21 + 16,
            COL_C6C6C6, 852);
  Sputs(buf_win, "118.31.248.215", 500 + 8 * 16, 21, COL_000000, 852);
  textbox = MakeTextBox(6, 40, 800, 16, sht_win);
  button = MakeButton(806, 13, 40, 16, sht_win, "Send", send);

  unsigned int stack = (unsigned int)page_malloc(64 * 1024);
  struct TASK *out_put_task =
      AddTask("out_put_task", 2, 2 * 8, (int)out_put_task_main_gui, 1 * 8,
              1 * 8, stack + 64 * 1024);
  (void)(stack);
  (void)(out_put_task);
}

#include <dos.h>
#define LI_ 0
#define VN_ 3
#define MODE_ 3
#define STRATUM_ 0
#define POLL_ 4
#define PREC_ -6
#define JAN_1970 0x83aa7e80
#define COMMON_YEAR_SEC 31536000
#define LEAP_YEAR_SEC 31622400
#define DAY_SEC 86400
static int table1[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int table2[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static uint32_t NTPTime;
static void ntp_handler(struct Socket *socket, void *base) {
  // printf("Recv: ");
  struct NTPMessage *ntp =
      (struct NTPMessage *)(base + sizeof(struct EthernetFrame_head) +
                            sizeof(struct IPV4Message) +
                            sizeof(struct UDPMessage));
  // for (int i = 0; i != sizeof(struct NTPMessage); i++) {
  //   printf("%02x ", p[i]);
  // }
  NTPTime = swap32(ntp->Transmission_Timestamp);
}
uint32_t ntp_get_server_time(uint32_t NTPServerIP) {
  struct Socket *socket = socket_alloc(UDP_PROTOCOL);
  extern uint32_t ip;
  Socket_Init(socket, NTPServerIP, 123, ip, 123);
  Socket_Bind(socket, ntp_handler);
  struct NTPMessage *ntp =
      (struct NTPMessage *)malloc(sizeof(struct NTPMessage));
  ntp->LI = LI_;
  ntp->VN = VN_;
  ntp->Mode = MODE_;
  ntp->Startum = STRATUM_;
  ntp->Poll = POLL_;
  ntp->Precision = PREC_;
  ntp->Root_Delay = 1 << 8;
  ntp->Root_Difference = 1 << 8;
  ntp->Transmission_Timestamp =
      swap32(ntp_time_stamp(get_year(), get_mon_hex(), get_day_of_month(),
                          get_hour_hex(), get_min_hex(), get_sec_hex()));
  NTPTime = NULL;
  while (!NTPTime) {
    socket->Send(socket, (uint8_t *)ntp, sizeof(struct NTPMessage));
    sleep(400);
  }
  // printf("SENDING: ");
  // for (int i = 0; i != sizeof(struct NTPMessage); i++) {
  //   printf("%02x ", p[i]);
  // }
  // printf("\n");
  socket_free(socket);
  return NTPTime;
}
uint32_t ntp_time_stamp(uint32_t year, uint32_t month, uint32_t day,
                      uint32_t hour, uint32_t min, uint32_t sec) {
  uint32_t leap = 0;
  for (int y = 1900; y != year; y++) {
    if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) {
      leap++;
    }
  }
  uint32_t s = 0;
  if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
    for (int i = 0; i != month - 1; i++) {
      s += table2[i] * DAY_SEC;
    }
  } else {
    for (int i = 0; i != month - 1; i++) {
      s += table1[i] * DAY_SEC;
    }
  }
  s += (day - 1) * DAY_SEC + hour * 60 * 60 + min * 60 + sec;
  return leap * LEAP_YEAR_SEC + (year - 1900 - leap) * COMMON_YEAR_SEC + s -
         28800;
}
uint32_t UTCTimeStamp(uint32_t year, uint32_t month, uint32_t day,
                      uint32_t hour, uint32_t min, uint32_t sec) {
  return ntp_time_stamp(year, month, day, hour, min, sec) - JAN_1970;
}
void UnNTPTimeStamp(uint32_t timestamp, uint32_t *year, uint32_t *month,
                    uint32_t *day, uint32_t *hour, uint32_t *min,
                    uint32_t *sec) {
  timestamp += 28800;
  uint32_t y = 1900;
  for (;; y++) {
    if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) {
      timestamp -= LEAP_YEAR_SEC;
      if (timestamp <= COMMON_YEAR_SEC) {
        break;
      }
    } else {
      timestamp -= COMMON_YEAR_SEC;
      if (timestamp <= COMMON_YEAR_SEC ||
          ((((y + 1) % 4 == 0 && (y + 1) % 100 != 0) || (y + 1) % 400 == 0) &&
           timestamp <= LEAP_YEAR_SEC)) {
        break;
      }
    }
  }
  *year = y + 1;
  uint32_t month0 = 1;
  if ((*year % 4 == 0 && *year % 100 != 0) || *year % 400 == 0) {
    for (; timestamp > table2[month0 - 1] * DAY_SEC; month0++) {
      timestamp -= table2[month0 - 1] * DAY_SEC;
    }
  } else {
    for (; timestamp > table1[month0 - 1] * DAY_SEC; month0++) {
      timestamp -= table1[month0 - 1] * DAY_SEC;
    }
  }
  *month = month0;
  *day = timestamp / DAY_SEC + 1;
  timestamp = timestamp % DAY_SEC;
  *hour = timestamp / 3600;
  timestamp = timestamp % 3600;
  *min = timestamp / 60;
  *sec = timestamp % 60;
  return;
}
void UnUTCTimeStamp(uint32_t timestamp, uint32_t *year, uint32_t *month,
                    uint32_t *day, uint32_t *hour, uint32_t *min,
                    uint32_t *sec) {
  return UnNTPTimeStamp(timestamp + JAN_1970, year, month, day, hour, min, sec);
}

#include <net.h>
// HTTP
typedef struct HTTPGetHeader {
  char head[11];    // GET / HTTP/
  char version[5];  // 1.1\r\n
  char host[6];     // Host:
} HTTPGetHeader;

bool IsHttpGetHeader(uint8_t* data, uint32_t size) {
  if (size < sizeof(HTTPGetHeader)) {
    return false;
  }
  HTTPGetHeader* header = (HTTPGetHeader*)data;
  if (memcmp(header->head, "GET / HTTP/", 11) == 0 &&
      memcmp(header->version, "1.1\r\n", 5) == 0 &&
      memcmp(header->host, "Host:", 5) == 0) {
    return true;
  }
  return false;
}

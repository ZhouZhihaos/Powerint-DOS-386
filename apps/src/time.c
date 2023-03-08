#include <time.h>
#include <rand.h>
time_t time(time_t timer) {
  return RAND();
}
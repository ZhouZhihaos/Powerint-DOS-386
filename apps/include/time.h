#ifndef __TIME__H__
#define __TIME__H__
typedef long clock_t;
struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};
typedef unsigned int time_t;
time_t time(time_t timer);
clock_t clock();
#endif
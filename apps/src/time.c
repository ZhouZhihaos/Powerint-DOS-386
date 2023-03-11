#include <time.h>
#include <rand.h>
time_t time(time_t timer) {
  return RAND();
}

//TODO: wday
struct tm *localtime(time_t t)
{
    struct tm *tm_;
    tm_->tm_sec = t % 60;
    tm_->tm_min = t % 3600 / 60;
    tm_->tm_hour = (t % (216000)) / 3600 % 24;
    int years = 1900;
    int days = t / 86400;
    while (days >= 365)
    {
        years++;
        if (years % 4 == 0 && years % 100 != 0)
            days -= 366;
        else if (years % 400 == 0)
            days -= 366;
        else
            days -= 365;
    }
    tm_->tm_year = years;
    tm_->tm_yday = days;
    char mons[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (char i = 0; i < 12; i++)
    {
        days -= mons[i];
        if (days <= 0)
        {
            tm_->tm_mon = i;
            tm_->tm_mday = days + mons[i] + 1;
            break;
        }
    }

    return tm_;
} 
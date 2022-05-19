#pragma once

#include <stdint.h>
#include <string>

#include <time.h>

#include "copyable.h"

class TimeStamp : copyable
{
public:
    TimeStamp() : microSecondsSinceEpoch_(0){};
    explicit TimeStamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch){};

    static TimeStamp Now()
    {
        return TimeStamp(time(NULL));
    }

    std::string ToString() const
    {
        char buf[128] = {0};
        tm *tm_time = localtime(&microSecondsSinceEpoch_);
        snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
                 tm_time->tm_year + 1900,
                 tm_time->tm_mon + 1,
                 tm_time->tm_mday,
                 tm_time->tm_hour,
                 tm_time->tm_min,
                 tm_time->tm_sec);
        return buf;
    }

private:
    int64_t microSecondsSinceEpoch_;
};
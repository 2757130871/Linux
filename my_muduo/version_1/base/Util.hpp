#pragma once

#include <stdlib.h>
#include <string>

#include <iostream>
#include <fstream>

#include <sys/time.h>
#include <stdint.h>

#include "TimeStamp.hpp"

////////////////////////////////////
//时间戳工具
////////////////////////////////////

class TimeUtil
{
public:
    //获取当前的时间戳
    static int64_t TimeStamp()
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec;
    }

    //时间戳 毫秒
    static int64_t TimeStampMS()
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
};

////////////////////////////////////
//打印日志工具
////////////////////////////////////

enum Level
{
    INFO,
    WARING,
    ERROR,
    FATAL,
};

inline std::ostream &Log(Level level,
                         const std::string &file_name, int line_num)
{
    std::string prefix = "[";
    if (level == INFO)
    {
        prefix += "INFO ";
    }
    else if (level == WARING)
    {
        prefix += "WARNING ";
    }
    else if (level == ERROR)
    {
        prefix += "ERROR ";
    }
    else if (level == FATAL)
    {
        prefix += "FATAL ";
    }

    prefix += (TimeStamp::Now().ToString());
    prefix += " ";
    prefix += file_name;
    prefix += ":";
    prefix += std::to_string(line_num);
    prefix += "] ";
    std::cerr << prefix;

    return std::cerr;
}

#define LOG(level) Log(level, __FILE__, __LINE__)

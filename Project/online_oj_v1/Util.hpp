#pragma once
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>

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
        prefix += "I";
    }
    else if (level == WARING)
    {
        prefix += "w";
    }
    else if (level == ERROR)
    {
        prefix += "E";
    }
    else if (level == FATAL)
    {
        prefix += "F";
    }
    prefix += std::to_string(TimeUtil::TimeStamp());
    prefix += " ";
    prefix += file_name;
    prefix += ":";
    prefix += std::to_string(line_num);
    prefix += "] ";
    std::cout << prefix;

    return std::cout;
}

#define LOG(level) Log(level, __FILE__, __LINE__)

////////////////////////////////////
//文件工具
////////////////////////////////////

class FileUtil
{
public:
    //传入一个文件路径，帮我们把文件的所有内容都读出来放/1到 content字符串中
    //输入型参数const引用
    //输出型参数指针
    //输入输出型参数引用
    static bool Read(const std::string &file_path, std::string *content)
    {
        content->clear();
        std::ifstream file(file_path.c_str());

        if (!file.is_open())
        {
            return false;
        }

        std::string line;
        while (std::getline(file, line))
        {
            *content += line + "\n";
        }
        file.close();
        return true;
    }

    static bool Write(const std::string &file_path, const std::string &content)
    {
        std::ofstream file(file_path.c_str());
        if (!file.is_open())
            return false;

        file.write(content.c_str(), content.size());
        file.close();
        return true;
    }

   
};

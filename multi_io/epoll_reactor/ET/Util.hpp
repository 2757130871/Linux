#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include <sys/time.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <boost/algorithm/string.hpp>

//ReadLine  and CutString
class Util
{
public:
        //从接收缓冲区中读取一行，
        static int ReadLine(int sock, std::string &out)
        {

                char ch = 'X';
                while (ch != '\n')
                {
                        ssize_t s = recv(sock, &ch, 1, 0);
                        if (s > 0)
                        {
                                if (ch == '\r')
                                {
                                        recv(sock, &ch, 1, MSG_PEEK); //窥探
                                        if (ch == '\n')
                                        {
                                                recv(sock, &ch, 1, 0);
                                        }
                                        else
                                        {
                                                ch = '\n';
                                        }
                                }
                                out += ch;
                        }
                        else if (s == 0)
                        {
                                return 0;
                        }
                        else
                        {
                                return -1;
                        }
                }

                return out.size();
        }

        static bool CurString(const std::string &target, std::string &key_out, std::string &value_out, const std::string &sep)
        {
                size_t pos = target.find(sep);
                if (pos != std::string::npos)
                {
                        key_out = target.substr(0, pos);
                        value_out = target.substr(pos + sep.size());
                        return true;
                }

                return false;
        }
};

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
        std::cerr << prefix;

        return std::cerr;
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

////////////////////////////////////
// 字符串操作
////////////////////////////////////

// 字符串切分
// 1.strtok 2. stringstream 3. boost split函数
class StringUtil
{
public:
        // aaa bbb ccc=>3 token_compress_on
        // aaa  bbb ccc =>3 vs 4  token_compress_off
        static void Split(const std::string &input,
                          const std::string &split_char,
                          std::vector<std::string> *output)
        {
                boost::split(*output, input, boost::is_any_of(split_char),
                             boost::token_compress_on);
        }
};

////////////////////////////////////
// URL 解析
////////////////////////////////////

class UrlUtil
{
public:
        static void ParseBody(const std::string &body,
                              std::unordered_map<std::string, std::string> *params)
        {
                // 1．先对这里body字符串进行切分，切分成键值对的形式
                //a)先按照&符号切分
                //b)再按照=切分
                std::vector<std::string> kvs;
                StringUtil::Split(body, "&", &kvs);

                std::vector<std::string> tmp_kv;
                for (const auto &var : kvs)
                {
                        tmp_kv.clear();
                        StringUtil::Split(var, "=", &tmp_kv);
                        if (tmp_kv.size() != 2)
                                continue;

                        (*params)[tmp_kv[0]] = UrlDecode(tmp_kv[1]); //对value进行 decode
                }

                // 2．对这里的键值对进行urldecode
        }

        static unsigned char ToHex(unsigned char x)
        {
                return x > 9 ? x + 55 : x + 48;
        }

        static unsigned char FromHex(unsigned char x)
        {
                unsigned char y;
                if (x >= 'A' && x <= 'Z')
                        y = x - 'A' + 10;
                else if (x >= 'a' && x <= 'z')
                        y = x - 'a' + 10;
                else if (x >= '0' && x <= '9')
                        y = x - '0';
                else
                        assert(0);
                return y;
        }

        static std::string UrlEncode(const std::string &str)
        {
                std::string strTemp = "";
                size_t length = str.length();
                for (size_t i = 0; i < length; i++)
                {
                        if (isalnum((unsigned char)str[i]) ||
                            (str[i] == '-') ||
                            (str[i] == '_') ||
                            (str[i] == '.') ||
                            (str[i] == '~'))
                                strTemp += str[i];
                        else if (str[i] == ' ')
                                strTemp += "+";
                        else
                        {
                                strTemp += '%';
                                strTemp += ToHex((unsigned char)str[i] >> 4);
                                strTemp += ToHex((unsigned char)str[i] % 16);
                        }
                }
                return strTemp;
        }

        static std::string UrlDecode(const std::string &str)
        {
                std::string strTemp = "";
                size_t length = str.length();
                for (size_t i = 0; i < length; i++)
                {
                        if (str[i] == '+')
                                strTemp += ' ';
                        else if (str[i] == '%')
                        {
                                assert(i + 2 < length);
                                unsigned char high = FromHex((unsigned char)str[++i]);
                                unsigned char low = FromHex((unsigned char)str[++i]);
                                strTemp += high * 16 + low;
                        }
                        else
                                strTemp += str[i];
                }
                return strTemp;
        }
};

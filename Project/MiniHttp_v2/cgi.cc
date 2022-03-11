#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include "Log.hpp"

using namespace std;

bool GetQueryString(std::string &query_string)
{
    bool result = false;
    std::string method = getenv("METHOD");

    if (method == "GET")
    {
        query_string = getenv("QUERY_STRING");

        result = true;
    }
    else if (method == "POST")
    {
        //CGI如何得知需要从标准输入读取多少个字节呢
        int content_length = atoi(getenv("CONTENT_LENGTH"));
        cerr << "DEBUG content_length :" << content_length << endl;
        char c = 0;
        while (content_length)
        {
            read(0, &c, 1);
            query_string.push_back(c);
            content_length--;
        }
        result = true;
    }
    else
    {
        result = false;
    }
    return result;
}

void CutString(std::string &in, const std::string &sep, std::string &outl, std::string &out2)
{
    auto pos = in.find(sep);
    if (std::string::npos != pos)
    {
        outl = in.substr(0, pos);
        out2 = in.substr(pos + sep.size());
    }
}

// CGI 程序表尊输入和标准输出已经被重定向了 不能使用cout
int main()
{
    std::string method = getenv("METHOD");
    cerr << "CGI START ------------------------------" << endl;
    std::string query_string;

    GetQueryString(query_string); //a-100&b-200
    
    std::string str1;
    std::string str2;

    CutString(query_string, "&", str1, str2);
    // CutString(query_string，“&”，strl，str2)；

    std::string name1;
    std::string value1;
    CutString(str1, "=", name1, value1);
    cerr << "DEBUG name1: " << name1 << "value1: " << value1 << endl;

    std::string name2;
    std::string value2;
    CutString(str2, "=", name2, value2);
    cerr << "DEBUG name2: " << name2 << "value2: " << value2 << endl;

    if (method == "GET")
    {

        std::cout << name1 << " : " << value1 << std ::endl;
        std::cout << name2 << " :" << value2 << std::endl;
    }
    else if (method == "POST")
    {

        std::cout << query_string << std::endl;
    }

    std::cerr << name1 << " : " << value1 << std::endl;
    std::cerr << name2 << " : " << value2 << std::endl;

    cerr << "CGI END ------------------------------" << endl;
}

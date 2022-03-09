#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

// CGI 程序表尊输入和标准输出已经被重定向了 不能使用cout
int main()
{
    string method = getenv("METHOD");
    cerr << "CGI: " << method << endl;
    string query_string;
    if (method == "GET")
    {
        query_string = getenv("QUERY_STRING");
        cerr << "---------------------------" << endl;
        cerr << query_string << endl;
        cerr << "---------------------------" << endl;
    }
    else if (method == "POST")
    {
        char buf[255];
        int rd = read(0, buf, 254);
        buf[rd] = 0;
        cerr << "-CGI- :" << buf << endl;
    }
    else
    {
    }
}

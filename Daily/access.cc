#include <iostream>
#include <unistd.h>
using namespace std;

//stat 函数也可以通过返回值是否为0判断文件存在，但是返回值可能是其他错误

// int  access(char* pathname, int mode);   //
// mode: F_OK  R_OK W_OK X_OK
//如果文件存在，返回0

int main()
{
    if(0 == access("1.txt", F_OK))
    {
        cout << "open" << endl;
    }
    else{
        cout << "fatal" << endl;
    }
}

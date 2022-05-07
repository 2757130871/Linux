#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>

using namespace std;

int main()
{
    int fd = open("1.txt", O_RDWR | O_CREAT, 0666);
    cout << "fd: " << fd << endl;
    int ret = write(fd, "123", 3);
    cout << ret << endl;
    lseek(fd, 10, SEEK_SET);
    ret = write(fd, "abc", 1);
    cout << ret << endl;
}

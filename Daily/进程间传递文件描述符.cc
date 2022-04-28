#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <cstring>
#include <iostream>
#include <cassert>

using namespace std;

static const int CONTROL_LEN = CMSG_LEN(sizeof(int));

void send_fd(int fd /*本地套接字*/, int fd_to_send /*待发送的fd*/)
{
    struct iovec iov[1];

    struct msghdr msg;
    char buf[0];
    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    cmsghdr cm;
    cm.cmsg_len = CONTROL_LEN;
    cm.cmsg_level = SOL_SOCKET;
    cm.cmsg_type = SCM_RIGHTS;

    *(int *)CMSG_DATA(&cm) = fd_to_send;
    msg.msg_control = &cm;
    /*设置辅助数据*/
    msg.msg_controllen = CONTROL_LEN;
    sendmsg(fd, &msg, 0);
}

int recv_fd(int fd)
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];
    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen - 1;

    cmsghdr cm;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;
    recvmsg(fd, &msg, 0);
    int fd_to_read = *(int *)CMSG_DATA(&cm);
    return fd_to_read;
}

int main()
{

    int fd[2];
    int ret = socketpair(AF_UNIX, SOCK_DGRAM, 0, fd);
    assert(ret >= 0);
    // parent
    if (fork() > 0)
    {
        // close(fd[1]);

        int ret = recv_fd(fd[0]);

        char buf[255];
        int rd = read(ret, buf, sizeof(buf) - 1);

        wait();
        buf[rd] = 0;
        cout << buf << endl;

        exit(0);
    }

    // child
    // close(fd[0]);

    int open_fd = open("1.txt", O_RDWR, 0666);
    cout << open_fd << endl;
    send_fd(fd[1], open_fd);

    close(open_fd);

    exit(0);
}

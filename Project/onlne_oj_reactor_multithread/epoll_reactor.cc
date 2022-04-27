#include <signal.h>

#include "EpollCallBack.hpp"
#include "TcpServer.hpp"

#include "ThreadPool.hpp"

#define ALARM_TIMEOUT 4

void Usage()
{
    cout << "Usage: ./server port" << endl;
    exit(-1);
}

// epoll_wait 返回时拷贝事件到evs数组
struct epoll_event evs[MAX_CONNECTION];
//存放封装回调事件的结构体
struct Epoll_CallBack cbs[MAX_CONNECTION];

//检测长连接是否活跃,关闭超时的连接
void *CheckTimeout(void *arg)
{
    while (1)
    {
        //每隔一分钟检测一次
        sleep(MAX_TIMEOUT);

        pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&lock);
        for (int i = 1; i < MAX_CONNECTION; i++)
        {
            Epoll_CallBack &call_back = cbs[i];
            if (-1 == call_back.fd)
                continue;

            if (TimeUtil::TimeStamp() - call_back.timestamp > MAX_TIMEOUT)
            {
                LOG(INFO) << "DEBUG: Timeout delete connection: " << call_back.fd << endl;
                Epoll_CallBack::DelConection(&call_back);
            }
        }
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

// EPOLL reactor + ET
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage();
    }

    //防止服务器被SIGPIPE信号终止
    signal(SIGPIPE, SIG_IGN);

    //注册检测超时函数
    // signal(SIGALRM, CheckTimeout);

    //此线程负责检连接是否超时
    // pthread_t t1;
    // pthread_create(&t1, NULL, CheckTimeout, NULL);

    //获取lsock
    int port = atoi(argv[1]);
    TcpServer *tcp_server = TcpServer::GetInstance(port);
    int lsock = tcp_server->GetLsock();

    //创建eoll模型
    int epfd = epoll_create(1);

    //添加lsock
    struct epoll_event ent;
    ent.events = EPOLLIN | EPOLLONESHOT;

    //设置lsock
    cbs[0] = Epoll_CallBack(epfd, lsock, ent.events, GET_LINK, cbs, TimeUtil::TimeStamp());

    ent.data.ptr = &cbs[0];
    epoll_ctl(epfd, EPOLL_CTL_ADD, lsock, &ent);

    ThreadPool *thread_pool = ThreadPool::GetInstance();

    while (1)
    {
        int nready = epoll_wait(epfd, evs, 1024, -1);
        if (nready < 0)
        {
            LOG(FATAL) << "epoll_wait return : -1!" << endl;
            continue;
        }

        for (int i = 0; i < nready; i++)
        {
            Epoll_CallBack *call_back = static_cast<Epoll_CallBack *>(evs[i].data.ptr);
            thread_pool->PushTask(new Task(call_back));
        }
    }
}

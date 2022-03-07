#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

using namespace std;

void* func(void* arg)
{
    cout << pthread_self() << endl;
    return (void*)110;   
}

int main()
{
    pthread_t t1;
    pthread_create(&t1, NULL, func, NULL);

    void* ret;
    cout << "wait thread quit..." << endl;
    pthread_join(t1, &ret);

    cout << (long)ret <<endl;
}

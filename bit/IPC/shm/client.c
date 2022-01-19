#include <stdio.h>
#include <sys/types.h>
#include  <sys/ipc.h>
#include <sys/stat.h>
#include "comm.h"
#include <sys/shm.h>
#include <unistd.h>
int main()
{
    key_t key = ftok(PATH, PROJ_ID);
    
    int ret = shmget(key, 4096, 0);
    // shm attach 
    char* addr = (char*)shmat(ret, NULL, 0);
    
        sleep(8);
    //shm 
    shmdt(addr);



   // shmctl(ret, IPC_RMID, NULL);
    printf("ret : %d\n",ret);
}

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
    //printf("key : %d\n");
    
    int ret = shmget(key, 4096, IPC_CREAT | IPC_EXCL | 0666);
    

    // shm attach 
    char* addr = (char*)shmat(ret, NULL, 0);
    
        sleep(8);
    //shm 
    shmdt(addr);



   // shmctl(ret, IPC_RMID, NULL);
    printf("ret : %d\n",ret);
}

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

int main()
{
    int fds[2];
    int ret = pipe(fds);
    if(ret == -1){
      perror("错误");
      exit(-1);
    }
    pid_t pid = fork();
    char buf[64];

    if(pid == 0)
    {
        close(fds[1]);
        memset(buf,0,64);
        int readret = read(fds[0],buf,64);
        printf("child read :%d\n",readret);
        close(fds[0]);
        
        printf("%s\n",buf);
        exit(0);
    }

    close(fds[0]);
  int n = write(fds[1],"helloworld",11);
  if(n < 0){
    printf("write error\n");
    return 0;
  }
    close(fds[1]);
    printf("parent write :%d\n",n);
    return 0;    
}

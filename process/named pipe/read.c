#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>


int main()
{
    int ret = open("fifo1",O_RDONLY);
    if(-1 == ret){
      perror("open error");
      return -1;
    }

    char buf[64];
    memset(buf, 0, 64);
   // int rCount = read(STDIN_FILENO, buf, 64);
   // printf("写了%d字节\n",rCount);

    int count = read(ret, buf, 64);
    printf("读了%d字节\n",count);
    printf("%s\n",buf);
}

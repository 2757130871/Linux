#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc,char* argv[])
{

   int x = atoi(argv[2]);
   int y = atoi(argv[3]);
   
  if(strcmp(argv[1],"-a") == 0)
      printf("%d\n",x + y);
  else if(strcmp(argv[1],"-s") == 0)
      printf("%d\n",x - y);
  else if(strcmp(argv[1],"-m") == 0)
    printf("%d\n",x * y);
  else 
    printf("%d\n",x / y);
}

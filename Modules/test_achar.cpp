#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>



int main()
{

   unsigned int fd, n, lo[3] = {0,0,0};

   fd = open("/dev/achar", O_RDWR);
    n = read(fd, &lo[0], 12);

   // printf("Timer LO: %08X\n", lo[0] );
   // printf("Timer HI: %08X\n", lo[1] );
   printf("Timer HI_LO: %08X_%08X\n", lo[1],lo[0] );
   printf("Timer CS: %08X\n", lo[2] );


        close(fd);

   return 0;

}

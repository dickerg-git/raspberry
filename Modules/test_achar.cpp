#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


// sudo mknod /dev/achar c 1000 0; sudo chmod 777 /dev/achar
// sudo insmod Achar_mod.ko
//
//      arm-linux-gnueabihf-g++ -o test_achar test_achar.cpp
//      ./test_achar
//
// Timer HI_LO: 00000015_40362C8A
// Timer CS: 00000002
// 

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

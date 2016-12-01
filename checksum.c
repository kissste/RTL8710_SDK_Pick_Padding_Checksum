#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, const char *argv[], const char *envp[])
{
  int resultMain; 
  signed int isGood; 
  int sizeOfBuffer;
  void *buffer; 
  unsigned int checksum; 
  unsigned int size; 
  int file; 
  int i;

  checksum = 0;
  if ( argc == 2 )
  {
    file = open(argv[1], 2);
    if ( file >= 0 )
    {
      lseek(file, 0, 0);
      size = lseek(file, 0, 2);
      lseek(file, 0, 0);
      printf("size = %d \n\r", size);
      sizeOfBuffer = 16 * ((size + 115) / 0x10);
      buffer = alloca(sizeOfBuffer);
      memset(&buffer, 0, size + 100);
      read(file, buffer, size);
      for ( i = 0; i < (signed int)size; ++i )
        checksum += *((char *)buffer + i);
      lseek(file, 0, 2);
      write(file, &checksum, 4u);
      printf("checksum %x\n\r", checksum);
      close(file);
      isGood = 1;
    }
    else
    {
      resultMain = -2;
      isGood = 0;
    }
  }
  else
  {
	printf("Usage: checksum.exe <file_name>\n");
    resultMain = -1;
    isGood = 0;
  }
  if ( isGood == 1 )
    resultMain = 0;
  return resultMain;
}

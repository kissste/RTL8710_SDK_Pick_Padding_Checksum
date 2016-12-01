#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

int main(int argc, const char *argv[], const char *envp[])
{
  int result; 
  size_t originalSize; 
  size_t paddedSize; 
  char *unit; 
  void *buffer; 
  FILE *file; 
  uint8_t paddingData; 
  size_t bufferSize; 

  if ( argc == 4 )
  {
	  bufferSize = (signed int)strtod(argv[1], &unit);
	  paddingData = (signed int)strtod(argv[2], NULL);
	  printf("total %d %s, padding data %x, name %s\n", bufferSize, unit, paddingData, argv[3]);
	  if ( *unit )
	  {
		if ( *unit != 75 && *unit != 107 )
		{
		  if ( *unit != 77 && *unit != 109 )
		  {
			if ( *unit != 71 && *unit != 103 )
			  return printf("unit %s is Not valid\n", unit);
			bufferSize <<= 20;
		  }
		  else
		  {
			bufferSize <<= 20;
		  }
		}
		else
		{
		  bufferSize <<= 10;
		}
	  }
	  file = fopen(argv[3], "r+b");
	  if ( file )
	  {
		buffer = malloc(bufferSize);
		memset(buffer, (char)paddingData, bufferSize);
		originalSize = fread(buffer, 1u, bufferSize, file);
		printf("Original size %d\n", originalSize);
		fseek(file, 0, 0);
		paddedSize = fwrite(buffer, 1u, bufferSize, file);
		printf("Padding  size %d\n", paddedSize);
		free(buffer);
		result = fclose(file);
		return result;
	  } else 
	  {
		  return 0;
	  }
  } else {
	  printf("Usage: padding.exe <size> <character> <file_name>\n");
	  result -1;
  }
}

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef unsigned long DWORD;
typedef unsigned short WORD;
#define LOWORD(a) ((WORD)(a))
#define HIWORD(a) ((WORD)(((DWORD)(a) >> 16) & 0xFFFF))

int main(int argc, const char *argv[], const char *envp[])
{
  int result; 
  int32_t address; 
  int32_t imageStartAddress;
  void *buffer; 
  int32_t size; 
  FILE *outputFile;
  FILE *inputFile; 
  int32_t startAddress; 
  const char *outputFileName; 
  const char *inputFileName; 
  bool isSig; 
  bool isRaw; 
  int32_t resetOffset;
  int32_t endAddress;
  signed int isHead; 

  isHead = 6;
  if ( strstr(argv[5], "head") )
    isHead = 7;
  if ( argc == isHead )
  {
    inputFileName = argv[3];
    outputFileName = argv[4];
    startAddress = strtol(argv[1], 0, 0);
    endAddress = strtol(argv[2], 0, 0);
    address = startAddress<<16;
    address &= 0xFFFF0000;
    resetOffset = address;
    if ( strstr(argv[5], "reset_offset") )
      resetOffset = startAddress;
    isRaw = strstr(argv[5], "raw") != 0;
    isSig = strstr(argv[5], "sig") != 0;
    printf("b:%d s:%d e:%d\n", resetOffset, startAddress, endAddress);
    inputFile = fopen(inputFileName, "rb");
    if ( inputFile )
    {
      outputFile = fopen(outputFileName, "wb");
      if ( outputFile )
      {
        fseek(inputFile, 0, 2);
        size = ftell(inputFile);
        printf("size %d\n", size);
        buffer = malloc(size);
        if ( buffer )
        {
          if ( !endAddress )
            endAddress = size + resetOffset;
          if ( endAddress - startAddress != -1 )
          {
            fseek(inputFile, startAddress - resetOffset, 0);
            fread(buffer, endAddress - startAddress, 1u, inputFile);
            if ( !isRaw )
            {
			  int32_t dword_40301C = 0xFFFFFFFF;
			  int32_t dword_403020 = 0xFFFFFFFF;
              if ( !strcmp(argv[5], "head") )
              {
                imageStartAddress = strtol(argv[6], 0, 0);
                printf("append fw head %x\n", imageStartAddress);
				int32_t fw_head = 0x99999696;
                fwrite(&fw_head, 4u, 4u, outputFile);
                dword_40301C = imageStartAddress / 1024 | 0xFFFF0000;
              }
              else if ( isSig )
              {
                dword_40301C = 892940600;
                dword_403020 = 825308984;
              }
              else
              {
                dword_40301C = -1;
                dword_403020 = -1;
              }
              int32_t seg_head = endAddress - startAddress;
              int32_t dword_403018 = startAddress;
              fwrite(&seg_head, 4u, 4u, outputFile);
            }
            fwrite(buffer, endAddress - startAddress, 1u, outputFile);
          }
          printf("copy size %d\n", endAddress - startAddress);
          fclose(inputFile);
          fclose(outputFile);
          free(buffer);
          result = 0;
        }
        else
        {
          result = -4;
        }
      }
      else
      {
        result = -3;
      }
    }
    else
    {
      result = -2;
    }
  }
  else
  {
    printf("Usage: pick.exe <start addr> <end addr> <input name> <output name> <body[+offset] , head [image2_start]>\n");
    result = -1;
  }
  return result;
}


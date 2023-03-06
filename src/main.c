#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define ArrayCount(arr) (sizeof(arr) / sizeof(arr[0]))

#define KB(size) (1024 * size)
#define MB(size) (1024 * KB(size))
#define GB(size) (1024 * MB(size))
#define TB(size) (1024 * GB(size))

#define CreateTempAlloc(size)         \
  tempAllocPointer = malloc(size);    \
  nextTempPointer = tempAllocPointer; \
  tempAllocSize = size
#define TempAlloc(size)                                               \
  nextTempPointer;                                                    \
  assert(nextTempPointer - tempAllocPointer + size <= tempAllocSize); \
  nextTempPointer += size
#define FreeTempAlloc() nextTempPointer = tempAllocPointer
#define PrintTotalTempAlloc() printf("TempAlloc current size: %d", (int)(nextTempPointer - tempAllocPointer))
#define DestroyTempAlloc() \
  free(tempAllocPointer);  \
  tempAllocSize = 0;

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;

static char *tempAllocPointer = NULL;
static char *nextTempPointer = NULL;
static size_t tempAllocSize = 0;

static u8 *ReadFile(const char *fileName, size_t *fileSize)
{
  u8 *result = NULL;
  FILE *file = NULL;
  size_t size = 0;
  fopen_s(&file, fileName, "r");
  if (file)
  {
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    result = malloc(size + 1);
    if (result)
    {
      size_t i;
      u32 c = getc(file);
      for (i = 0; i < size && !(c == '\0' || c == EOF); i++, c = getc(file))
      {
        result[i] = (u8)c;
      }
      result[i] = '\0';
    }

    fclose(file);
  }
  *fileSize = size;
  return result;
}

static char *GetOpName(u32 opCode)
{
  switch (opCode)
  {
  case 0b00100010:
    return "mov";
    break;

  default:
    return NULL;
    break;
  }
}

static char *GetRegName(u32 regNumber)
{
  static char *registers[] = {
      "al", // 0b00000000
      "ax", // 0b00000001
      "cl", // 0b00000010
      "cx", // 0b00000011
      "dl", // 0b00000100
      "dx", // 0b00000101
      "bl", // 0b00000110
      "bx", // 0b00000111
      "ah", // 0b00001000
      "sp", // 0b00001001
      "ch", // 0b00001010
      "bp", // 0b00001011
      "dh", // 0b00001100
      "si", // 0b00001101
      "bh", // 0b00001111
      "di", // 0b00001111
  };

  if (regNumber > ArrayCount(registers))
  {
    return NULL;
  }
  return registers[regNumber];
}

int main(int argc, char **argv)
{
  // CreateTempAlloc(MB(16));

  if (argc > 1)
  {
    size_t fileSize = 0;
    u8 *fileContents = ReadFile(argv[1], &fileSize);
    if (fileContents == NULL)
    {
      printf("Couldn't open file: %s", argv[1]);
    }
    else
    {
      for (size_t i = 0; i < fileSize; i += 2)
      {

        u32 byte1 = *(fileContents + i);
        u32 byte2 = *(fileContents + i + 1);

        char *opName = GetOpName(byte1 >> 2);
        if (opName == NULL)
        {
          printf("Couldn't find op for opCode: %d", byte1);
        }
        else
        {
          printf("%s ", opName);
        }

        u32 D = byte1 >> 1 & 1;
        u32 W = byte1 & 1;
        u32 MOD = byte2 >> 6;
        u32 REG = byte2 >> 3 & ~(-1 << 3);
        u32 RM = byte2 & ~(-1 << 3);
        char *regName = GetRegName(REG << 1 | W);
        char *rmName = GetRegName(RM << 1 | W);
        if (D)
        {
          printf("%s, ", regName);
          printf("%s\n", rmName);
        }
        else
        {
          printf("%s, ", rmName);
          printf("%s\n", regName);
        }
      }
    }
  }

  // DestroyTempAlloc();
  return 0;
}

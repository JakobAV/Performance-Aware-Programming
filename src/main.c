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

#define MOV 0b10001000
#define MOV_BITS 0b11111100
#define MOV_I 0b11000110
#define MOV_I_BITS 0b11111110
#define MOV_IR 0b10110000
#define MOV_IR_BITS 0b11110000
#define MOV_MEM_AC 0b10100000
#define MOV_MEM_AC_BITS 0b11111110
#define MOV_AC_MEM 0b10100010
#define MOV_AC_MEM_BITS 0b11111110

#define OpCodeIs(bits, pattern) ((bits & (pattern##_BITS)) == pattern)

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
      for (i = 0; i < size; i++, c = getc(file))
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
  case MOV:
  case MOV_I:
  case MOV_IR:
  case MOV_MEM_AC:
  case MOV_AC_MEM:
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

static char *GetRmName(u32 rmNumber)
{
  static char *rmText[] = {
      "bx + si", // 0b00000000
      "bx + di", // 0b00000001
      "bp + si", // 0b00000010
      "bp + di", // 0b00000011
      "si",      // 0b00000100
      "di",      // 0b00000101
      "bp",      // 0b00000110
      "bx",      // 0b00000111
  };

  if (rmNumber > ArrayCount(rmText))
  {
    return NULL;
  }
  return rmText[rmNumber];
}

void OutputOpCode(char *opCodeName)
{

  if (opCodeName == NULL)
  {
    printf("Couldn't find op for opCode");
  }
  else
  {
    printf("%s ", opCodeName);
  }
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
      printf("; %s\n", argv[1]);
      printf("bits 16\n\n");
      for (size_t i = 0; i < fileSize; i += 2)
      {

        u32 byte1 = *(fileContents + i);
        u32 byte2 = *(fileContents + i + 1);

        char *opName = NULL;
        u32 D = 0;
        u32 W = 0;
        u32 MOD = 0;
        u32 REG = 0;
        u32 RM = 0;
        if (OpCodeIs(byte1, MOV))
        {
          opName = GetOpName(MOV);
          OutputOpCode(opName);
          D = byte1 >> 1 & 1;
          W = byte1 & 1;
          MOD = byte2 >> 6;
          REG = byte2 >> 3 & ~(-1 << 3);
          RM = byte2 & ~(-1 << 3);

          char *regName = GetRegName(REG << 1 | W);
          if ((MOD & (1 << 1)) > 0)
          {
            if ((MOD & 1) > 0)
            {
              // Register mode
              char *rmName = GetRegName(RM << 1 | W);
              if (D)
              {
                printf("%s, ", regName);
                printf("%s", rmName);
              }
              else
              {
                printf("%s, ", rmName);
                printf("%s", regName);
              }
            }
            else
            {
              // Memory mode, 16-bit
              char *rmName = GetRmName(RM);
              i += 2;
              u32 byte3 = *(fileContents + i);
              u32 byte4 = *(fileContents + i + 1);
              if (D)
              {
                printf("%s, ", regName);
              }
              printf("[");
              printf("%s", rmName);
              printf(" + ");
              printf("%d", (byte4 << 8) | byte3);
              if (!D)
              {
                printf("], ");
                printf("%s", regName);
              }
              else
              {
                printf("]");
              }
            }
          }
          else
          {
            if ((MOD & 1) > 0)
            {
              // Memory mode, 8-bit
              char *rmName = GetRmName(RM);
              ++i;
              u32 byte3 = *(fileContents + i + 1);
              if (D)
              {
                printf("%s, ", regName);
              }
              printf("[");
              printf("%s", rmName);
              printf(" + ");
              printf("%d", byte3);
              if (!D)
              {
                printf("], ");
                printf("%s", regName);
              }
              else
              {
                printf("]");
              }
            }
            else
            {
              // Memory mode, none unless RM 110
              char *rmName = GetRmName(RM);
              if (D)
              {
                printf("%s, ", regName);
              }
              printf("[");
              printf("%s", rmName);
              if (RM == (1 << 3 | 1 << 2))
              {
                ++i;
                u32 byte3 = *(fileContents + i + 1);
                printf(" + ");
                printf("%d", byte3);
              }
              if (!D)
              {
                printf("], ");
                printf("%s", regName);
              }
              else
              {
                printf("]");
              }
            }
          }
        }
        else if (OpCodeIs(byte1, MOV_I))
        {
          opName = GetOpName(MOV_I);
          OutputOpCode(opName);
          W = byte1 & 1;
          MOD = byte2 >> 6;
          RM = byte2 & ~(-1 << 3);

          char *regName = GetRegName(RM << 1 | W);
          if ((MOD & (1 << 1)) > 0)
          {
            // Memory mode, 16-bit
            char *rmName = GetRmName(RM);
            i += 2;
            u32 byte3 = *(fileContents + i);
            u32 byte4 = *(fileContents + i + 1);
            u32 byte5 = *(fileContents + i + 2);

            printf("%s, ", regName);
            printf("[");
            printf("%s", rmName);
            printf(" + ");
            printf("%d", (byte4 << 8) | byte3);
            printf("]");
          }
          else
          {
            if ((MOD & 1) > 0)
            {
              // Memory mode, 8-bit
              char *rmName = GetRmName(RM);
              ++i;
              u32 byte3 = *(fileContents + i + 1);
              if (D)
              {
                printf("%s, ", regName);
              }
              printf("[");
              printf("%s", rmName);
              printf(" + ");
              printf("%d", byte3);
              if (!D)
              {
                printf("], ");
                printf("%s", regName);
              }
              else
              {
                printf("]");
              }
            }
            else
            {
              // Memory mode, none unless RM 110
              char *rmName = GetRmName(RM);
              if (D)
              {
                printf("%s, ", regName);
              }
              printf("[");
              printf("%s", rmName);
              if (RM == (1 << 3 | 1 << 2))
              {
                ++i;
                u32 byte3 = *(fileContents + i + 1);
                printf(" + ");
                printf("%d", byte3);
              }
              if (!D)
              {
                printf("], ");
                printf("%s", regName);
              }
              else
              {
                printf("]");
              }
            }
          }
        }
        else if (OpCodeIs(byte1, MOV_IR))
        {
          opName = GetOpName(MOV_IR);
          OutputOpCode(opName);
          W = byte1 >> 3 & 1;
          REG = byte1 & ~(-1 << 3);
          char *regName = GetRegName(REG << 1 | W);
          // Memory mode, 16-bit
          char *rmName = GetRmName(RM);
          printf("%s, ", regName);
          if (W > 0)
          {
            i += 1;
            u32 byte3 = *(fileContents + i + 1);
            printf("%d", byte2 | (byte3 << 8));
          }
          else
          {
            printf("%d", byte2);
          }
        }
        else if (OpCodeIs(byte1, MOV_MEM_AC))
        {
          opName = GetOpName(MOV_MEM_AC);
          OutputOpCode(opName);
          W = byte1 & 1;
        }
        else if (OpCodeIs(byte1, MOV_AC_MEM))
        {
          opName = GetOpName(MOV_AC_MEM);
          OutputOpCode(opName);
          W = byte1 & 1;
        }
        printf("\n");
      }
    }
  }

  // DestroyTempAlloc();
  return 0;
}

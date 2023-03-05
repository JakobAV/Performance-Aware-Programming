#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
static char *tempAllocPointer = NULL;
static char *nextTempPointer = NULL;
static size_t tempAllocSize = 0;

static char *ReadFile(char *fileName, size_t* fileSize)
{
  char *result = NULL;
  FILE *file = NULL;
  fileSize = 0;
  fopen_s(&file, fileName, "r");
  if (file)
  {
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    result = malloc(fileSize + 1);
    if (result)
    {
      size_t i;
      int c = getc(file);
      for (i = 0; i < fileSize && !(c == '\0' || c == EOF); i++, c = getc(file))
      {
        result[i] = (char)c;
      }
      result[i] = '\0';
    }

    fclose(file);
  }

  return result;
}

static char *GetOpName(char opCode)
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

int main(int argc, char **argv)
{
  // CreateTempAlloc(MB(16));

  if (argc > 1)
  {
    size_t fileSize = 0;
    char *fileContents = ReadFile(argv[1], &fileSize);
    if (fileContents == NULL)
    {
      printf("Couldn't open file: %s", argv[1]);
    }
    else
    {
      char byte1 = *fileContents;
      char byte2 = *fileContents + 1;

      char* opName = GetOpName(byte1 >> 2);
      if(opName == NULL)
      {
        printf("Couldn't find op for opCode: %d", byte1);
      }
      else
      {
        printf("%s ", opName);
      }

      char regName = NULL;
      int D = byte1 >> 1 & 1;
      int W = byte1 & 1;
      int MOD = byte2 >> 6;
      int REG = byte2 >> 3 & (-1 << 3);
      int RM = byte2 & (-1 << 3);
    }
  }

  // DestroyTempAlloc();
  return 0;
}

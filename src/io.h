#ifndef IO_H_
#define IO_H_

#include "common.h"

typedef struct File {
  char* fullpath; //< will be copied
  char* content;
  u64 len;
} File;

File* read_file(const char* path);

#endif // IO_H_

#include "io.h"

File* read_file(const char* path) {
  File* file = (File*) malloc(sizeof(File)); 
  file->fullpath = (char*) malloc(strlen(path));

  strcpy(file->fullpath, path);

  FILE* f = fopen(path, "r");

  fseek(f, 0, SEEK_END);

  u64 size = ftell(f);

  rewind(f);

  file->content= malloc(size + 1);
  
  fread(file->content, 1, size, f);
  
  file->content[size] = 0;
  file->len = size;

  return file;
}

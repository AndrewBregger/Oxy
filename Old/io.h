#ifndef IO_H_
#define IO_H_

#include "common.h"
#include <vector>

struct File {
  std::string filename;
  std::string path;
  std::string content;
  std::vector<std::pair<u32, u32>> lines;
};

File load_file(const std::string& path);

std::string get_line(File* file, u32 line);



#endif

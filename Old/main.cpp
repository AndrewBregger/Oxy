#include "common.h"
#include "io.h"
#include "pratt.h"

int main(int argc, const char** argv) {
  std::vector<std::string> args;
  for(int i = 0; i < argc; ++i)
    args.emplace_back(argv[i]);
  return pratt_main(args);
}
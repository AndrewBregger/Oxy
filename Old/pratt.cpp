#include "pratt.h"
#include "io.h"
#include "parser.h"

bool validate_input(const std::vector<std::string>& args) {
  if(args.size() < 2) {
    printf("Error: pratt [file.pr]\n");
    return false;
  }
  return true;
}

void compile_root(File* file);

int pratt_main(const std::vector<std::string>& args) {
  if(validate_input(args)) {
    File root = load_file(args[1]);

    compile_root(&root);
  }
  return 0;
}

void compile_root(File* file) {
  // parse
  parse_test(file);
  // type check
  // generate object file
}

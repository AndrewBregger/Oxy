#include "oxy.h"
#include "io.h"
#include "common.h"
#include "lex.h"

bool validate_input(u32 num, const char* const* args) {
  if(num < 2) {
    printf("Error: pratt [file.pr]\n");
    return false;
  }
  return true;
}

void compile_root(File* file);

int oxy_main(u32 num, const char* const* argv) {
  if(validate_input(num, argv)) {
    File* root = read_file(argv[1]);
    
    compile_root(root);
  }
  else
    return 1;

  return 0;
}

void compile_root(File* file) {
  //buf_test();
  //map_test();
  // parse
  //scan_test(file);
  parse_test(file);
  // parse_test(file);
  // type check
  // generate object file
}

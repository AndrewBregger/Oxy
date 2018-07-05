#include "oxy.h"
#include "io.h"
#include "common.h"
#include "parser.h"
#include "ast.h"
#include "lex.h"
#include "print.h"
#include "checker.h"

StringTable* table = NULL;

bool validate_input(u32 num, const char* const* args) {
  if(num < 2) {
    printf("Error: oxc [file.oxy]\n");
    return false;
  }
  return true;
}

void compile_root(File* file);

int oxy_main(u32 num, const char* const* argv) {
  if(validate_input(num, argv)) {
    File* root = read_file(argv[1]);

    table = (StringTable*) malloc(sizeof(StringTable));
    *table = create_table(TABLE_START);

    // checker_test();

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
  // parse_test(file);
  AstFile* ast = parse_file(file, table);
  for(u32 i = 0; i < ast_num_items(ast); ++i)
    print_item(ast->items[i]);

  // Checker checker = new_checker(table);

  // type check
  // if(resolve_file(&checker, ast)) {
    // generate object file
  // }
}

StringTable* get_string_table() {
  return table;
}
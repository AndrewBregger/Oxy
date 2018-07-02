#ifndef PARSER_H_
#define PARSER_H_

#include "io.h"

typedef struct AstFile AstFile;


AstFile* parse_file(File* file, StringTable* table);

void parse_test(File* file);

#endif

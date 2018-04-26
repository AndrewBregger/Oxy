#ifndef PARSER_H_
#define PARSER_H_

#include "io.h"

typedef struct Item** ItemSet;

ItemSet parse_file(File* file);

void parse_test(File* file);

#endif

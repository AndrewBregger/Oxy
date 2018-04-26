#ifndef PARSER_H_
#define PARSER_H_

#include <cctype>

#include "common.h"

#include "io.h"
#include "token.h"
#include "lex.h"
#include "ast.h"

ItemSet parse_file(File* root);

void parse_test(File* file);



#endif

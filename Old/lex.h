#ifndef LEX_H_
#define LEX_H_

#include "common.h"
#include "io.h"
#include "token.h"


std::vector<Token> scan_tokens(File* file);

void scanner_test(File* file);


#endif

#ifndef LEX_H_
#define LEX_H_

#include "io.h"
typedef struct Token Token;

Token* get_tokens(File* file, u32* num, StringTable* table);

void scan_test(File* file);

#endif

#ifndef REPORT_H_
#define REPORT_H_

#include "ast.h"
#include "io.h"

void syntax_error(SourceLoc loc, const char* msg, ...);

void compiler_error(const char* msg, ...);

void scan_error(File* file, u32 line, u32 column, const char* msg, ...);

void check_error(SourceLoc, const char* msg, ...);

#endif

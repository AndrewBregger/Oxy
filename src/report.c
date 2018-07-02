#include "report.h"
#include <stdarg.h>

void vprint(const char* msg, va_list va) {
  vprintf(msg, va);
}

void print_location(File* file, u32 line, u32 column, const char* type) {
  printf("%s:%u:%u %s ", file->fullpath, line, column, type);
}

// this will eventually handle print the source code line, showing where the error occured.

void syntax_error(SourceLoc loc, const char* msg, ...) {
  print_location(loc.file, loc.line, loc.column, "Error:");
  va_list va;
  va_start(va, msg);
  vprint(msg, va);
  va_end(va);
}

void compiler_error(const char* msg, ...) {
  printf("Internal Compiler Error: ");
  va_list va;
  va_start(va, msg);
  vprint(msg, va);
  va_end(va);
}

void scan_error(File* file, u32 line, u32 column, const char* msg, ...) {
  print_location(file, line, column, "Error:");
  va_list va;
  va_start(va, msg);
  vprint(msg, va);
  va_end(va);
}

void check_error(SourceLoc loc, const char* msg, ...) {
  print_location(loc.file, loc.line, loc.column, "Error:");
  va_list va;
  va_start(va, msg);
  vprint(msg, va);
  va_end(va);
}

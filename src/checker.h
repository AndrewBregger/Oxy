#ifndef CHECKER_H_
#define CHECKER_H_

// typedef struct Scope Scope;
// typedef struct Symbol Symbol;
// typedef struct File File;

#include "common.h"
#include "scope.h"
#include "entity.h"
#include "ast.h"
#include "queue.h"

// NOTES:
// - How to handles built in functions?
// - What functions should be built in?
// - How should overloaded function be handled?

typedef struct Checker {
	Type** types;

	AstFile** files;
	Scope* global_scope;
	Scope* current_scope;

	StringTable* string_table;
	Queue queue;
} Checker;

Checker new_checker(StringTable* table);

bool resolve_file(Checker* chekcer, AstFile* file);

#endif

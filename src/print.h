#ifndef PRINT_H_
#define PRINT_H_

#include "common.h"
#include "token.h"
#include "ast.h"

struct Entity;

void print_token(Token* token);

void print_ident(Ident* ident);

void print_literal(Token* token);

void print_expr(Expr* expr);
//
void print_item(Item* item);
//
void print_stmt(Stmt* stmt);
//
//void print_typespec(TypeSpec* spec);
//
void print_pattern(Pattern* pattern);
//
//void print_entity(Entity* entity);

void note(const char* msg, ...);

#endif

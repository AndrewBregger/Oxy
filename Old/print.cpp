#include "print.h"


void print_token(Token* token) {
  printf("Token(%s, %d, %llu, %llu, %llu)\n", get_token_c_str(token), token->type, token->line, token->column, token->span);
}

// void print_pattern(Pattern* pattern) {}

void print_expr(Expr* expr, int i);
void print_item(Item* item, int i);
void print_stmt(Stmt* stmt, int i);
void print_typespec(TypeSpec* spec, int i);
void print_pattern(Pattern* pattern, int i);


std::string indent(int i) {
  std::string temp;
  while(i--)
    temp += "  ";
  return temp;
}

void print_stmt(Stmt* stmt, int i);

void print_token(Token* token, int i) {
  printf("%s", indent(i).c_str());
  print_token(token);
}

void print_ident(Ident* ident, int i) {
  printf("%sIdent(%s)\n", indent(i), ident->value.c_str());
}

void print_literal(Token token, int i) {
  print_token(&token, i);
}

void print_expr(Expr* expr, int i);
void print_stmt(Stmt* expr, int i);
void print_Item(Item* expr, int i);
//void print_expr(Expr* expr, int i);

template <typename T>
inline void print_expr_list(T e, u32 num, u32 i) {
  for(u32 i = 0; i < num; ++i)
    print_expr(e[i], i);
}

template <typename T>
inline void print_stmt_list(T e, u32 num, u32 i) {
  for(u32 i = 0; i < num; ++i)
    print_stmt(e[i], i);
}

template <typename T>
inline void print_item_list(T e, u32 num, u32 i) {
  for(u32 i = 0; i < num; ++i)
    print_item(e[i], i);
}

void print_expr(Expr* expr, int i) {
  printf("%s%s\n", indent(i).c_str(), expr_string(expr->kind));
  switch(expr->kind) {
    case Name: {
      print_ident(expr->name, i + 1);
    } break;
    case Literal: {
      print_literal(expr->literal, i + 1);
    } break;
    case StructLiteral: {
      print_typespec(expr->struct_lit.name, i + 1);
      print_expr_list(expr->struct_lit.members, expr->struct_lit.num_members, i + 1);
    } break;
    case CompoundLiteral: {
      print_expr_list(expr->compound_lit.members, expr->compound_lit.num_members, i + 1);
    } break;
    case Unary: {
      print_token(&expr->unary.op, i + 1);
      print_expr(expr->unary.expr, i + 1);
    } break;
    case Binary: {
      print_token(&expr->binary.op, i + 1);
      print_expr(expr->binary.lhs, i + 1);
      print_expr(expr->binary.rhs, i + 1);
    } break;
    case FnCall: {
      print_expr(expr->fncall.name, i + 1);
      print_expr_list(expr->fncall.actuals, expr->fncall.num_actuals, i + 1);
    } break;
    case Field: {
      print_expr(expr->field.operand, i + 1);
      print_ident(expr->field.name, i + 1);
    } break;
    case DotFnCall: {
      print_expr(expr->dotcall.operand, i + 1);
      print_ident(expr->dotcall.name, i + 1);
      print_expr_list(expr->dotcall.actuals, expr->dotcall.num_actuals, i + 1);
    } break;
    case If: {
      print_expr(expr->if_expr.cond, i + 1);
      print_expr(expr->if_expr.body, i + 1);
      print_expr(expr->if_expr.else_if, i + 1);
    } break;
    case While: {
      print_expr(expr->while_expr.cond, i + 1);
      print_expr(expr->while_expr.body, i + 1);
    } break;
    case For: {
      print_expr(expr->for_expr.cond, i + 1);
      print_expr(expr->for_expr.body, i + 1);
    } break;
    case Return: {
      print_expr_list(expr->return_expr.exprs, expr->return_expr.num_exprs, i + 1);
    } break;
    case Break: {
      print_token(&expr->break_expr, i + 1);
    } break;
    case Continue: {
      print_token(&expr->continue_expr, i + 1);
    } break;
    case Block: {
      print_stmt_list(expr->block.stmts, expr->block.num_stmts, i + 1);
    } break;
    case Binding: {
      print_expr(expr->binding.name, i + 1);
      print_expr(expr->binding.binding, i + 1);
    } break;
    case In: {
      print_expr(expr->in.in, i + 1);
      print_expr(expr->in.expr, i + 1);
    } break;
    case Tuple: {
      print_expr_list(expr->tuple.elems, expr->tuple.num_elems, i + 1);
    } break;
    case PatternExpr: {
    } break;
  }
}

void print_item(Item* item, int i) {
  printf("%s%s\n", indent(i).c_str(), item_string(item->kind));
  switch(item->kind) {
    case ItemLocal: {

    } break;
    case ItemFunction: {

    } break;
    case ItemStruct: {

    } break;
    case ItemTupleStruct: {

    } break;
    case ItemEnum: {

    } break;
    case ItemUse: {

    } break;
    case ItemModule: {

    } break;
    case ItemField: {

    } break;
  }
}

void print_stmt(Stmt* stmt, int i) {
  printf("%s%s\n", indent(i).c_str(), stmt_string(stmt->kind));
  switch(stmt->kind) {
    case ExprStmt:;
    case ItemStmt:;
  }
}

void print_typespec(TypeSpec* spec, int i) {
  printf("%s%s\n", indent(i).c_str(), typespec_string(spec->kind));
  switch(spec->kind) {
    case TypeSpecNone: {

    } break;
    case TypeSpecName: {

    } break;
    case TypeSpecFunc: {

    } break;
    case TypeSpecArray: {

    } break;
    case TypeSpecPtr: {

    } break;
    case TypeSpecRef: {

    } break;
    case TypeSpecMap: {

    } break;
    case TypeSpecTuple: {

    } break;
  }
}

void print_pattern(Pattern* pattern) {

}



void print_expr(Expr* expr) { print_expr(expr, 0); }
void print_item(Item* item) { print_item(item, 0); }
void print_stmt(Stmt* stmt) { print_stmt(stmt, 0); }
void print_typespec(TypeSpec* spec) { print_typespec(spec, 0); }
void print_ident(Ident* ident) { print_ident(ident, 0); }
void print_literal(Token token) { print_literal(token, 0); }

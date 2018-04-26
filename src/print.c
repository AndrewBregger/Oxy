#include "print.h"

void print_expr_(Expr* expr, int i);
void print_item_(Item* item, int i);
void print_stmt_(Stmt* stmt, int i);
void print_typespec_(TypeSpec* spec, int i);
void print_pattern_(Pattern* pattern, int i);

const char* indent(int i) {
  if(i == 0)
    return "";

  char* buffer = NULL;
  while(--i >= 0)
    buf_push(buffer, ' ');
  return buffer;
}

// void print_stmt(Stmt* stmt, int i);

void print_token(Token* token) {
  printf("Token(%s, %d, %llu, %llu, %llu)\n", get_token_string(token), token->type, token->line, token->column, token->span);
}

void print_token_(Token* token, int i) {
  printf("%s", indent(i));
  print_token(token);
}

void print_literal_(Token* token, int i) {
  print_token_(token, i);
}

void print_ident_(Ident* ident, int i) {
  printf("%sIdent(%s)\n", indent(i), ident->value);
}

void print_expr_list(Expr** e, u32 num, u32 i) {
  for(u32 i = 0; i < num; ++i)
    print_expr_(e[i], i);
}

void print_stmt_list(Stmt** e, u32 num, u32 i) {
  for(u32 i = 0; i < num; ++i)
    print_stmt_(e[i], i);
}

void print_item_list(Item** e, u32 num, u32 i) {
  for(u32 i = 0; i < num; ++i)
    print_item_(e[i], i);
}


void print_expr_(Expr* expr, int i) {
  printf("%s%s\n", indent(i), expr_string(expr->kind));
  switch(expr->kind) {
    case Name: {
      print_ident_(expr->name, i + 1);
    } break;
    case Literal: {
      print_literal_(&expr->literal, i + 1);
    } break;
    case StructLiteral: {
      print_typespec_(expr->struct_lit.name, i + 1);
      print_expr_list(expr->struct_lit.members, expr->struct_lit.num_members, i + 1);
    } break;
    case CompoundLiteral: {
      print_expr_list(expr->compound_lit.members, expr->compound_lit.num_members, i + 1);
    } break;
    case Unary: {
      print_token_(&expr->unary.op, i + 1);
      print_expr_(expr->unary.expr, i + 1);
    } break;
    case Binary: {
      print_token_(&expr->binary.op, i + 1);
      print_expr_(expr->binary.lhs, i + 1);
      print_expr_(expr->binary.rhs, i + 1);
    } break;
    case FnCall: {
      print_expr_(expr->fncall.name, i + 1);
      print_expr_list(expr->fncall.actuals, expr->fncall.num_actuals, i + 1);
    } break;
    case Field: {
      print_expr_(expr->field.operand, i + 1);
      print_ident_(expr->field.name, i + 1);
    } break;
    case DotFnCall: {
      print_expr_(expr->dotcall.operand, i + 1);
      print_ident_(expr->dotcall.name, i + 1);
      print_expr_list(expr->dotcall.actuals, expr->dotcall.num_actuals, i + 1);
    } break;
    case If: {
      print_expr_(expr->if_expr.cond, i + 1);
      print_expr_(expr->if_expr.body, i + 1);
      print_expr_(expr->if_expr.else_if, i + 1);
    } break;
    case While: {
      print_expr_(expr->while_expr.cond, i + 1);
      print_expr_(expr->while_expr.body, i + 1);
    } break;
    case For: {
      print_expr_(expr->for_expr.cond, i + 1);
      print_expr_(expr->for_expr.body, i + 1);
    } break;
    case Return: {
      print_expr_list(expr->return_expr.exprs, expr->return_expr.num_exprs, i + 1);
    } break;
    case Break: {
      print_token_(&expr->break_expr, i + 1);
    } break;
    case Continue: {
      print_token_(&expr->continue_expr, i + 1);
    } break;
    case Block: {
      print_stmt_list(expr->block.stmts, expr->block.num_stmts, i + 1);
    } break;
    case Binding: {
      print_expr_(expr->binding.name, i + 1);
      print_expr_(expr->binding.binding, i + 1);
    } break;
    case In: {
      print_expr_(expr->in.in, i + 1);
      print_expr_(expr->in.expr, i + 1);
    } break;
    case Tuple: {
      print_expr_list(expr->tuple.elems, expr->tuple.num_elems, i + 1);
    } break;
    case PatternExpr: {
    } break;
  }
}

void print_item_(Item* item, int i) {
  printf("%s%s\n", indent(i), item_string(item->kind));
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

void print_stmt_(Stmt* stmt, int i) {
  printf("%s%s\n", indent(i), stmt_string(stmt->kind));
  switch(stmt->kind) {
    case ExprStmt:;
    case ItemStmt:;
  }
}


void print_typespec_(TypeSpec* spec, int i) {
  printf("%s%s\n", indent(i), typespec_string(spec->kind));
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

void print_literal(Token* token) { print_literal_(token, 0); }
void print_expr(Expr* expr) { print_expr_(expr, 0); }

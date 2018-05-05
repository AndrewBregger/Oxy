#include "print.h"

void print_expr_(Expr* expr, int i);
void print_item_(Item* item, int i);
void print_stmt_(Stmt* stmt, int i);
void print_typespec_(TypeSpec* spec, int i);
void print_pattern_(Pattern* pattern, int i);
void print_clause_(Clause* clause, int i);

const char* indent(int i) {
  if(i == 0)
    return "";

  // tabbing is two spaces
  i *= 2;
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

void print_expr_list(Expr** e, u32 num, u32 in) {
  for(u32 i = 0; i < num; ++i)
    print_expr_(e[i], in);
}

void print_stmt_list(Stmt** e, u32 num, u32 in) {
  for(u32 i = 0; i < num; ++i)
    print_stmt_(e[i], in);
}

void print_item_list(Item** e, u32 num, u32 in) {
  for(u32 i = 0; i < num; ++i)
    print_item_(e[i], in);
}

void print_pattern_list(Pattern** p, u32 num, u32 in) {
  for(u32 i = 0; i < num; ++i)
    print_pattern_(p[i], in);
}

void print_expr_(Expr* expr, int i) {
  if(!expr) return;
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
      print_expr_(expr->dotcall.name, i + 1);
      print_expr_list(expr->dotcall.actuals, expr->dotcall.num_actuals, i + 1);
    } break;
    case If: {
      print_expr_(expr->if_expr.cond, i + 1);
      print_expr_(expr->if_expr.body, i + 1);
      print_expr_(expr->if_expr.else_if, i + 1);
    } break;
    case MatchIf: {
      print_expr_(expr->matchif_expr.cond, i + 1);
      for(int x = 0; x < expr->matchif_expr.num_body; ++x)
        print_clause_(expr->matchif_expr.body[i], i + 1);
    } break;
    case While: {
      print_expr_(expr->while_expr.cond, i + 1);
      print_expr_(expr->while_expr.body, i + 1);
    } break;
    case For: {
      print_pattern_(expr->for_expr.pat, i + 1);
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
    case Assignment: {
      print_token_(&expr->assign.op, i + 1);
      print_expr_(expr->assign.variable, i + 1);
      print_expr_(expr->assign.value, i + 1);
    } break;
  }
}

void print_item_(Item* item, int i) {
  if(!item) return;
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
  if(!stmt) return;
  printf("%s%s\n", indent(i), stmt_string(stmt->kind));
  switch(stmt->kind) {
    case ExprStmt:
      print_expr_(stmt->expr, i + 1);
      break;
    case SemiStmt:
      print_expr_(stmt->semi, i + 1);
      break;
    case ItemStmt:;
      print_item_(stmt->item, i + 1);
      break;

  }
}


void print_typespec_(TypeSpec* spec, int i) {
  printf("%s%s\n", indent(i), typespec_string(spec->kind));
  switch(spec->kind) {
    case TypeSpecNone: {
    } break;
    case TypeSpecName: {
      print_ident_(spec->name.name, i + 1);
    } break;
    case TypeSpecPath: {
      print_typespec_(spec->path.parent, i + 1);
      print_typespec_(spec->path.elem, i + 1);
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

void print_mutablity(Mutablity mut, int i) {
  printf("%s%s\n", indent(i), (mut == Immutable? "Immutable" : "Mutable"));
}
void print_pattern_(Pattern* pat, int i) {
  if(!pat) return;
  printf("%s%s\n", indent(i), pattern_string(pat->kind));
  switch(pat->kind) {
    case WildCard: {
      print_token_(&pat->wildcard, i + 1);
    } break;
    case StructPattern: {
      print_typespec_(pat->structure.path, i + 1);
      print_pattern_list(pat->structure.elems, pat->structure.num_elems, i + 1);
    } break;
    case TuplePattern: {
      print_pattern_list(pat->tuple.elems, pat->tuple.num_elems, i + 1);
    } break;
    case RefPattern: {
      print_mutablity(pat->ref.mut, i + 1);
      print_pattern_(pat->ref.pat, i + 1);
    } break;
    case PointerPattern: {
      print_mutablity(pat->ptr.mut, i + 1);
      print_pattern_(pat->ptr.pat, i + 1);
    } break;
    case IdentPattern: {
      print_ident_(pat->ident, i + 1);
    } break;
    case LiteralPattern: {
      print_literal_(&pat->literal, i + 1);
    } break;

  }
}

void print_clause_(Clause* clause, int i) {
  print_pattern_list(clause->patterns, clause->num_patterns, i);
  print_expr_(clause->body, i);
}

void note(const char* msg, ...) {
  printf("\tNote: ");
  va_list va;
  va_start(va, msg);
  vprintf(msg, va);
  va_end(va);
}

void print_literal(Token* token) { print_literal_(token, 0); }
void print_expr(Expr* expr) { print_expr_(expr, 0); }
void print_pattern(Pattern* pat) { print_pattern_(pat, 0); }
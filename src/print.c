#include "print.h"
#include "entity.h"
#include "type.h"

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

void print_loc(SourceLoc loc) {
  printf("\t%llu|%llu-%llu", loc.line, loc.column, loc.span);
}

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

void print_typespec_list(TypeSpec** p, u32 num, u32 in) {
  for(u32 i = 0; i < num; ++i)
    print_typespec_(p[i], in);
}

void print_expr_(Expr* expr, int i) {
  if(!expr) return;
  printf("%s%s", indent(i), expr_string(expr->kind));
  print_loc(expr->loc);
  printf("\n");
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
    case Index: {
      print_expr_(expr->index.operand, i + 1);
      print_expr_(expr->index.index, i + 1);
    } break;
    case TupleElem: {
      print_expr_(expr->tupleelem.operand, i + 1);
      print_token_(&expr->tupleelem.elem, i + 1);
    } break;
    case Range: {
      print_expr_(expr->range.start, i + 1);
      print_expr_(expr->range.end, i + 1);
      print_expr_(expr->range.step, i + 1);
    } break;
    case Slice: {
      print_expr_(expr->slice.operand, i + 1);
      print_expr_(expr->slice.start, i + 1);
      print_expr_(expr->slice.end, i + 1);
    } break;
    default:;
  }
}

void print_mutablity(Mutability mut, int i) {
  printf("%s%s\n", indent(i), (mut == Immutable? "Immutable" : "Mutable"));
}

void print_item_(Item* item, int i) {
  if(!item) return;
  printf("%s%s", indent(i), item_string(item->kind));
  print_loc(item->loc);
  printf("\n");
  switch(item->kind) {
    case ItemLocal: {
      print_mutablity(item->local.mut, i + 1);
      print_pattern_(item->local.name, i + 1);
      print_typespec_(item->local.type, i + 1);
      print_expr_(item->local.init, i + 1);
    } break;
    case ItemFunction: {
      print_ident_(item->function.name, i + 1);
      print_item_list(item->function.arguments, item->function.num_args, i + 1);
      print_typespec_(item->function.ret, i + 1);
      print_expr_(item->function.body, i + 1);
    } break;
    case ItemStruct: {
      print_ident_(item->structure.name, i + 1);
      print_item_list(item->structure.fields, item->structure.num_fields, i + 1);
    } break;
    case ItemTupleStruct: {
      print_ident_(item->tuplestruct.name, i + 1);
      print_typespec_list(item->tuplestruct.fields, item->tuplestruct.num_fields, i + 1);
    } break;
    case ItemEnum: {
      print_ident_(item->enumeration.name, i + 1);
      print_item_list(item->enumeration.elems, item->enumeration.num_elems, i + 1);
    } break;
    case ItemUse: {

    } break;
    case ItemModule: {

    } break;
    case ItemField: {
      print_ident_(item->field.name, i + 1);
      print_typespec_(item->field.type, i + 1);
      print_expr_(item->field.init, i + 1);
    } break;
    case ItemAlias: {
      print_ident_(item->alias.name, i + 1);
      print_typespec_(item->alias.type, i + 1);
    } break;
    case ItemName: {
      print_ident_(item->name.name, i + 1);
      print_expr_(item->name.value, i + 1);
    } break;
  }
}


void print_stmt_(Stmt* stmt, int i) {
  if(!stmt) return;
  printf("%s%s", indent(i), stmt_string(stmt->kind));
  print_loc(stmt->loc);
  printf("\n");
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
  if(!spec) return;
  printf("%s%s", indent(i), typespec_string(spec->kind));
  print_loc(spec->loc);
  printf("\n");
  print_mutablity(spec->mut, i + 1);
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
      // print_ident_(item->function.name, i + 1);
      print_typespec_list(spec->funct.args, spec->funct.num_args, i + 1);
      print_typespec_(spec->funct.ret, i + 1);
    } break;
    case TypeSpecArray: {
      print_typespec_(spec->array.elem, i + 1);
    } break;
    case TypeSpecPtr: {
      print_typespec_(spec->ptr.elem, i + 1);
    } break;
    case TypeSpecRef: {
      print_typespec_(spec->ref.elem, i + 1);
    } break;
    case TypeSpecMap: {
      print_typespec_(spec->map.key, i + 1);
      print_typespec_(spec->map.value, i + 1);
    } break;
    case TypeSpecTuple: {
      print_typespec_list(spec->tuple.types, spec->tuple.num_types, i + 1);
    } break;
  }
}

void print_pattern_(Pattern* pat, int i) {
  if(!pat) return;
  printf("%s%s", indent(i), pattern_string(pat->kind));
  print_loc(pat->loc);
  printf("\n");
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
    case RangePattern: {
      print_pattern_(pat->range.start, i + 1);
      print_pattern_(pat->range.end, i + 1);
    } break;
    default:;
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
void print_stmt(Stmt* stmt) { print_stmt_(stmt, 0); }
void print_pattern(Pattern* pat) { print_pattern_(pat, 0); }
void print_item(Item* item) { print_item_(item, 0); }


// void print_entity(Entity* entity) {
//   if(!entity) return;
//   printf("%s: %s\n", entity_string(entity), entity->name->value);
//   printf("\t%s\n", type_string(entity->type));
//   switch(entity->kind) {
//     case Entity_Const: {
//     } break;
//     case Entity_Local: {
//     } break;
//     case Entity_Alias: {
//     } break;
//     case Entity_Funct: {
//     } break;
//     case Entity_Type: {
//     } break;
//   }
// }
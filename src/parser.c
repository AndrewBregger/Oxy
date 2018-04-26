#include "parser.h"
#include "common.h"
#include "token.h"

#include "ast.h"

typedef u32 Restriciton;

const Restriciton DEFAULT = 0;
const Restriciton NO_STRUCT_LITERAL = 1 << 2;

typedef struct Parser {
  Token* tokens;
  Token* end;

  Token* current;

  Restriciton restrction;
  File* file;
} Parser;

Parser new_parser(File* file) {

}

void set_restriction(Parser* parser, Restriciton res) {
  parser->restrction = res;
}

Item** parse_file(File* file);

Expr* parse_expr(Parser* parser);
Expr* parse_expr_with_res(Parser* parser, Restriciton res);

Item* parse_item(Parser* parser);
Ident* parse_ident(Parser* parser);

Expr* parse_expr(Parser* parser) {
  return parse_expr_with_res(parser, DEFAULT);
}

Expr* parse_assoc_expr(Parser* parser);

Expr* parse_expr_with_res(Parser* parser, Restriciton res) {
  Restriciton old_res = parser->restrction;
  set_restriction(parser, res);
  Expr* expr = parse_assoc_expr(parser);
  set_restriction(parser, old_res);
  return expr;
}

Expr* parse_assoc_expr(Parser* parser) {

}


void parse_test(File* file) {

}

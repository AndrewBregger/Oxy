#include "parser.h"
#include "print.h"


#include <vector>
#include <iterator>
#include <cstring>
#include <cassert>
#include <cstdarg>

//const u32
using Restriction = u32;
const Restriction DEFAULT = 0xffffffff;

//Token current_token(Parser* parser);
//Token next_token(Parser* parser);

#define debug debug_(__FUNCTION__, current(parser));

void debug_(const char* function, Token token) {
  printf("%s| ", function);
  print_token(&token);
}

struct Parser {
  File* file;
  std::vector<Token> tokens;
  u64 index;
};



Parser new_parser(File* file) {
  std::vector<Token> tokens = scan_tokens(file);

  auto parser = Parser{
    file,
    tokens
  };

  return parser;
}

Token current(Parser* parser) {
  u64 index = parser->index;
  return parser->tokens[index];
}

Token next(Parser* parser) {
  u64 index = parser->index;
  assert(index + 1 > parser->tokens.size());
  return parser->tokens[index + 1];
}

SourceLoc expand_loc(SourceLoc start, SourceLoc next) {
  SourceLoc temp = start;
  temp.span += next.span;
  return temp;
}

SourceLoc loc_from_token(Parser* parser, Token token) {
  return new_sourceloc(parser->file, token.line, token.column, token.span);
}

void print_file_loc(SourceLoc loc) {
  std::string line = get_line(loc.file, loc.line);
  printf("\t|\n");
  printf("\t%llu|%s\n", loc.line, line.c_str());
  printf("\t|%s^\n", std::string(' ', loc.column).c_str());
}

void syntax_error(SourceLoc loc, const char* msg, ...) {
  printf("Error: %s:%llu:%llu ", loc.file->filename.c_str(), loc.line, loc.column);
  va_list va;
  va_start(va, msg);
  vprintf(msg, va);
  va_end(va);
  print_file_loc(loc);
}

void consume(Parser* parser) {
  // assert(parser->index + 1 > parser->tokens.size());
  ++parser->index;
}

bool match(Parser* parser, TokenKind kind) {
  if(current(parser).kind == kind) {
    consume(parser);
    return true;
  }
  return false;
}

bool expect(Parser* parser, TokenKind kind) {
  if(match(parser, kind)) {
    return true;
  }
  else {
    Token tok = new_token(kind, 0, 0, 0, 0);
    Token curr = current(parser);
    syntax_error(loc_from_token(parser, curr), "expecting: '%s', found: '%s'\n", get_token_c_str(&tok), get_token_c_str(&curr));
    return false;
  }
}

bool check(Parser* parser, TokenKind kind) {
  return current(parser).kind == kind;
}

Expr* parse_expr(Parser* parser, Restriction res);
Stmt* parse_stmt(Parser* parser);

ItemSet parse_file(File* root) {
  Parser parser = new_parser(root);
}

void parse_test(File* file) {
  Parser parser = new_parser(file);
  Expr* expr = parse_expr(&parser, DEFAULT);
  print_expr(expr);
}

Expr* parse_assoc_expr(Parser* parser, Restriction res, u32 prec);


Expr* parse_expr(Parser* parser, Restriction res) {
  debug
  return parse_assoc_expr(parser, res, 1);
}

Expr* parse_prefix(Parser* parser, Restriction res);

Expr* parse_assoc_expr(Parser* parser, Restriction res, u32 prec_min) {
  debug
  Expr* expr = parse_prefix(parser, res);
  while(true) {
    Token curr = current(parser);

    if(!is_operator(&curr))
      break;

    u32 prec = precedence(&curr);

    if(prec < prec_min)
      break;

    Assoc assoc = associative(&curr);

    consume(parser);

    Expr* rhs = parse_assoc_expr(parser, res, prec + 1);
    SourceLoc loc = expand_loc(expr->loc, rhs->loc);
    expr = new_binary(curr, expr, rhs, loc);
  }
  return expr;
}

Expr* parse_bottom(Parser* parser, Restriction res);
Expr* parse_suffix(Parser* parser, Restriction res);
Expr* parse_dot_suffix(Parser* parser, Restriction res, Expr* expr);
Expr* parse_dot_or_call(Parser* parser, Restriction res, Expr* already_parsed);

Expr* parse_prefix(Parser* parser, Restriction res) {
  debug
  Token curr= current(parser);
  switch(curr.kind) {
    case Tkn_Minus:
    case Tkn_Ampersand:
    case Tkn_Astrick: {
      consume(parser);
      Expr* expr = parse_prefix(parser, res);
      SourceLoc loc = new_sourceloc(parser->file, curr.index, curr.column, curr.span);
      loc = expand_loc(loc, expr->loc);
      return new_unary(curr, expr, loc);
    }
    default: {
      Expr* bottom = parse_bottom(parser, res);
      return parse_dot_or_call(parser, res, bottom);
    }
  }
  return nullptr;
}

Expr* parse_name(Parser* parser);
Expr* parse_literal(Parser* parser);
Expr* parse_block(Parser* parser);

Expr* parse_if(Parser* parser);
Expr* parse_while(Parser* parser);
Expr* parse_for(Parser* parser);

Expr* parse_bottom(Parser* parser, Restriction res) {
  debug
  Token curr = current(parser);

  switch(curr.kind) {
    case Tkn_Identifier:
      return parse_name(parser);
    case Tkn_If:
      return parse_if(parser);
//    case Tkn_Loop:
    case Tkn_While:
      //return parse_while(parser);
    case Tkn_For:
      //return parse_for(parser);
    case Tkn_OpenParen: {
      consume(parser);
    } break;
    case Tkn_OpenBracket:
      return parse_block(parser);
    default:
      return parse_literal(parser);
  }
}

std::vector<Expr*> parse_arguments(Parser* parser);

Expr* parse_dot_or_call(Parser* parser, Restriction res, Expr* already_parsed) {
  debug
  switch(already_parsed->kind) {
    case Literal:
    case Binary:
    case Unary:
    case CompoundLiteral:
      return already_parsed;
    default:;
  }

  Expr* expr = already_parsed;
  for(;;) {
    Token curr = current(parser);
    switch(curr.kind) {
      case Tkn_Period:
        consume(parser);
        expr = parse_dot_suffix(parser, res, expr);
        break;
      case Tkn_OpenParen: {
        consume(parser);
        std::vector<Expr*> args = parse_arguments(parser);
        expect(parser, Tkn_CloseParen);
        SourceLoc loc = expr->loc;
        for(auto x : args)
          loc = expand_loc(loc, x->loc);
        expr = new_fncall(expr, args, loc);
        break;
      }
      case Tkn_OpenBrace: {
        // indexing
      } break;
      case Tkn_OpenBracket: {
        printf("Paring open bracket from dot or call\n");
      }
      default:
        return expr;
    }
  }
}

Expr* parse_name(Parser* parser) {
  debug
  assert(current(parser).kind == Tkn_Identifier);
  Token curr = current(parser);

  consume(parser);
  SourceLoc loc = new_sourceloc(parser->file, curr.line, curr.column, curr.span);
  Ident* ident = new_ident(get_string(&curr), loc);
  return new_name(ident, ident->loc);
}

Expr* parse_literal(Parser* parser) {
  debug
  Token lit = current(parser);

  if(!is_literal(&lit)) {
    return nullptr;
  }
  consume(parser);
  SourceLoc loc = new_sourceloc(parser->file, lit.line, lit.column, lit.span);
  return new_literal(lit, loc);
}

Expr* parse_block(Parser* parser) {
  debug

  Token open = current(parser);
  expect(parser, Tkn_OpenBracket);
  std::vector<Stmt*> stmts;
  while(!check(parser, Tkn_CloseBracket)) {
    Stmt* stmt = parse_stmt(parser);
    if(stmt)
      stmts.push_back(stmt);
  }
  SourceLoc loc = loc_from_token(parser, open);
  for(auto x : stmts)
    loc = expand_loc(loc, x->loc);
  expect(parser, Tkn_CloseBracket);
  return new_block(stmts, loc);
}

std::vector<Expr*> parse_arguments(Parser* parser) {
  debug
  std::vector<Expr*> exprs;
  do {
    Expr* expr = parse_expr(parser, DEFAULT);
    if(!expr and !exprs.empty()) {
      // there was a common denoting another expression however nothing was found
      syntax_error(loc_from_token(parser, current(parser)), "expecting expression following ',' in function arguments\n");
      break;
    }
    else if(!expr)
      break;
    exprs.push_back(expr);
    // a valid argument will finish naturally here or in the else if statement
  } while(match(parser, Tkn_Comma));
  return exprs;
}

Expr* parse_dot_suffix(Parser* parser, Restriction res, Expr* expr) {
  debug

  Token curr = current(parser);
  switch(curr.kind) {
    // struct member excess
    case Tkn_Identifier: {
      Ident* field = new_ident(get_string(&curr), new_sourceloc(parser->file, curr.line, curr.column, curr.span));
      consume(parser);
      if(match(parser, Tkn_OpenParen)) {
        std::vector<Expr*> args = parse_arguments(parser);
        expect(parser, Tkn_CloseParen);

        SourceLoc loc = expand_loc(expr->loc, field->loc);
        loc.span += 2; // for the parens
        for(auto x : args)
          loc = expand_loc(loc, x->loc);

        return new_dotfncall(expr, field, args, loc);
      }
      else {
        return new_field(expr, field, expand_loc(expr->loc, field->loc));
      }
    }
    // tuple indexing
    case Tkn_IntLiteral:

    default:;
      // syntax error
  }
  return nullptr;
}

Expr* parse_if(Parser* parser) {
  debug
  
  SourceLoc loc = loc_from_token(parser, current(parser));
  expect(parser, Tkn_If);

  Expr* cond = parse_expr(parser, DEFAULT);
  loc = expand_loc(loc, cond->loc);

  Token curr = current(parser);
  print_token(&curr);

  if(check(parser, Tkn_OpenBracket) and next(parser).kind == Tkn_Pipe) {
    // this is match if expression
    printf("Compiler Error: Unimplemented\n");
    return nullptr;
  }
  else if(check(parser, Tkn_OpenBracket)) {
    printf("Parsing if body\n");
    Expr* body = parse_block(parser);
    loc = expand_loc(loc, body->loc);
    Expr* else_if = nullptr;

    if(match(parser, Tkn_Else)) {
      if(check(parser, Tkn_If))
        else_if = parse_if(parser); 
      else
        else_if = parse_block(parser);
    }
    return new_if(cond, body, else_if, loc);
  }
  else {
    printf("Compiler Error: if expression must have a body\n");
    return nullptr;
  }
}

Expr* parse_while(Parser* parser) {
  debug

}

Expr* parse_for(Parser* parser) {
  debug

}

Item* parse_item(Parser* parser);

Stmt* parse_stmt(Parser* parser) {
  Token token = current(parser);
  switch(token.kind) {
    case Tkn_Fn:
    case Tkn_Let:
    case Tkn_Struct: {
      Item* decl = parse_item(parser);
      expect(parser, Tkn_Newline);
      return new_item_stmt(decl, decl->loc);
    } break;
    default:;
  }
  Expr* expr = parse_expr(parser, DEFAULT);
  if(!expr) {
    Token nl = current(parser);
    expect(parser, Tkn_Newline);
    return new_expr_stmt(nullptr, loc_from_token(parser, nl));
  }
  return new_expr_stmt(expr, expr->loc);
}

Item* parse_item(Parser* parser) {
  return nullptr;;
}

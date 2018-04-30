#include "parser.h"
#include "common.h"
#include "token.h"

#include "ast.h"
#include "lex.h"
#include "print.h"
#include "report.h"

extern bool debug;

// assumes the use of parser pointer
#define Current() (*parser->current)
#define Next() (*(parser->current + 1))
// #define Consume() consume_(parser)
#define Consume() (++parser->current)
#define IS_EOF() parser->current == parser->end

#define Debug() debug_(__FUNCTION__, __LINE__, Current())

void debug_(const char* funct, i32 line, Token token) {
  if(debug) {
    printf("%s|%d\t", funct, line); 
    print_token(&token);
  }
}


SourceLoc expand_loc(SourceLoc src, SourceLoc add) {
  SourceLoc temp = src;
  temp.span += add.span;
  return temp;
}

typedef u32 Restriciton;

const Restriciton DEFAULT = 0;
const Restriciton NO_STRUCT_LITERAL = 1 << 1;
const Restriciton NO_EXPR_STMT = 1 << 2;

typedef struct Parser {
  Token* tokens;
  Token* end;

  Token* current;

  Restriciton restrction;
  File* file;
  Token* comments;
} Parser;

TypeSpec* expr_to_typespec(Expr* expr);

TypeSpec* parse_typespec(Parser* parser);

SourceLoc loc_from_token(Parser* parser, Token token) {
  SourceLoc loc;
  loc.file = parser->file;
  loc.line = token.line;
  loc.column = token.line;
  loc.span = token.span;
  return loc;
}

Parser new_parser(File* file) {
  u32 num;
  Token* tokens = get_tokens(file, &num);

  Parser parser;
  parser.tokens = tokens;
  parser.end = tokens + num;
  parser.current = tokens;
  parser.file = file;
  parser.comments = NULL;
  return parser;
}

void set_restriction(Parser* parser, Restriciton res) {
  parser->restrction = res;
}

#define check(tok) check_(parser, tok)
#define match(tok) match_(parser, tok)
#define expect(tok) expect_(parser, tok)

bool check_(Parser* parser, TokenKind kind) {
  return Current().kind == kind;
}

// void consume_(Parser* parser) {
//   while(true) {
//     ++parser->current;
//     if(Current().kind == Tkn_Comment)
//       buf_push(parser->comments, Current());
//     else
//       break;
//   }
// }

bool match_(Parser* parser, TokenKind kind) {
  if(Current().kind == kind) {
    Consume();
    return true;
  }
  return false;
}

bool expect_(Parser* parser, TokenKind kind) {
  if(match(kind))
    return true;
  else {
    Token current = Current();
    Token temp = current;
    temp.kind = kind;
    syntax_error(loc_from_token(parser, Current()), "Expecting '%s', found '%s'\n", get_token_string(&temp),
      get_token_string(&current));
    return false;
  }
}



void sync(Parser* parser);

Expr* parse_expr(Parser* parser);
Expr* parse_expr_with_res(Parser* parser, Restriciton res);
Stmt* parse_stmt(Parser* parser);
Item* parse_item(Parser* parser);
Ident* parse_ident(Parser* parser);

Expr* parse_expr(Parser* parser) {
  Debug();
  return parse_expr_with_res(parser, DEFAULT);
}

Expr* parse_assoc_expr(Parser* parser, u32 min_prec);

Expr* parse_expr_with_res(Parser* parser, Restriciton res) {
  Debug();
  Restriciton old_res = parser->restrction;
  set_restriction(parser, res);
  Expr* expr = parse_assoc_expr(parser, 1);
  set_restriction(parser, old_res);
  return expr;
}

Expr* parse_prefix_expr(Parser* parser);
Expr* parse_bottom_expr(Parser* parser);
Expr* parse_dot_call_expr(Parser* parser, Expr* already_parsed);
Expr* parse_if_expr(Parser* parser);
Expr* parse_while_expr(Parser* parser);
Expr* parse_for_expr(Parser* parser);
Expr* parse_dot_suffix_expr(Parser* parser, Expr* operand);

Expr* parse_assoc_expr(Parser* parser, u32 min_prec) {
  Debug();
  Expr* expr = parse_prefix_expr(parser);
  while(precedence(&Current()) >= min_prec) {
    Token current = Current();
    if(debug) {
      printf("Current Op: ");
      print_token(&current);
      printf("\tPrec: %u, Min Prec: %u\n", precedence(&current), min_prec);
    }
    Consume();
    if(precedence(&current) < min_prec)
      break;
    if(!is_operator(&current))
      syntax_error(loc_from_token(parser, current), "Execting binary operator in expression, found: '%s'\n", get_token_string(&current));
      // continue parsing as if it was valid

    Expr* rhs = parse_assoc_expr(parser, precedence(&current) + 1);
    SourceLoc loc = expand_loc(expr->loc, loc_from_token(parser, current));
    loc = expand_loc(loc, rhs->loc);
    if(is_assignment(&current))
      expr = new_assign(current, expr, rhs, loc);
    else
      expr = new_binary(current, expr, rhs, loc);
  }
  return expr;
}

Expr* parse_prefix_expr(Parser* parser) {
  Debug();
  Token current = Current();
  switch(current.kind) {
    case Tkn_Minus:
    case Tkn_Astrick:
    case Tkn_Ampersand:
    case Tkn_Tilde: {
      Consume();
      Expr* expr = parse_prefix_expr(parser);
      SourceLoc loc = loc_from_token(parser, current);
      return new_unary(current, expr, expand_loc(loc, expr->loc));
    }
    default:;
  }

  Expr* bottom = parse_bottom_expr(parser);
  return parse_dot_call_expr(parser, bottom);
}

Expr* parse_name_expr(Parser* parser);
Expr* parse_literal(Parser* parser);

Expr* parse_block_expr(Parser* parser);

Expr* parse_bottom_expr(Parser* parser) {
  Debug();
  Token current = Current();
  switch(current.kind) {
    case Tkn_If:
      // return parse_if_expr(parser);
    case Tkn_While:
      // return parse_while_expr(parser);
    case Tkn_For:;
      // return parse_for_expr(parser);
    case Tkn_Identifier:
      return parse_name_expr(parser);
    case Tkn_OpenParen: {
      // sub expression
      Token current = Current();
      SourceLoc loc = loc_from_token(parser, current);
      Consume();
      Expr* expr = parse_expr(parser);
      
      loc = expand_loc(loc, expr->loc);
      // tuple expression
      if(check(Tkn_Comma)) {

        Expr** elems = NULL;
        buf_push(elems, expr);
        while(match(Tkn_Comma)) {
          loc = expand_loc(loc, loc_from_token(parser, *(parser->current - 1)));
          Expr* e = parse_expr(parser);
          loc = expand_loc(loc, e->loc);
          if(!e) {
            syntax_error(loc_from_token(parser, Current()), "Expecting primary expression in tuple\n");
            break;
          }

          buf_push(elems, e);
        }
        expect(Tkn_CloseParen);

        // add the close paren to the location and span.
        loc = expand_loc(loc, loc_from_token(parser, *(parser->current - 1)));
        return new_tuple(elems, loc);
      }
      else {
        expect(Tkn_CloseParen);
        return expr;
      }
    }
    case Tkn_OpenBracket:
      return parse_block_expr(parser);
    // block expression
    case Tkn_OpenBrace:;
    // array and map literals
    default:;
  }

  if(is_literal(&current))
    return parse_literal(parser);
  
  return NULL;
}

Expr* parse_block_expr(Parser* parser) {
  if(check(Tkn_OpenBracket)) {
    SourceLoc loc = loc_from_token(parser, Current());
    Consume();
    Stmt** stmts = NULL;
    while(!check(Tkn_CloseBracket)) {
      Stmt* stmt = parse_stmt(parser);
      if(!stmt)
        break;
      loc = expand_loc(loc, stmt->loc);
      buf_push(stmts, stmt);
    }
    expect(Tkn_CloseBracket);
    loc.span += 1;
    return new_block(stmts, loc);
  }
  else {
   syntax_error(loc_from_token(parser, Current()), "Execting '{' to begin a block, found: '%s'\n", get_token_string(&Current())); 
  }
  return NULL;
}

Expr* parse_literal(Parser* parser) {
  Debug();
  Token current = Current();
  if(is_literal(&current)) {
    Consume();
    return new_literal(current, loc_from_token(parser, current));
  }
  syntax_error(loc_from_token(parser, current), "Expecting literal: found '%s'\n", get_token_string(&current));
  return NULL;
}

Expr** parse_fn_arguments(Parser* parser);

Expr* parse_dot_call_expr(Parser* parser, Expr* already_parsed) {
  Debug();
  Expr* expr = already_parsed;
  u32 iter = 1;
  for(;;) {
    if(debug) {
      printf("Dot Call Iteration: %u\t", iter);
      print_token(&Current());
    }

    // assert(expr->kind != Binding);

    switch(Current().kind) {
      case Tkn_Period:
        Consume();
        expr = parse_dot_suffix_expr(parser, expr);
      break;
      case Tkn_OpenParen: {
        SourceLoc loc = expr->loc;
        expect(Tkn_OpenParen);
        loc.span += 1;
        Expr** args = parse_fn_arguments(parser);
        u32 len = buf_len(args);
        for(u32 i = 0; i < len; ++i)
          loc = expand_loc(loc, args[i]->loc);
        loc.span += len;
        expr = new_fncall(expr, args, loc);
      } break;
      case Tkn_OpenBracket: {
        TypeSpec* type = expr_to_typespec(expr);
        expect(Tkn_OpenBracket);
        Expr** args = NULL;

        bool expecting_expr = true;
        if(!check(Tkn_CloseBracket)) {
          while(expecting_expr) {
            expecting_expr = false;
            Expr* expr = parse_expr(parser);
            if(expr) {
              buf_push(args, expr);
            }
            else {
              syntax_error(loc_from_token(parser, Current()), "Expecting primary expression following comma in struct literal\n");
              break;
            }
            
            if(match(Tkn_Comma))
              expecting_expr = true;
          }
        }
        expect(Tkn_CloseBracket);

        SourceLoc loc = expand_loc(expr->loc, expr->loc);

        for(u32 i = 0; i < buf_len(args); ++i)
          loc = expand_loc(loc, args[i]->loc);

        return new_structliteral(type, args, loc);
      } break;
      case Tkn_Colon: {
        expect(Tkn_Colon);

        Expr* e = parse_expr(parser);
        // i know the name isnt right but it does the same thing.
        SourceLoc loc = expand_loc(expr->loc, e->loc);
        loc.span += 1;
        // the second term is to check if this name has generics.
        if(expr->kind != Name and true) {
          syntax_error(e->loc, "recieving term must be an identifier\n");
        }

        expr = new_binding(expr, e, loc);
      } break;
      default:
        return expr;
    }
    ++iter;
  }

  return expr;
}


// assumes the period has already been parsed
Expr* parse_dot_suffix_expr(Parser* parser, Expr* operand) {
  Debug();
  Token current = Current();
  switch(current.kind) {
    case Tkn_Identifier: {
      if(Next().kind == Tkn_OpenParen) {
        SourceLoc loc = operand->loc;
        Expr* name = parse_name_expr(parser);
        Consume();
        loc = expand_loc(loc, name->loc);
        Expr** args = parse_fn_arguments(parser);
        u32 len = buf_len(args);
        for(u32 i = 0; i < len; ++i)
          loc = expand_loc(loc, args[i]->loc);
        loc.span += len;
        return new_dotfncall(operand, name, args, loc);
      }
      else {
        Ident* name = parse_ident(parser);
        SourceLoc loc = expand_loc(operand->loc, name->loc);
        return new_field(operand, name, loc);
      }
    } break; 
    case Tkn_IntLiteral:
    default: {
      syntax_error(loc_from_token(parser, current), "Expecting an identifier or integer following period, found: '%s'\n", get_token_string(&current));
    }
  }
  return NULL;
}

Expr** parse_fn_arguments(Parser* parser) {
  Expr** args = NULL;
  bool expecting_expr = true;
  if(!check(Tkn_CloseParen)) {
    while(expecting_expr) {
      printf("Token Loop: ");
      print_token(&Current());
      expecting_expr = false;
      Expr* expr = parse_expr(parser);
      if(expr) {
        buf_push(args, expr);
      }
      else {
        syntax_error(loc_from_token(parser, Current()), "Expecting primary expression following comma in function arguments\n");
        break;
      }
      
      if(match(Tkn_Comma))
        expecting_expr = true;
    }
  }
  print_token(&Current());
  expect(Tkn_CloseParen);
  return args;
}

Expr* parse_name_expr(Parser* parser) {
  Debug();
  Token current = Current();
  Ident* ident = parse_ident(parser);
  // check for polymohpic parameters
  return new_name(ident, ident->loc);
}

Ident* parse_ident(Parser* parser) {
  Debug();
  Token current = Current();
  if(check(Tkn_Identifier)) {
    Consume();
    return new_ident(get_string(&current), loc_from_token(parser, current));
  }
  else
    syntax_error(loc_from_token(parser, current), "Expecting identifier: found '%s'\n", get_token_string(&current));
  return NULL;
}

Stmt* parse_stmt(Parser* parser) {
  Debug();
  Token current = Current();
  switch(current.kind) {
    case Tkn_Fn:
    case Tkn_Let:
    case Tkn_Struct:
      syntax_error(loc_from_token(parser, current), "Unimplemented feature\n");
      return NULL;
    // case Tkn_Enum:
    default:;
  }

  Expr* expr = parse_expr(parser);

  if(expr) {
    SourceLoc loc = expr->loc;
    if(match(Tkn_Semicolon)) {
      loc.span += 1;
      return new_semi_stmt(expr, loc);
    }
    else
      return new_expr_stmt(expr, loc);
  }
  return NULL;
}

TypeSpec* expr_to_name_type(Expr* name) {

}

TypeSpec* expr_to_typespec(Expr* expr) {
  if(!expr) return NULL;

  switch(expr->kind) {
    case Name:
      return new_name_typespec(expr->name, Immutable, expr->loc);
    case Field: {
      return new_path_typespec(expr_to_typespec(expr->field.operand), new_name_typespec(expr->field.name, 
        Immutable, expr->field.name->loc), Immutable, expr->loc);
    }
    case DotFnCall: {
      printf("Type Generics not implemented\n");
    }
    default: return NULL;
  }
}

TypeSpec* parse_typespec(Parser* parser) {

}

void sync(Parser* parser) {

}

void parse_test(File* file) {
  Parser parser = new_parser(file);

  Expr* expr = parse_expr(&parser);

  print_expr(expr);
}


Item** parse_file(File* file) {
  return NULL;
}
#include "parser.h"
#include "common.h"
#include "token.h"

#include "ast.h"
#include "lex.h"
#include "print.h"
#include "report.h"
#include "oxy.h"

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
const Restriciton NO_BINDING = 1 << 3;


typedef struct Parser {
  Token* tokens;
  Token* end;

  Token* current;

  Restriciton restriction;
  File* file;
  Token* comments;

  StringTable* table;
} Parser;

TypeSpec* expr_to_typespec(Expr* expr);

TypeSpec* parse_typespec(Parser* parser);

Item* parse_item(Parser* parser);

SourceLoc loc_from_token(Parser* parser, Token token) {
  SourceLoc loc;
  loc.file = parser->file;
  loc.line = token.line;
  loc.column = token.line;
  loc.span = token.span;
  return loc;
}

Parser new_parser(File* file, StringTable* table) {
  u32 num;
  Parser parser;

  parser.table = table;// create_table(TABLE_START);
  //the tokenizer builds the string table.

  Token* tokens = get_tokens(file, &num, parser.table);

  parser.tokens = tokens;
  parser.end = tokens + num;
  parser.current = tokens;
  parser.file = file;
  parser.comments = NULL;

  return parser;
}

void set_restriction(Parser* parser, Restriciton res) {
  parser->restriction = res;
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
  if(check(kind)) {
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
  Restriciton old_res = parser->restriction;
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

    if(current.kind == Tkn_PeriodPeriod) {
      Expr* end = parse_expr_with_res(parser, NO_STRUCT_LITERAL);
      Expr* step = NULL;
      if(match(Tkn_Comma))
        step = parse_expr_with_res(parser, NO_STRUCT_LITERAL);
      SourceLoc loc = expr->loc;
      if(end)
        loc = expand_loc(loc, end->loc);
      if(step)
        loc = expand_loc(loc, step->loc);
      return new_range(expr, end, step, loc);
    }

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
      // Consume();
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
      return parse_if_expr(parser);
    case Tkn_While:
      return parse_while_expr(parser);
    case Tkn_For:;
      return parse_for_expr(parser);
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
    case Tkn_OpenBrace: {
      // array and map literals
      expect(Tkn_OpenBrace);
      Expr** elems = NULL;
      SourceLoc loc = loc_from_token(parser, current);
      while(!check(Tkn_CloseBrace)) {
        Expr* elem = parse_expr(parser);
        if(elem) {
          buf_push(elems, elem);
          loc = expand_loc(loc, elem->loc);
        }
        match(Tkn_Comma);
      }
      expect(Tkn_CloseBrace);
      loc.span += 1;
      return new_compoundliteral(elems, loc);
    }
    default:;
  }

  if(is_literal(&current))
    return parse_literal(parser);

  return NULL;
}

/// @TODO(Andrew): make sure expr statement is last
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
        if(parser->restriction & NO_STRUCT_LITERAL)
          return expr;
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

        expr = new_structliteral(type, args, loc);
      } break;
      case Tkn_OpenBrace: {
        expect(Tkn_OpenBrace);
        Expr* index = parse_expr_with_res(parser, NO_BINDING);
        Expr* end = NULL;
        bool is_slice = false;
        SourceLoc loc = expand_loc(expr->loc, index->loc);

        // check for slice syntax
        if(match(Tkn_Colon)) {
          end = parse_expr_with_res(parser, NO_BINDING);
          // it is allowed to be null. it just means it is the rest of the array from start
          if(end)
            loc = expand_loc(loc, end->loc);

          is_slice = true;
        }

        expect(Tkn_CloseBrace);
        if(is_slice)
          expr = new_slice(expr, index, end, loc);
        else
          expr = new_index(expr, index, loc);
      } break;
      case Tkn_Colon: {
        if(parser->restriction & NO_BINDING)
          return expr;
        expect(Tkn_Colon);

        Expr* e = parse_expr(parser);
        SourceLoc loc;
        if(!e) {
          syntax_error(loc_from_token(parser, Current()), "expecting primary expression following colon\n");
        }
        else {
          // i know the name isnt right but it does the same thing.
          loc = expand_loc(expr->loc, e->loc);
          loc.span += 1;
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
    case Tkn_IntLiteral: {
      Token index = Current();
      Consume();
      return new_tupelelem(operand, index, operand->loc);
    }
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
      // printf("Token Loop: ");
      // print_token(&Current());
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
    const char* str = get_string(&current);
    // const char* ident = table_insert_string(&parser->table, str);
    // printf("Ident, using string at: %p\n", ident);
    return new_ident(str, loc_from_token(parser, current));
  }
  else
    syntax_error(loc_from_token(parser, current), "Expecting identifier: found '%s'\n", get_token_string(&current));
  return NULL;
}

Expr* parse_match(Parser* parser, Expr* cond);
Expr* parse_if_body(Parser* parser, Expr* cond);
Clause* parse_clause(Parser* parser);

Expr* parse_if_expr(Parser* parser) {
  Debug();
  expect(Tkn_If);
  Expr* cond = parse_expr_with_res(parser, NO_STRUCT_LITERAL);
  if(check(Tkn_OpenBracket)) {
    if(Next().kind == Tkn_Pipe) {
      Consume();
      return parse_match(parser, cond);
    }
    else
      return parse_if_body(parser, cond);
  }
  else {
    syntax_error(loc_from_token(parser, Current()), "expecting open bracket\n");
    note("if body must start with '}'\n");
  }
  return NULL;
}

Pattern* parse_pattern(Parser* parser);

Expr* parse_for_expr(Parser* parser) {
  Token top = Current();
  expect(Tkn_For);
  Pattern* pat = parse_pattern(parser);
  SourceLoc loc = loc_from_token(parser, Current());
  if(!pat) {
    syntax_error(loc, "execting pattern in for\n");
    sync(parser);
  }
  if(match(Tkn_In)) {
    Expr* expr = parse_expr_with_res(parser, NO_STRUCT_LITERAL);
    if(check(Tkn_OpenBracket)) {
      Expr* body = parse_block_expr(parser);
      SourceLoc l = loc_from_token(parser, top);
      l = expand_loc(l, pat->loc);
      l = expand_loc(l, expr->loc);
      l = expand_loc(l, body->loc);
      return new_for(pat, expr, body, l);
    }
    else {
      loc = loc_from_token(parser, Current());
      syntax_error(loc, "expecting '{' in a block\n");
      sync(parser);
    }
  }
  else {
    syntax_error(loc, "expecting 'in' in for\n");
    sync(parser);
  }

  return NULL;
}

Expr* parse_while_expr(Parser* parser) {
  Token top = Current();
  expect(Tkn_While);
  Expr* cond = parse_expr_with_res(parser, NO_STRUCT_LITERAL);
  if(check(Tkn_OpenBracket)) {
    Expr* body = parse_block_expr(parser);
    SourceLoc l = loc_from_token(parser, top);
    l = expand_loc(l, cond->loc);
    l = expand_loc(l, body->loc);
    return new_while(cond, body, l);
  }
  else {
    SourceLoc loc = loc_from_token(parser, Current());
    syntax_error(loc, "expecting '{' to begin a body\n");
    sync(parser);
  }
  return NULL;
}

Mutability parse_mutability(Parser* parser);
Pattern* parse_ident_pattern(Parser* parser);

Pattern* parse_pattern(Parser* parser) {
  Debug();
  Token current = Current();
  switch(current.kind) {
    case Tkn_Identifier:
      return parse_ident_pattern(parser);
    case Tkn_OpenParen: {
      // @Note(Andrew): Maybe this should be valid, at the moment it isnt.
      // (Io(_), z, y)
      Pattern** pats = NULL;
      Consume();
      SourceLoc loc = loc_from_token(parser, current);
      while(!check(Tkn_CloseParen)) {
        Pattern* pat = parse_pattern(parser);
        if(pat) {
          if(pat->kind == StructPattern || pat->kind == TuplePattern) {
            syntax_error(pat->loc, "tuple elements must be build to a name\n");
          }
          buf_push(pats, pat);
          loc = expand_loc(loc, pat->loc);
        }
        else {
          syntax_error(loc_from_token(parser, Current()), "expecting pattern\n");
          break;
        }
        if(!match(Tkn_Comma))
          break;
      }
      expect(Tkn_CloseParen);
      loc.span += 1;
      return new_tuple_pat(pats, loc);
    } break;
    case Tkn_Ampersand: {
      Consume();
      Mutability mut = parse_mutability(parser);
      Pattern* pat = parse_pattern(parser);
      if(pat->kind != IdentPattern) {
        syntax_error(pat->loc, "reference pattern must bind to a name\n");
      }
      return new_ref_pat(mut, pat, pat->loc);
    } break;
    case Tkn_Astrick: {
      Consume();
      Mutability mut = parse_mutability(parser);
      Pattern* pat = parse_pattern(parser);
      if(pat->kind != IdentPattern) {
        syntax_error(pat->loc, "pointer pattern must bind to a name\n");
      }
      return new_ptr_pat(mut, pat, pat->loc);
    } break;
    case Tkn_Underscore: {
      Consume();
      return new_wildcard(current, loc_from_token(parser, current));
    }
    default:;
  }
  if(is_literal(&current)) {
    Consume();
    Pattern* lit = new_literal_pat(current, loc_from_token(parser, current));
    if(match(Tkn_PeriodPeriod)) {
      Pattern* end = parse_pattern(parser);
      if(!end) {
        syntax_error(loc_from_token(parser, Current()), "execting end value for range pattern\n");
        return NULL;
      }
      if(end->kind != LiteralPattern) {
        syntax_error(loc_from_token(parser, Current()), "range end must be a literal\n");
      }
      SourceLoc loc = lit->loc;
      loc.span += 2;
      return new_range_pat(lit, end, expand_loc(loc, end->loc));
    }
    return lit;
  }
  return NULL;
}

Mutability parse_mutability(Parser* parser) {
  Debug();
  Token curr = Current();
  if(curr.kind == Tkn_Mut) {
    Consume();
    return Mutable;
  }
  else
    return Immutable;
}

Pattern* parse_stuct_pattern(Parser* parser) {
  Debug();
  TypeSpec* spec = parse_typespec(parser);
  if(match(Tkn_OpenParen)) {
    Pattern** pats = NULL;
    // @TODO(Andrew): handle ... case
    SourceLoc loc = spec->loc;
    while(!check(Tkn_CloseParen)) {
      Pattern* pat = parse_pattern(parser);
      if(pat && (pat->kind == StructPattern || pat->kind == TuplePattern)) {
        syntax_error(pat->loc, "invalid sub pattern in structure pattern\n");
        sync(parser);
      }
      if(pat) {
        buf_push(pats, pat);
        loc = expand_loc(loc, pat->loc);
      }
      else {
        syntax_error(loc_from_token(parser, Current()), "expecting pattern, found: '%s'\n", get_token_string(&Current()));
        sync(parser);
      }

      if(!match(Tkn_Comma))
        break;
    }
    expect(Tkn_CloseParen);
    loc.span += 1;
    return new_struct_pat(spec, pats, loc);
  }
  else {
    syntax_error(loc_from_token(parser, Current()), "expecting '(' follow type path\n");
    sync(parser);
  }
  return NULL;
}

Pattern* parse_ident_pattern(Parser* parser) {
  Debug();
  if(check(Tkn_Identifier)) {
    Token next = Next();
    if(next.kind == Tkn_Period)
      return parse_stuct_pattern(parser);
    else {
      if(next.kind == Tkn_OpenParen)
        return parse_stuct_pattern(parser);
      else {
        Ident* ident = parse_ident(parser);
        return new_ident_pat(ident, ident->loc);
      }
    }
  }
  return NULL;
}

Pattern** parse_patterns(Parser* parser, TokenKind delim);

Clause* parse_clause(Parser* parser) {
  Debug();
  Pattern** patterns = parse_patterns(parser, Tkn_Pipe);
  SourceLoc loc;
  if(patterns == NULL)
    syntax_error(loc_from_token(parser, Current()), "expecting pattern in matching if\n");
  else {
    loc = patterns[0]->loc;
    for(u32 i = 1; i < buf_len(patterns); ++i)
      loc = expand_loc(loc, patterns[i]->loc);
  }
  Expr* expr = parse_expr(parser);
  return new_clause(patterns, expr, loc);
}

Pattern** parse_patterns(Parser* parser, TokenKind delim) {
  Debug();
  Pattern** pats = NULL;
  do {
    Pattern* pat = parse_pattern(parser);
    if(pat) {
      buf_push(pats, pat);
    }
    else {
      syntax_error(loc_from_token(parser, Current()), "expecting pattern in match clause\n");
      sync(parser);
    }
  } while(match(delim));
  return pats;
}


Expr* parse_match(Parser* parser, Expr* cond) {
  Debug();
  Clause** clauses = NULL;

  // @TODO(Andrew): better error checking for when there are more clauses
  // but there isnt a comma delimiting them.
  do {
    if(match(Tkn_Pipe)) {
      Clause* clause = parse_clause(parser);
      if(clause)
        buf_push(clauses, clause);
      else {
        syntax_error(loc_from_token(parser, Current()), "expecting pattern following pipe\n");
        sync(parser);
        break;
      }
    }
    else break;
  } while(match(Tkn_Comma));

  return new_matchif(cond, clauses, loc_from_token(parser, Current()));
}

Expr* parse_if_body(Parser* parser, Expr* cond) {
  Debug();
  Expr* body = parse_block_expr(parser);
  Expr* else_if = NULL;
  if(match(Tkn_Else)) {
    if(check(Tkn_If))
      else_if = parse_if_expr(parser);
    else
      else_if = parse_block_expr(parser);
  }
  SourceLoc loc = cond->loc;
  loc = expand_loc(loc, body->loc);
  if(else_if)
    loc = expand_loc(loc, else_if->loc);
  return new_if(cond, body, else_if, loc);
}

bool requires_semicolon(Expr* expr) {
  if(!expr) return false;
  switch(expr->kind) {
    case If:
    case While:
    case For:
      return false;
    default:
      return true;
  }
}

Stmt* parse_stmt(Parser* parser) {
  Debug();
  Token current = Current();
  switch(current.kind) {
    case Tkn_Fn:
    case Tkn_Let:
    case Tkn_Struct:
    case Tkn_Enum:
    case Tkn_Type: {
      // syntax_error(loc_from_token(parser, current), "Unimplemented feature\n");
      Item* item = parse_item(parser);
      if(item->kind == ItemLocal || item->kind == ItemUse || item->kind == ItemAlias
         || item->kind == ItemTupleStruct)
        expect(Tkn_Semicolon);
      return new_item_stmt(item, item->loc);
    }
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
    else {
      // if(requires_semicolon(expr)) {
        // syntax_error(loc_from_token(parser, Current()), "expecting ';'\n");
      // }
      return new_expr_stmt(expr, loc);
    }
  }
  return NULL;
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

TypeSpec* parse_name_or_path(Parser* parser);

TypeSpec* parse_typespec(Parser* parser) {
  Debug();
  Token current = Current();
  SourceLoc loc = loc_from_token(parser, current);
  switch(current.kind) {
    case Tkn_Identifier:
      return parse_name_or_path(parser);
    case Tkn_Astrick: {
      Consume();
      Mutability mut = parse_mutability(parser);
      loc.span += (mut == Mutable ? 3 : 0);
      TypeSpec* type = parse_typespec(parser);
      return new_ptr_typespec(type, mut, expand_loc(loc, type->loc));
    } break;
    case Tkn_Ampersand: {
#ifdef USE_REF_TYPE
      Consume();
      Mutability mut = parse_mutability(parser);
      loc.span += (mut == Mutable ? 3 : 0);
      TypeSpec* type = parse_typespec(parser);
      return new_ref_typespec(type, mut, expand_loc(loc, type->loc));
#else
     syntax_error(loc, "reference types are allowed at the moment\n");
#endif
    } break;
    case Tkn_OpenBrace: {
      Consume();
      if(match(Tkn_CloseBrace)) {
        TypeSpec* type = parse_typespec(parser);
        loc.span += 2;
        return new_array_typespec(type, expand_loc(loc, type->loc));
      }
      else {
        TypeSpec* key = parse_typespec(parser);
        expect(Tkn_Comma);
        TypeSpec* value = parse_typespec(parser);
        expect(Tkn_CloseBrace);
        return new_map_typespec(key, value, Immutable, expand_loc(key->loc, value->loc));
      }
    } break;
    case Tkn_OpenParen: {

    } break;
    case Tkn_Fn: {} break;
    default:;
  }
  return NULL;
}

TypeSpec* parse_name_typespec(Parser* parser) {
  Debug();
  Ident* ident = parse_ident(parser);
  // parse genarics
  return new_name_typespec(ident, 1, ident->loc);
}

TypeSpec* parse_name_or_path(Parser* parser) {
  Debug();
  TypeSpec* spec = parse_name_typespec(parser);
  for(;;) {
    if(Current().kind == Tkn_Period) {
      Consume();
      TypeSpec* elem = parse_name_typespec(parser);
      spec = new_path_typespec(spec, elem, 1, expand_loc(spec->loc, elem->loc));
    }
    else break;
  }
  return spec;
}

Item* parse_local_item(Parser* parse);
Item* parse_function_item(Parser* parse);
Item* parse_structure_item(Parser* parse);
Item* parse_tuplestruct_item(Parser* parse);
Item* parse_enum_item(Parser* parse);
Item* parse_use_item(Parser* parse);
Item* parse_module_item(Parser* parse);
Item* parse_field_item(Parser* parse);
Item* parse_type_or_tuplestruct(Parser* parser);


Item* parse_item(Parser* parser) {
  Debug();
  switch(Current().kind) {
    case Tkn_Let:
      return parse_local_item(parser);
    case Tkn_Fn:
      return parse_function_item(parser);
    case Tkn_Struct:
      return parse_structure_item(parser);
    case Tkn_Enum:
      return parse_enum_item(parser);
    case Tkn_Use:
      return parse_use_item(parser);
    case Tkn_Type:
      return parse_type_or_tuplestruct(parser);
    default:
      return NULL;
  }
}

Item* parse_local_item(Parser* parser) {
  Debug();
  Token current = Current();
  SourceLoc loc = loc_from_token(parser, current);

  expect(Tkn_Let);
  Mutability mut = parse_mutability(parser);
  if(mut == Mutable)
    loc.span += 3;

  // Pattern** patterns = parse_patterns(parser, Tkn_Comma);
  // for(u32 x = 0; x < buf_len(patterns); ++x)
  //   loc = expand_loc(loc, patterns[x]->loc);
  Pattern* pattern = parse_pattern(parser);

  TypeSpec* spec = NULL;
  Expr* init = NULL;
  if(match(Tkn_Colon)) {
    spec = parse_typespec(parser);
    loc = expand_loc(loc, spec->loc);
  }

  if(match(Tkn_Equal)) {
    init = parse_expr(parser);
    loc = expand_loc(loc, init->loc);
  }

  if(!spec and !init) {
    syntax_error(loc_from_token(parser, Current()), "must annotate type without initialization\n");
  }

  if(!match(Tkn_Semicolon)) {
    syntax_error(loc_from_token(parser, Current()), "expecting ';' following variable declaration\n");
  }

  return new_itemlocal(pattern, spec, init, mut, loc);
}

Item* parse_function_item(Parser* parser) {
  Debug();
  Token top = Current();
  expect(Tkn_Fn);
  SourceLoc loc = loc_from_token(parser, top);

  if(check(Tkn_Identifier)) {
    Ident* name = parse_ident(parser);

    expect(Tkn_OpenParen);
    Item** params = NULL;
    while(!check(Tkn_CloseParen)) {
      if(!check(Tkn_Identifier)) {
        syntax_error(loc_from_token(parser, Current()), "execting an identifer following '('\n");
        sync(parser);
        break;
      }

      Item* param = parse_field_item(parser);
      buf_push(params, param);
      loc = expand_loc(loc, param->loc);

      if(!match(Tkn_Comma))
        break;
    }
    expect(Tkn_CloseParen);
    TypeSpec* spec = NULL;
    Expr* body = NULL;
    if(!check(Tkn_OpenBracket)) {
      spec = parse_typespec(parser);
      if(!spec) {
        syntax_error(loc_from_token(parser, Current()), "expecting return type, found: '%s'\n", get_token_string(&Current()));
        sync(parser);
      }
      else
        loc = expand_loc(loc, spec->loc);
    }
    if(check(Tkn_OpenBracket)) {
      body = parse_block_expr(parser);
    }
    else {
      syntax_error(loc_from_token(parser, Current()), "expecting '{'\n");
    }
    return new_itemfunction(name, params, spec, body, loc);
  }
  else if(is_operator(&Current())) {
    printf("Overloading of operators is currently unimplemented\n");
  }

  return NULL;
}

Item* parse_structure_item(Parser* parser) {
  Debug();
  Token top = Current();
  expect(Tkn_Struct);
  SourceLoc loc = loc_from_token(parser, top);
  if(check(Tkn_Identifier)) {
    Ident* name = parse_ident(parser);

    expect(Tkn_OpenBracket);
    Item** fields = NULL;
    for(;;) {
      if(check(Tkn_CloseBracket)) break;

      if(!check(Tkn_Identifier)) {
        syntax_error(loc_from_token(parser, Current()), "execting an identifer following '{'\n");
        sync(parser);
        break;
      }

      Item* field = parse_field_item(parser);
      buf_push(fields, field);
      loc = expand_loc(loc, field->loc);

      if(!match(Tkn_Comma))
        break;

    }
    expect(Tkn_CloseBracket);
    return new_itemstruct(name, fields, loc);
  }
  syntax_error(loc_from_token(parser, Current()), "expecting ident following keyword 'struct'\n");
  return NULL;
}

Item* parse_tuple_struct(Parser* parser) {
  Debug();
  // type is already parsed.

  if(check(Tkn_Identifier)) {
    Ident* name = parse_ident(parser);
    SourceLoc loc = name->loc;
    if(check(Tkn_OpenParen)) {
      Consume();
      loc.span += 1;

      TypeSpec** types = NULL;
      bool expecting_type = true;
      while(expecting_type) {
        expecting_type = false;
        TypeSpec* type = parse_typespec(parser);
        if(type) {
          buf_push(types, type);
          loc = expand_loc(loc, type->loc);
        }
        else {
          syntax_error(loc_from_token(parser, Current()), "Expecting type following comma in tuple struct\n");
          break;
        }

        if(match(Tkn_Comma))
          expecting_type = true;
      }
      expect(Tkn_CloseParen);
      return new_itemtuplestruct(name, types, loc);
    }
  }
  return NULL;
}

Item* parse_enum_elem(Parser* parser) {
  Debug();
  if(check(Tkn_Identifier)) {
    if(Next().kind == Tkn_OpenParen)
      return parse_tuple_struct(parser);
    else {
      Ident* name = parse_ident(parser);
      Expr* value = NULL;
      if(match(Tkn_Equal))
        value = parse_expr(parser);
      return new_itemname(name, value, expand_loc(name->loc, value ? value->loc : (SourceLoc) {0, 0, 0, 0}));
    }
  }
  else {
    expect(Tkn_Identifier);
    sync(parser);
  }
  return NULL;
}

Item* parse_enum_item(Parser* parser) {
  Debug();
  Token top = Current();
  SourceLoc loc = loc_from_token(parser, top);

  expect(Tkn_Enum);

  if(check(Tkn_Identifier)) {
    Ident* name = parse_ident(parser);
    loc = expand_loc(loc, name->loc);
    Item** body = NULL;
    if(check(Tkn_OpenBracket)) {
      Consume();
      while(!check(Tkn_CloseBracket)) {
        Item* elem = parse_enum_elem(parser);

        if(elem) {
          buf_push(body, elem);
          loc = expand_loc(loc, elem->loc);
        }

        match(Tkn_Comma);
      }
      expect(Tkn_CloseBracket);

      return new_itemenum(name, body, loc);
    }
    else {
      syntax_error(loc_from_token(parser, Current()), "execting '{' in enum declaration\n");
      sync(parser);
    }
  }
  else {
    syntax_error(loc_from_token(parser, Current()), "execting identifier following keyword 'enum'\n");
    sync(parser);
  }
  return NULL;
}

Item* parse_use_item(Parser* parser) {
  Debug();

  if(!match(Tkn_Semicolon)) {
    syntax_error(loc_from_token(parser, Current()), "expecting ';' following variable declaration\n");
  }
  return NULL;
}

Item* parse_module_item(Parser* parser) {
  Debug();

  if(!match(Tkn_Semicolon)) {
    syntax_error(loc_from_token(parser, Current()), "expecting ';' following variable declaration\n");
  }
  return NULL;
}

// field items are only used as a sub item for other items.
// used by function and structure.
// similar to local.
// <ident> <type-annotation>? <initialization>?
// x: i32
// x = 1
// x: i32 = 1
Item* parse_field_item(Parser* parser) {
  Debug();
  if(check(Tkn_Identifier)) {
    Ident* ident = parse_ident(parser);
    SourceLoc loc = ident->loc;
    TypeSpec* spec = NULL;
    Expr* init = NULL;
    if(match(Tkn_Colon)) {
      spec = parse_typespec(parser);
      loc = expand_loc(loc, spec->loc);
    }

    if(match(Tkn_Equal)) {
      init = parse_expr(parser);
      loc = expand_loc(loc, init->loc);
    }

    if(!spec and !init)
      syntax_error(loc_from_token(parser, Current()), "a type or initial value must be given\n");

    return new_itemfield(spec, ident, init, loc);
  }
  // syntax_error
  // sync(parser);
  return NULL;
}

Item* parse_type_or_tuplestruct(Parser* parser) {
  Debug();

  SourceLoc loc = loc_from_token(parser, Current());
  expect(Tkn_Type);

  if(check(Tkn_Identifier)) {
    if(Next().kind == Tkn_OpenParen)
      return parse_tuple_struct(parser);
    else if(Next().kind == Tkn_Equal) {
      Ident* name = parse_ident(parser);
      expect(Tkn_Equal);
      loc = expand_loc(loc, name->loc);
      TypeSpec* type = parse_typespec(parser);
      loc = expand_loc(loc, type->loc);
      return new_itemalias(name, type, loc);
    }
    else {
      syntax_error(loc_from_token(parser, Current()), "expecting '(' or '='\n");
    }
  }
  else {
    return NULL;
  }
  return NULL;
}


void sync(Parser* parser) {

}

void parse_test(File* file) {
  StringTable* table = (StringTable*) malloc(sizeof(StringTable));
  *table = create_table(TABLE_START);

  Parser parser = new_parser(file, table);


  // Expr* expr = parse_expr(&parser);
  Stmt* stmt = parse_stmt(&parser);
  // print_pattern(pat);
  // print_expr(expr);
  print_stmt(stmt);
}


AstFile* parse_file(File* file, StringTable* table) {
  AstFile* ast = new_ast_file(file);
  // this works but might be bad practice

  assert(table);

  Parser parser = new_parser(ast->file, table);
  while(!check_(&parser, Tkn_Eof)) {
    Item* item = parse_item(&parser);
    if(item)
      add_item(ast, item);
    else {
      syntax_error(loc_from_token(&parser, *parser.current), "invalid declaration in file scope\n");
      break;
    }
  }

  return ast;
}

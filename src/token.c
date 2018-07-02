#include "token.h"

char* token_strings[] = {
  #define TOKEN_KIND(name, str) str,
  TOKEN_KINDS
  #undef TOKEN_KIND
};

Token new_token(TokenKind kind, u64 line, u64 column, u64 span, u64 index) {
  Token token;
  memset(&token.literal, 0, sizeof(token.literal));
  token.kind = kind;
  token.line = line;
  token.column = column;
  token.span = span;
  token.index = index;
  token.type = NoType;
  return token;
}

Token new_integer_token(u64 value, u64 line, u64 column, u64 span, u64 index) {
  Token token = new_token(Tkn_IntLiteral, line, column, span, index);
  token.literal.value_i64 = value;
  return token;
}

Token new_float_token(f32 value, u64 line, u64 column, u64 span, u64 index) {
  Token token = new_token(Tkn_FloatLiteral, line, column, span, index);
  token.literal.value_float = value;
  return token;
}

Token new_string_token(const char* value, u32 len, u64 line, u64 column, u64 span, u64 index) {
  Token token = new_token(Tkn_StrLiteral, line, column, span, index);
  token.literal.value_string.value = (char*) value;
  token.literal.value_string.len = len;
  return token;
}

Token new_identifier_token(const char* value, u32 len, u64 line, u64 column, u64 span, u64 index) {
  Token token = new_token(Tkn_Identifier, line, column, span, index);
  token.literal.value_string.value = (char*) value;
  token.literal.value_string.len = len;
  return token;
}

Token new_char_token(char value, u64 line, u64 column, u64 span, u64 index) {
  Token token = new_token(Tkn_CharLiteral, line, column, span, index);
  token.literal.value_char = value;
  return token;
}

Token new_integer_token_type(u64 value, u64 line, u64 column, u64 span, u64 index, ExpectedType type) {
  Token token = new_integer_token(value, line, column, span, index);
  token.type = type;
  return token;
}

Token new_float_token_type(f32 value, u64 line, u64 column, u64 span, u64 index, ExpectedType type) {
  Token token = new_float_token(value, line, column, span, index);
  token.type = type;
  return token;
}

Token new_char_token_type(char value, u64 line, u64 column, u64 span, u64 index, ExpectedType type) {
  Token token = new_char_token(value, line, column, span, index);
  token.type = type;
  return token;
}

const char* get_token_string(Token* token) {
  if(is_literal(token) or is_identifier(token)) {
    switch(token->kind) {
      case Tkn_IntLiteral:
      case Tkn_FloatLiteral:
      case Tkn_CharLiteral:
        return token->string;
      case Tkn_StrLiteral:
      case Tkn_Identifier:
        return token->literal.value_string.value;
      default:
        break;
    }
  }
  
  return token_strings[token->kind];
}

i64 get_integer(Token* token) {
  return token->literal.value_i64;
}

f64 get_float(Token* token) {
  return token->literal.value_float;
}

const char* get_string(Token* token) {
  return token->literal.value_string.value;
}

const char** all_token_strings() {
  return (const char**) token_strings; 
}

bool is_literal(Token* token) {
  return (token->kind >= Tkn_IntLiteral and token->kind <= Tkn_CharLiteral) ||
          token->kind == Tkn_True || token->kind == Tkn_False;
}

bool is_operator(Token* token) {
  return Tkn_Plus <= token->kind and token->kind <= Tkn_PipeEqual;
}

bool is_assignment(Token* token) {
  return Tkn_Equal <= token->kind and token->kind <= Tkn_PipeEqual;
}

bool is_identifier(Token* token) {
  return token->kind == Tkn_Identifier;
}

u32 precedence(Token* token) {
 switch(token->kind) {
   // left associative operator
    case Tkn_AstrickAstrick:
      return 12;
    case Tkn_Slash:
    case Tkn_Astrick:
    case Tkn_Percent:
      return 11;
    case Tkn_Plus:
    case Tkn_Minus:
      return 10;
    case Tkn_LessLess:
    case Tkn_GreaterGreater:
      return 9;
    case Tkn_EqualEqual:
    case Tkn_BangEqual:
      return 8;
    case Tkn_LessEqual:
    case Tkn_GreaterEqual:
    case Tkn_Less:
    case Tkn_Greater:
      return 7;
    case Tkn_Ampersand:
      return 6;
    case Tkn_Carrot:
      return 5;
    case Tkn_Pipe:
      return 4;
    case Tkn_And:
      return 3;
    case Tkn_Or:
      return 2;
   // case Tkn_Question:
   // case Tkn_PeriodPeriod:
   // case Tkn_PeriodPeriodPeriod:
   // case Tkn_InfixOp:
   //   return 2;
    case Tkn_Equal:
    case Tkn_LessLessEqual:
    case Tkn_GreaterGreaterEqual:
    case Tkn_PlusEqual:
    case Tkn_MinusEqual:
    case Tkn_SlashEqual:
    case Tkn_AstrickEqual:
    case Tkn_AmpersandEqual:
    case Tkn_PipeEqual:
    case Tkn_CarrotEqual:
    case Tkn_AstrickAstrickEqual:
    case Tkn_PeriodPeriod:
      return 1;
    default:
      return 0;
  } 
}

Assoc associative(Token* token) {
  if(token->kind == Tkn_AstrickAstrick)
    return Right;
  return Left;
}

void set_string(Token* token, const char* value) {
  token->string = value;
  token->string_len = strlen(value);
}

TokenKind find_keyword(const char* str) {
  // if is the first keyword
  for(u8 i = (u8) Tkn_If; i < Num_Tokens; ++i)
    if(strcmp(str, token_strings[i]) == 0)
      return (TokenKind) i;

  return Tkn_None;
}

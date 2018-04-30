#ifndef TOKEN_H_
#define TOKEN_H_

#include "common.h"

#define TOKEN_KINDS \
  TOKEN_KIND(Error, "error") \
  TOKEN_KIND(None, "none") \
  TOKEN_KIND(Comment, "Comment") \
  TOKEN_KIND(Eof, "EOF") \
  TOKEN_KIND(IntLiteral, "integer literal") \
  TOKEN_KIND(FloatLiteral, "float literal") \
  TOKEN_KIND(StrLiteral, "string literal") \
  TOKEN_KIND(CharLiteral, "character literal") \
  TOKEN_KIND(Identifier, "identifier") \
  TOKEN_KIND(OpenParen, "(") \
  TOKEN_KIND(CloseParen, ")") \
  TOKEN_KIND(OpenBrace, "[") \
  TOKEN_KIND(CloseBrace, "]") \
  TOKEN_KIND(OpenBracket, "{") \
  TOKEN_KIND(CloseBracket, "}") \
  TOKEN_KIND(Period, ".") \
  TOKEN_KIND(Comma, ",") \
  TOKEN_KIND(Colon, ":") \
  TOKEN_KIND(Semicolon, ";") \
  TOKEN_KIND(ColonColon, "::") \
  TOKEN_KIND(Dollar, "$") \
  TOKEN_KIND(At, "@") \
  TOKEN_KIND(Plus, "+") \
  TOKEN_KIND(Minus, "-") \
  TOKEN_KIND(Slash, "/") \
  TOKEN_KIND(Percent, "%") \
  TOKEN_KIND(Astrick, "*") \
  TOKEN_KIND(AstrickAstrick, "**") \
  TOKEN_KIND(LessLess, "<<") \
  TOKEN_KIND(GreaterGreater, ">>") \
  TOKEN_KIND(Ampersand, "&") \
  TOKEN_KIND(Pipe, "|") \
  TOKEN_KIND(Carrot, "^") \
  TOKEN_KIND(Tilde, "~") \
  TOKEN_KIND(Bang, "!") \
  TOKEN_KIND(Less, "<") \
  TOKEN_KIND(Greater, ">") \
  TOKEN_KIND(LessEqual, "<=") \
  TOKEN_KIND(GreaterEqual, ">=") \
  TOKEN_KIND(EqualEqual, "==") \
  TOKEN_KIND(BangEqual, "!=") \
  TOKEN_KIND(Equal, "=") \
  TOKEN_KIND(PlusEqual, "+=") \
  TOKEN_KIND(MinusEqual, "-=") \
  TOKEN_KIND(AstrickEqual, "*=") \
  TOKEN_KIND(SlashEqual, "/=") \
  TOKEN_KIND(PercentEqual, "%=") \
  TOKEN_KIND(AstrickAstrickEqual, "**=") \
  TOKEN_KIND(LessLessEqual, "<<=") \
  TOKEN_KIND(GreaterGreaterEqual, ">>=") \
  TOKEN_KIND(CarrotEqual, "^=") \
  TOKEN_KIND(AmpersandEqual, "&=") \
  TOKEN_KIND(PipeEqual, "|=") \
  TOKEN_KIND(If, "if") \
  TOKEN_KIND(Else, "else") \
  TOKEN_KIND(Let, "let") \
  TOKEN_KIND(Mut, "mut") \
  TOKEN_KIND(Fn, "fn") \
  TOKEN_KIND(Type, "type") \
  TOKEN_KIND(Struct, "struct") \
  TOKEN_KIND(Break, "break") \
  TOKEN_KIND(Continue, "continue") \
  TOKEN_KIND(Return, "return") \
  TOKEN_KIND(While, "while") \
  TOKEN_KIND(For, "for") \
  TOKEN_KIND(Match, "match") \
  TOKEN_KIND(Sizeof, "sizeof") \
  TOKEN_KIND(Alignof, "alignof") \
  TOKEN_KIND(When, "when") \
  TOKEN_KIND(Use, "use") \
  TOKEN_KIND(And, "and") \
  TOKEN_KIND(Or, "or") \
  TOKEN_KIND(True, "true") \
  TOKEN_KIND(False, "false")
  

typedef enum TokenKind {
  #define TOKEN_KIND(name, ...) Tkn_##name,
    TOKEN_KINDS
  #undef TOKEN_KIND
  Num_Tokens
} TokenKind;

typedef enum ExpectedType {
  NoType,
  I8,
  I16,
  I32,
  I64,
  U8,
  U16,
  U32,
  U64,
  F32,
  F64,
} ExpectedType;

typedef struct Token {
  union {
    i64 value_i64;
    f64 value_float;
    char value_char;
    struct {
      char* value;
      u32 len;
    } value_string;
  } literal;
  
  // this is only used for numberic literals
  // it stores the original string value.
  const char* string; // this will be set explicitly
  u32 string_len;

  TokenKind kind;
  ExpectedType type;
  
  u64 line;
  u64 column;
  u64 span;
  u64 index;
} Token;

Token new_token(TokenKind kind, u64 line, u64 column, u64 span, u64 index);
Token new_integer_token(u64 value, u64 line, u64 column, u64 span, u64 index);
Token new_float_token(f32 value, u64 line, u64 column, u64 span, u64 index);
Token new_string_token(const char* value, u32 len, u64 line, u64 column, u64 span, u64 index);
Token new_identifier_token(const char* value, u32 len, u64 line, u64 column, u64 span, u64 index);
Token new_char_token(char value, u64 line, u64 column, u64 span, u64 index);
Token new_integer_token_type(u64 value, u64 line, u64 column, u64 span, u64 index, ExpectedType type);
Token new_float_token_type(f32 value, u64 line, u64 column, u64 span, u64 index, ExpectedType type);
Token new_char_token_type(char value, u64 line, u64 column, u64 span, u64 index, ExpectedType type);

void set_string(Token* token, const char* value);
const char* get_token_string(Token* token);

i64 get_integer(Token* token);
f32 get_float32(Token* token);
f64 get_float64(Token* token);
const char* get_string(Token* token);

const char** all_token_strings();

bool is_literal(Token* token);
bool is_identifier(Token* token);
bool is_operator(Token* token);
bool is_assignment(Token* token);

typedef enum Assoc {
  Left,
  Right
} Assoc;

u32 precedence(Token* token);
Assoc associative(Token* token);

TokenKind find_keyword(const char* str);

#endif

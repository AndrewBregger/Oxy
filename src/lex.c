#include "lex.h"
#include "token.h"
#include "print.h"
#include "report.h"
#include <assert.h>
#include <ctype.h>

typedef struct Scanner Scanner;

Token scan_token(Scanner* scanner);

const char* substr(const char* src, u64 start, u64 len);
ExpectedType scan_literal_suffix(Scanner* scanner);

const char* table_get_string(StringTable* table, char* string) {
  u64 index = 0;
  if(table_contains(table, string)) {
    u64 hash = string_hash(string, strlen(string));
    u64 index = hash % table->cap;
    const char* temp = table->strings[index];
    buf_free(string);
    return temp;
  }
  else {
    u32 len = buf_len(string);
    char* temp = (char*) malloc(sizeof(char) * (len + 1));
    memcpy(temp, string, len);
    temp[len] = 0;
    buf_free(string);

    const char* val = table_insert_string(table, temp);
    free(temp);
    return val;
  }
}

typedef struct Scanner {
  u64 index;
  u64 line;
  u64 column;
  const char* source;
  u64 len;
  File* file;
  StringTable* table;
} Scanner;

Scanner new_scanner(File* file, StringTable* table) {
  Scanner context;
  context.file = file;
  context.index  = 0;
  context.line   = 1;
  context.column = 1;
  context.source = (const char*) file->content;
  context.len = file->len;
  context.table = table;
  return context;
}

Token* get_tokens(File* file, u32* num, StringTable* table) {
  Token* tokens = NULL;
  Scanner scanner = new_scanner(file, table);
  assert(scanner.table->cap == 1024);
  while(true) {
    Token token = scan_token(&scanner);
    print_token(&token);
    buf_push(tokens, token);
    if(token.kind == Tkn_Eof)
      break;

  }
  *num = buf_len(tokens);
  return tokens;
}

bool is_eof(Scanner* scanner);
Token scan_identifier(Scanner* scanner);
Token scan_string(Scanner* scanner);
Token scan_number(Scanner* scanner, u64 start);
Token scan_character(Scanner* scanner);
ExpectedType scan_literal_suffix(Scanner* scanner);
char validate_escape(Scanner* scanner);
Token scan_comment(Scanner* scanner);

void advance(Scanner* scanner);

// Gets the current character of the scanner
#define Current(scanner) (scanner)->source[(scanner)->index]
#define Check(ch, scanner) (ch) == Current((scanner))
// Builds a generic token
#define BUILD_TOKEN(tok, scanner) new_token(tok, (scanner)->line, (scanner)->column, 1, (scanner)->index)


bool is_character(char ch) {
  return isalpha(ch);
}

bool is_number(char ch) {
  return isdigit(ch);
}

#define DoubleToken(ch, kind1, kind2) \
	case ch: \
		if(Current(scanner) == '=') { \
			advance(scanner); \
			return new_token(kind2, save.line, save.column, 2, save.index); \
		} \
		else { \
			return new_token(kind1, save.line, save.column, 1, save.index); \
		} \
		break;

#define TripleToken(ch, kind1, kind2, kind3) \
	case ch: \
		if(Current(scanner) == '=') { \
			advance(scanner); \
			return new_token(kind2, save.line, save.column, 2, save.index); \
		} \
		else if(Current(scanner) == (ch)) { \
			advance(scanner); \
			return new_token(kind3, save.line, save.column, 2, save.index); \
		} \
		else { \
			return new_token(kind1, save.line, save.column, 1, save.index); \
		} \
		break;

#define FourToken(ch, kind1, kind2, kind3, kind4) \
	case ch: {\
		if(Current(scanner) == '=') { \
			advance(scanner); \
			return new_token(kind2, save.line, save.column, 2, save.index); \
		} \
		else if(Current(scanner) == (ch)) { \
			advance(scanner); \
			if(Current(scanner) == '=') { \
			  advance(scanner); \
        return new_token(kind4, save.line, save.column, 2, save.index); \
			} \
			else { \
        return new_token(kind3, save.line, save.column, 1, save.index); \
			} \
		} \
		else { \
      return new_token(kind1, save.line, save.column, 1, save.index); \
		} \
	} break;

Token scan_token(Scanner* scanner) {
  assert(scanner->table->cap == 1024);
  while(Current(scanner) == '\t' or
        Current(scanner) == ' '  or
        Current(scanner) == '\v' or
        Current(scanner) == '\n')
    advance(scanner);
  if(is_eof(scanner))
    return BUILD_TOKEN(Tkn_Eof, scanner);

  if(is_character(Current(scanner)))
    return scan_identifier(scanner);
  else if(is_number(Current(scanner)))
    return scan_number(scanner, scanner->index);
  else {
    char ch = Current(scanner);
    Scanner save = *scanner;
    advance(scanner);
    switch(ch) {
      // case '\n': return BUILD_TOKEN(Tkn_Newline, &save);
      case '(': return BUILD_TOKEN(Tkn_OpenParen, &save);
      case ')': return BUILD_TOKEN(Tkn_CloseParen, &save);
      case '[': return BUILD_TOKEN(Tkn_OpenBrace, &save);
      case ']': return BUILD_TOKEN(Tkn_CloseBrace, &save);
      case '{': return BUILD_TOKEN(Tkn_OpenBracket, &save);
      case '}': return BUILD_TOKEN(Tkn_CloseBracket, &save);
      case ',': return BUILD_TOKEN(Tkn_Comma, &save);
      case '~': return BUILD_TOKEN(Tkn_Tilde, &save);
      case ';': return BUILD_TOKEN(Tkn_Semicolon, &save);
      case '_': return BUILD_TOKEN(Tkn_Underscore, &save);
      case ':': {
        if(Check(':', scanner)) {
          advance(scanner);
          return new_token(Tkn_ColonColon, save.line, save.column, 2, save.index);
        }
        return BUILD_TOKEN(Tkn_Colon, &save);
      }
      case '.': {
        if(Check('.', scanner)) {
          advance(scanner);
          return new_token(Tkn_PeriodPeriod, save.line, save.column, 2, save.index);
        }
        return BUILD_TOKEN(Tkn_Period, &save);
      }
      DoubleToken('^', Tkn_Carrot, Tkn_CarrotEqual)
      DoubleToken('%', Tkn_Percent, Tkn_PercentEqual)
      DoubleToken('!', Tkn_Bang, Tkn_BangEqual)
      DoubleToken('=', Tkn_Equal, Tkn_EqualEqual)
      DoubleToken('+', Tkn_Plus, Tkn_PlusEqual)
      DoubleToken('-', Tkn_Minus, Tkn_MinusEqual)

      TripleToken('|', Tkn_Pipe, Tkn_PipeEqual, Tkn_Or)
      TripleToken('&', Tkn_Ampersand, Tkn_AmpersandEqual, Tkn_And)

      FourToken('*', Tkn_Astrick, Tkn_AstrickEqual, Tkn_AstrickAstrick, Tkn_AstrickAstrickEqual)
      FourToken('<', Tkn_Less, Tkn_LessEqual, Tkn_LessLess, Tkn_LessLessEqual)
      FourToken('>', Tkn_Greater, Tkn_GreaterEqual, Tkn_GreaterGreater, Tkn_GreaterGreaterEqual)
      case '/': {
        if(Current(scanner) == '/' or Current(scanner) == '*')
          return scan_comment(scanner);
        else if(Current(scanner) == '=')
          return new_token(Tkn_SlashEqual, save.line, save.column, 2, save.index);
        else
          return BUILD_TOKEN(Tkn_Slash, &save);
      }
      case '\'': {
        return scan_character(scanner);
      }
      case '"': {
        return scan_string(scanner);
      }
      default:
        scan_error(scanner->file, scanner->line, scanner->column, "unrecognized character: '%c'\n", Current(scanner));
    }
  }
  return BUILD_TOKEN(Tkn_Error, scanner);
}

Token scan_identifier(Scanner* scanner) {
  Scanner save = *scanner;
  u64 start = scanner->index;
  while(isalnum(Current(scanner)) or Current(scanner) == '_')
    advance(scanner);

  const char* str = substr(scanner->source, start, scanner->index - start);

  // clean up the string if it isnt used by the table

  TokenKind kind = find_keyword(str);
  if(kind == Tkn_None) {
    const char* temp = table_insert_string(scanner->table, str);
    // printf("What is the pointer here: %p\n", (void*) temp);
    free((void*) str);
    return new_identifier_token(temp, scanner->index - start, save.line, save.column, scanner->index - start, save.index);
  }
  else {
    free((void*) str);
    return new_token(kind, save.line, save.column, strlen(all_token_strings()[kind]),
                     save.index);
  }
}

Token scan_string(Scanner* scanner) {
  Scanner save = *scanner;
  char* buffer = NULL;
  while(Current(scanner) != '"') {
    if(Current(scanner) == '\\')
      buf_push(buffer, validate_escape(scanner));
    else
      buf_push(buffer, Current(scanner));

    advance(scanner);
  }
  // buf_push(buffer, '\0');
  u64 size = scanner->index - save.index;
  advance(scanner);
  const char* str = table_get_string(scanner->table, buffer);
  return new_string_token(buffer, size, save.line, save.column, size, save.index);
}


Token scan_number(Scanner* scanner, u64 start) {
  Scanner s = *scanner;
  u32 base = 10;
  if(Current(scanner) == '0') {
    advance(scanner);

    if(Current(scanner) == 'x' or
       Current(scanner) == 'X') {
      advance(scanner);
      Scanner save = *scanner;
      base = 16;
      while(isxdigit(Current(scanner)))
        advance(scanner);

      const char* str = substr(scanner->source, save.index, scanner->index - save.index);
      u64 val = strtoll(str, NULL, 16);
      return new_integer_token(val, s.line, s.column, scanner->index - start, s.index);
    }
    else if(Current(scanner) == 'b' or
            Current(scanner) == 'B') {
      advance(scanner);
      Scanner save = *scanner;
      base = 2;
      while(Current(scanner) == '1' or
            Current(scanner) == '0')
        advance(scanner);

      const char* str = substr(scanner->source, save.index, scanner->index - save.index);
      u64 val = strtoll(str, NULL, 2);
      return new_integer_token(val, s.line, s.column, scanner->index - start, s.index);
    }
  }
  bool floating_point = false;
  bool scientific_notation = false;
  while(isdigit(Current(scanner)))
    advance(scanner);
  if(Current(scanner) == '.' and (scanner->index + 1 < scanner->len and scanner->source[scanner->index + 1] != '.')) {
    advance(scanner);
    while(isdigit(Current(scanner)))
      advance(scanner);
    floating_point = true;
  }
  if(Current(scanner) == 'e' or
     Current(scanner) == 'E') {
    advance(scanner);
    if(Current(scanner) == '-' or
       Current(scanner) == '+')
      advance(scanner);

    if(isdigit(Current(scanner)))
      while(isdigit(Current(scanner)))
        advance(scanner);
    else {
      // report errors
      scan_error(scanner->file, scanner->line, scanner->column,
        "Missing exponent in float literal\n");
    }
  }
  const char* tmp = substr(scanner->source, start, scanner->index - start);
  ExpectedType type = scan_literal_suffix(scanner);
  Token token;
  if(floating_point) {
    f64 val = strtod(tmp, NULL);
    if(type == NoType)
      token = new_float_token(val, s.line, s.column, scanner->index - start, s.index);
    else
      token = new_float_token_type(val, s.line, s.column, scanner->index - start, s.index, type);
  }
  else {
    i64 val = strtoll(tmp, NULL, 10);
    if(type == NoType)
      token = new_integer_token(val, s.line, s.column, scanner->index - start, s.index);
    else
      token = new_integer_token_type(val, s.line, s.column, scanner->index - start, s.index, type);
  }
  const char* n = table_insert_string(scanner->table, tmp);
  free((void*) tmp);
  set_string(&token, n);
  return token;
}

Token scan_character(Scanner* scanner) {
  char temp;
  Scanner save = *scanner;
  switch(Current(scanner)) {
    case '\\':
      temp = validate_escape(scanner);
      break;
    default:
      temp = Current(scanner);
  }
  return new_char_token(temp, save.line, save.column, 1, save.index);
}

Token scan_comment(Scanner* scanner) {
  Scanner save = *scanner;
  // the actual start is the previous character;
  u64 start = save.index - 1;
  if(Current(scanner) == '/') {
    while(!is_eof(scanner) and Current(scanner) != '\n')
      advance(scanner);
    // takes the new line

    return scan_token(scanner);
    const char* src = substr(scanner->source, start, scanner->index - start);
    advance(scanner);
    Token token = new_token(Tkn_Comment, save.line, save.column, scanner->index - start, start);
    token.literal.value_string.value = (char*) src;
    token.literal.value_string.len = scanner->index - start;
    return token;
  }
  else if(Current(scanner) == '*') {
		int count = 1;
		advance(scanner);
		while(count) {
			if(is_eof(scanner)) {
				// report_scanner_error(filename, cursor, "failed to close block comment\n");
        scan_error(scanner->file, scanner->line, scanner->column, "failed to close block comment\n");
				break;
			}
			else if(Current(scanner) == '/') {
        advance(scanner);
				if(Current(scanner) == '*') {
					advance(scanner);
					++count;
				}
			}
			else if(Current(scanner) == '*') {
				advance(scanner);
				if(Current(scanner) == '/') {
					advance(scanner);
					--count;
				}
			}
			else {
				advance(scanner);
			}
		}

    return scan_token(scanner);

    const char* src = substr(scanner->source, start, scanner->index - start);
    Token token = new_token(Tkn_Comment, save.line, save.column, scanner->index - start, start);
    token.literal.value_string.value = (char*) src;
    token.literal.value_string.len = scanner->index - start;
    return token;
  }

  return BUILD_TOKEN(Tkn_Error, scanner);
}


char validate_escape(Scanner* scanner) {
  advance(scanner);
	switch(Current(scanner)) {
		case 'a':
			return 0x07;
		case 'b':
			return 0x08;
		case 'f':
			return 0x0C;
		case 'n':
			return 0x0A;
		case 'r':
			return 0x0D;
		case 't':
			return 0x09;
		case 'v':
			return 0x0B;
		case '\\':
			return 0x5C;
		case '\'':
			return 0x27;
		case '"':
			return 0x22;
		case '?':
			return 0x3F;
      /*
		case 'x':
			bump();
			while(isxdigit(curr)) bump();
			str = cursor->substr(start);
			pos.span = str.size();
			return EscapeCharecterType::Hex;
		case 'e':
			str.push_back(0x1B);
			return EscapeCharecterType::Character;
		case 'U':
			bump();
			while(isxdigit(curr)) bump();
			str = cursor->substr(start);
			pos.span = str.size();
			return EscapeCharecterType::UnicodeLong;
		case 'u':
			bump();
			while(isxdigit(curr)) bump();
			character = cursor->substr(start);
			pos.span = character.size();
			return EscapeCharecterType::UnicodeShort;
    */
		default:
			scan_error(scanner->file, scanner->line, scanner->column, "invalid escape character: %c\n", Current(scanner));
    	// next();
      break;
	}
  return 0x0;
}

void advance(Scanner* scanner) {
  if(!is_eof(scanner)) {
    if(Current(scanner) == '\n') {
      scanner->column = 0;
      scanner->line++;
    }
    scanner->index++;
    scanner->column++;
  }
}

bool is_eof(Scanner* scanner) {
  return Current(scanner) == '\0' or scanner->index >= scanner->len;
}

ExpectedType scan_literal_suffix(Scanner* scanner) {
  Scanner save = *scanner;
  // if it isnt i, f, or u then ignore it.
  if(Current(scanner) == 'i' or
     Current(scanner) == 'f' or
     Current(scanner) == 'u') {
    Token token = scan_identifier(scanner);
    //print_token(&token);
    const char* val = get_string(&token);
    ExpectedType type = NoType;
    if(strcmp(val, "i8") == 0)
      type = I8;
    else if(strcmp(val, "i16") == 0)
      type = I16;
    else if(strcmp(val, "i16") == 0)
      type = I32;
    else if(strcmp(val, "i16") == 0)
      type = I64;
    else if(strcmp(val, "u86") == 0)
      type = U8;
    else if(strcmp(val, "u16") == 0)
      type = U16;
    else if(strcmp(val, "u32") == 0)
      type = U32;
    else if(strcmp(val, "u64") == 0)
      type = U64;
    else if(strcmp(val, "f32") == 0)
      type = F32;
    else if(strcmp(val, "f64") == 0)
      type = F64;
    else
      *scanner = save;
    return type;
  }
  return NoType;
}

const char* substr(const char* src, u64 start, u64 len) {
  char* temp = (char*) malloc(sizeof(char) * (len + 1));
  memcpy(temp, &src[start], len);
  temp[len] = '\0';
  return temp;
}

void scan_test(File* file) {
  u32 num;
  StringTable table = create_table(TABLE_START);
  Token* tokens = get_tokens(file, &num, &table);
  for(u32 i = 0; i < num; ++i)
    print_literal(tokens + i);
}


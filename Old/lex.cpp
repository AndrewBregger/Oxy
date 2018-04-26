#include <vector>
#include <string>

#include "lex.h"
#include "print.h"
#include <iostream>

const char* substr(const char* src, u64 start, u64 len);

struct ScanContext {
  u64 index;
  u64 line;
  u64 column;
  const char* source;
  u64 len;
};

ScanContext new_scanner(File* file) {
  ScanContext context;
  context.index  = 0;
  context.line   = 1;
  context.column = 1;
  context.source = file->content.c_str();
  context.len = file->content.size();
  return context;
}


Token scan_token(ScanContext* scanner);

std::vector<Token> scan_tokens(File* file) {
  ScanContext scanner = new_scanner(file);
  std::vector<Token> tokens;

  while(true) {
    Token token = scan_token(&scanner);
    tokens.push_back(token);

    if(token.kind == Tkn_Eof)
      break;
  }

  return tokens;
}


bool scan_eof(ScanContext* scanner);
Token scan_identifier(ScanContext* scanner);
Token scan_string(ScanContext* scanner);
Token scan_number(ScanContext* scanner, u64 start);
Token scan_character(ScanContext* scanner);
ExpectedType scan_literal_suffix(ScanContext* scanner);
char validate_escape(ScanContext* scanner);

void advance(ScanContext* scanner);

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

Token scan_token(ScanContext* scanner) {
  while(Current(scanner) == '\t' or
        Current(scanner) == ' '  or
        Current(scanner) == '\v')
    advance(scanner);
  if(scan_eof(scanner))
    return BUILD_TOKEN(Tkn_Eof, scanner);

  if(is_character(Current(scanner)))
    return scan_identifier(scanner);
  else if(is_number(Current(scanner)))
    return scan_number(scanner, scanner->index);
  else {
    char ch = Current(scanner);
    ScanContext save = *scanner;
    advance(scanner);
    switch(ch) {
      case '\n': return BUILD_TOKEN(Tkn_Newline, &save);
      case '(': return BUILD_TOKEN(Tkn_OpenParen, &save);
      case ')': return BUILD_TOKEN(Tkn_CloseParen, &save);
      case '[': return BUILD_TOKEN(Tkn_OpenBrace, &save);
      case ']': return BUILD_TOKEN(Tkn_CloseBrace, &save);
      case '{': return BUILD_TOKEN(Tkn_OpenBracket, &save);
      case '}': return BUILD_TOKEN(Tkn_CloseBracket, &save);
      case ',': return BUILD_TOKEN(Tkn_Comma, &save);
      case '~': return BUILD_TOKEN(Tkn_Tilde, &save);
      case ':': {
        if(Check(':', scanner)) {
          advance(scanner);
          return new_token(Tkn_ColonColon, save.line, save.column, 2, save.index);
        }
        return BUILD_TOKEN(Tkn_Colon, &save);
      }
      case '.': {
        if(isdigit(Current(scanner))) {
          Token token = scan_number(&save, save.index);
          *scanner = save;
          return token;
        }
        else
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
      case '\'': {
        return scan_character(scanner);
      }
      case '"': {
        return scan_string(scanner);
      }
    }
  }
}

Token scan_identifier(ScanContext* scanner) {
  ScanContext save = *scanner;
  u64 start = scanner->index;
  while(isalnum(Current(scanner)) or Current(scanner) == '_')
    advance(scanner);

  const char* str = substr(scanner->source, start, scanner->index - start);
  const auto& strings = all_token_strings();
  u64 index = (u64) -1;
  std::cout << std::string(str) << std::endl;
  auto iter = std::find(strings.begin(), strings.end(), std::string(str));
  if(iter != strings.end())
    index = std::distance(strings.begin(), iter);

  printf("%llu\n", index);
  if(index == (u64) -1)
    return new_identifier_token(str, scanner->index - start, save.line, save.column, scanner->index - start, save.index);
  else
    return new_token((TokenKind) index, save.line, save.column, strings[index].size(), save.index);
}

Token scan_string(ScanContext* scanner) {
  std::string buffer;
  ScanContext save = *scanner;
  while(Current(scanner) != '"') {
    if(Current(scanner) == '\\')
      buffer.push_back(validate_escape(scanner));
    else
      buffer.push_back(Current(scanner));
    advance(scanner);
  }
  advance(scanner);

  char* temp = new char[buffer.size() +  1];
  strncpy(temp, buffer.c_str(), buffer.size());
  return new_string_token(temp, buffer.size(), save.line, save.column, buffer.size(), save.index);
}

Token scan_number(ScanContext* scanner, u64 start) {
  ScanContext s = *scanner;
  if(Current(scanner) == '0') {
    advance(scanner);

    if(Current(scanner) == 'x' or
       Current(scanner) == 'X') {
      advance(scanner);
      ScanContext save = *scanner;
      while(isxdigit(Current(scanner)))
        advance(scanner);

      const char* str = substr(scanner->source, save.index, scanner->index - save.index);
      u64 val = strtoll(str, nullptr, 16);
      return new_integer_token(val, s.line, s.column, scanner->index - start, s.index);
    }
    else if(Current(scanner) == 'b' or
            Current(scanner) == 'B') {
      advance(scanner);
      ScanContext save = *scanner;
      while(Current(scanner) == '1' or
            Current(scanner) == '0')
        advance(scanner);

      const char* str = substr(scanner->source, save.index, scanner->index - save.index);
      printf("%s\n", str);
      u64 val = strtoll(str, nullptr, 2);
      return new_integer_token(val, s.line, s.column, scanner->index - start, s.index);
    }
  }
  bool floating_point = false;
  bool scientific_notation = false;
  while(isdigit(Current(scanner)))
    advance(scanner);
  if(Current(scanner) == '.') {
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
      printf("Missing exponent in literal\n");
    }
  }
  const char* tmp = substr(scanner->source, start, scanner->index - start);
  ExpectedType type = scan_literal_suffix(scanner);
  if(floating_point) {
    f64 val = strtod(tmp, nullptr);
    if(type == NoType)
      return new_float_token(val, s.line, s.column, scanner->index - start, s.index);
    else
      return new_float_token_type(val, s.line, s.column, scanner->index - start, s.index, type);
  }
  else {
    i64 val = strtoll(tmp, nullptr, 10);
    if(type == NoType)
      return new_integer_token(val, s.line, s.column, scanner->index - start, s.index);
    else
      return new_integer_token_type(val, s.line, s.column, scanner->index - start, s.index, type);
  }
}

Token scan_character(ScanContext* scanner) {
  char temp;
  ScanContext save = *scanner;
  switch(Current(scanner)) {
    case '\\':
      temp = validate_escape(scanner);
      break;
    default:
      temp = Current(scanner);
  }
  return new_char_token(temp, save.line, save.column, 1, save.index);
}

char validate_escape(ScanContext* scanner) {
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
			// report_scanner_error(filename, cursor, "invalid escape character: %c\n", character);
    	// next();
      break;
	}
}

void advance(ScanContext* scanner) {
  if(!scan_eof(scanner)) {
    if(Current(scanner) == '\n') {
      scanner->column = 0;
      scanner->line++;
    }
    scanner->index++;
    scanner->column++;
  }
}

bool scan_eof(ScanContext* scanner) {
  return Current(scanner) == '\0' or scanner->index >= scanner->len;
}

ExpectedType scan_literal_suffix(ScanContext* scanner) {
  ScanContext save = *scanner;
  // if it isnt i, f, or u then ignore it.
  if(Current(scanner) == 'i' or
     Current(scanner) == 'f' or
     Current(scanner) == 'u') {
    Token token = scan_identifier(scanner);
    printf("Printing Suffix: ");
    print_token(&token);
    std::string val = get_string(&token);
    ExpectedType type = NoType;
    if(val == "i8")
      type = I8;
    else if(val == "i16")
      type = I16;
    else if(val == "i32")
      type = I32;
    else if(val == "i64")
      type = I64;
    else if(val == "u8")
      type = U8;
    else if(val == "u16")
      type = U16;
    else if(val == "u32")
      type = U32;
    else if(val == "u64")
      type = U64;
    else if(val == "f32")
      type = F32;
    else if(val == "f64")
      type = F64;
    else
      *scanner = save;
    return type;
  }
  return NoType;
}

const char* substr(const char* src, u64 start, u64 len) {
  char* temp = new char[len + 1];
  memcpy(temp, &src[start], len);
  temp[len] = '\0';
  return temp;
}

void scanner_test(File* file) {
  std::vector<Token> tokens = scan_tokens(file);

  for(auto& x : tokens)
    print_token(&x);
}

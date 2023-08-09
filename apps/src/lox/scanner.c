//> Scanning on Demand scanner-c
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
  const char* start;
  const char* current;
  int line;
} Scanner;

Scanner scanner;
//> init-scanner
void initScanner(const char* source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}
//< init-scanner
//> is-alpha
static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}
//< is-alpha
//> is-digit
static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}
//< is-digit
//> is-at-end
static bool isAtEnd() {
  return *scanner.current == '\0';
}
//< is-at-end
//> advance
static char advance_scanner() {
  scanner.current++;
  return scanner.current[-1];
}
//< advance
//> peek
static char peek_scanner() {
  return *scanner.current;
}
//< peek
//> peek-next
static char peekNext() {
  if (isAtEnd()) return '\0';
  return scanner.current[1];
}
//< peek-next
//> match
static bool match_scanner(char expected) {
  if (isAtEnd()) return false;
  if (*scanner.current != expected) return false;
  scanner.current++;
  return true;
}
//< match
//> make-token
static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}
//< make-token
//> error-token
static Token errorToken(const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}
//< error-token
//> skip-whitespace
static void skipWhitespace() {
  for (;;) {
    char c = peek_scanner();
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance_scanner();
        break;
//> newline
      case '\n':
        scanner.line++;
        advance_scanner();
        break;
//< newline
//> comment
      case '/':
        if (peekNext() == '/') {
          // A comment goes until the end of the line.
          while (peek_scanner() != '\n' && !isAtEnd()) advance_scanner();
        } else {
          return;
        }
        break;
//< comment
      default:
        return;
    }
  }
}
//< skip-whitespace
//> check-keyword
static TokenType checkKeyword(int start, int length,
    const char* rest, TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}
//< check-keyword
//> identifier-type
static TokenType identifierType() {
//> keywords
  switch (scanner.start[0]) {
    case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
//> keyword-f
    case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
          case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
        }
      }
      break;
//< keyword-f
    case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
//> keyword-t
    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
          case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
        }
      }
      break;
//< keyword-t
    case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }

//< keywords
  return TOKEN_IDENTIFIER;
}
//< identifier-type
//> identifier
static Token identifier() {
  while (isAlpha(peek_scanner()) || isDigit(peek_scanner())) advance_scanner();
  return makeToken(identifierType());
}
//< identifier
//> number
static Token number_scanner() {
  while (isDigit(peek_scanner())) advance_scanner();

  // Look for a fractional part.
  if (peek_scanner() == '.' && isDigit(peekNext())) {
    // Consume the ".".
    advance_scanner();

    while (isDigit(peek_scanner())) advance_scanner();
  }

  return makeToken(TOKEN_NUMBER);
}
//< number
//> string
static Token string_scanner() {
  while (peek_scanner() != '"' && !isAtEnd()) {
    if (peek_scanner() == '\n') scanner.line++;
    advance_scanner();
  }

  if (isAtEnd()) return errorToken("Unterminated string.");

  // The closing quote.
  advance_scanner();
  return makeToken(TOKEN_STRING);
}
//< string
//> scan-token
Token scanToken() {
//> call-skip-whitespace
  skipWhitespace();
//< call-skip-whitespace
  scanner.start = scanner.current;

  if (isAtEnd()) return makeToken(TOKEN_EOF);
//> scan-char
  
  char c = advance_scanner();
//> scan-identifier
  if (isAlpha(c)) return identifier();
//< scan-identifier
//> scan-number
  if (isDigit(c)) return number_scanner();
//< scan-number

  switch (c) {
    case '(': return makeToken(TOKEN_LEFT_PAREN);
    case ')': return makeToken(TOKEN_RIGHT_PAREN);
    case '{': return makeToken(TOKEN_LEFT_BRACE);
    case '}': return makeToken(TOKEN_RIGHT_BRACE);
    case ';': return makeToken(TOKEN_SEMICOLON);
    case ',': return makeToken(TOKEN_COMMA);
    case '.': return makeToken(TOKEN_DOT);
    case '-': return makeToken(TOKEN_MINUS);
    case '+': return makeToken(TOKEN_PLUS);
    case '/': return makeToken(TOKEN_SLASH);
    case '*': return makeToken(TOKEN_STAR);
//> two-char
    case '!':
      return makeToken(
          match_scanner('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
      return makeToken(
          match_scanner('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(
          match_scanner('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(
          match_scanner('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
//< two-char
//> scan-string
    case '"': return string_scanner();
//< scan-string
  }
//< scan-char

  return errorToken("Unexpected character.");
}
//< scan-token

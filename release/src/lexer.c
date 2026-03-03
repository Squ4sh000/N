#include "n_lang.h"

Lexer* createLexer(const char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->start = 0;
    lexer->current = 0;
    lexer->line = 1;
    return lexer;
}

bool isAtEnd(Lexer* lexer) {
    return lexer->source[lexer->current] == '\0';
}

char advance(Lexer* lexer) {
    return lexer->source[lexer->current++];
}

char peek(Lexer* lexer) {
    return lexer->source[lexer->current];
}

char peekNext(Lexer* lexer) {
    if (isAtEnd(lexer)) return '\0';
    return lexer->source[lexer->current + 1];
}

bool match(Lexer* lexer, char expected) {
    if (isAtEnd(lexer)) return false;
    if (lexer->source[lexer->current] != expected) return false;
    lexer->current++;
    return true;
}

Token makeToken(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    int length = lexer->current - lexer->start;
    token.lexeme = (char*)malloc(length + 1);
    strncpy(token.lexeme, lexer->source + lexer->start, length);
    token.lexeme[length] = '\0';
    token.line = lexer->line;
    return token;
}

Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !isAtEnd(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }

    if (isAtEnd(lexer)) {
        // Error: Unterminated string.
        return makeToken(lexer, TOKEN_EOF);
    }

    advance(lexer); // The closing "
    return makeToken(lexer, TOKEN_STRING);
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

Token number(Lexer* lexer) {
    while (isDigit(peek(lexer))) advance(lexer);

    if (peek(lexer) == '.' && isDigit(peekNext(lexer))) {
        advance(lexer);
        while (isDigit(peek(lexer))) advance(lexer);
    }

    return makeToken(lexer, TOKEN_NUMBER);
}

bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

TokenType checkKeyword(Lexer* lexer, int start, int length, const char* rest, TokenType type) {
    if (lexer->current - lexer->start == start + length &&
        memcmp(lexer->source + lexer->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

TokenType identifierType(Lexer* lexer) {
    switch (lexer->source[lexer->start]) {
        case 'c':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'o': return checkKeyword(lexer, 2, 3, "nst", TOKEN_CONST);
                    case 'l': return checkKeyword(lexer, 2, 3, "ass", TOKEN_CLASS);
                }
            }
            break;
        case 'e': return checkKeyword(lexer, 1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'n': return checkKeyword(lexer, 2, 0, "", TOKEN_FN);
                    case 'a': return checkKeyword(lexer, 2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(lexer, 2, 1, "r", TOKEN_FOR);
                }
            }
            break;
        case 'i':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'f': return checkKeyword(lexer, 2, 0, "", TOKEN_IF);
                    case 'm': return checkKeyword(lexer, 2, 4, "port", TOKEN_IMPORT);
                    case 'n': return checkKeyword(lexer, 2, 0, "", TOKEN_IN);
                }
            }
            break;
        case 'n': return checkKeyword(lexer, 1, 3, "ull", TOKEN_NULL);
        case 'r': return checkKeyword(lexer, 1, 5, "eturn", TOKEN_RETURN);
        case 't': return checkKeyword(lexer, 1, 3, "rue", TOKEN_TRUE);
        case 'v': return checkKeyword(lexer, 1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(lexer, 1, 4, "hile", TOKEN_WHILE);
        case 'l':
            if (lexer->current - lexer->start == 3 && memcmp(lexer->source + lexer->start + 1, "et", 2) == 0) {
                return TOKEN_VAR; // Allow 'let' as alias for 'var'
            }
            break;
        case 's':
            if (lexer->current - lexer->start == 4 && memcmp(lexer->source + lexer->start + 1, "elf", 3) == 0) {
                return TOKEN_IDENTIFIER; // self is a special identifier
            }
            break;
    }
    return TOKEN_IDENTIFIER;
}

Token identifier(Lexer* lexer) {
    while (isAlpha(peek(lexer)) || isDigit(peek(lexer))) advance(lexer);
    return makeToken(lexer, identifierType(lexer));
}

Token scanToken(Lexer* lexer) {
    // Skip whitespace and comments
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                advance(lexer);
                break;
            case '/':
                if (peekNext(lexer) == '/') {
                    while (peek(lexer) != '\n' && !isAtEnd(lexer)) advance(lexer);
                } else {
                    goto start_token;
                }
                break;
            default:
                goto start_token;
        }
    }

start_token:
    lexer->start = lexer->current;
    if (isAtEnd(lexer)) return makeToken(lexer, TOKEN_EOF);

    char c = advance(lexer);
    if (isAlpha(c)) return identifier(lexer);
    if (isDigit(c)) return number(lexer);

    switch (c) {
        case '(': return makeToken(lexer, TOKEN_LPAREN);
        case ')': return makeToken(lexer, TOKEN_RPAREN);
        case '{': return makeToken(lexer, TOKEN_LBRACE);
        case '}': return makeToken(lexer, TOKEN_RBRACE);
        case '[': return makeToken(lexer, TOKEN_LBRACKET);
        case ']': return makeToken(lexer, TOKEN_RBRACKET);
        case ',': return makeToken(lexer, TOKEN_COMMA);
        case '.': return makeToken(lexer, TOKEN_DOT);
        case ';': return makeToken(lexer, TOKEN_SEMICOLON);
        case ':': return makeToken(lexer, TOKEN_COLON);
        case '+': return makeToken(lexer, TOKEN_PLUS);
        case '-': return makeToken(lexer, TOKEN_MINUS);
        case '*': return makeToken(lexer, TOKEN_STAR);
        case '/': return makeToken(lexer, TOKEN_SLASH);
        case '!': return makeToken(lexer, match(lexer, '=') ? TOKEN_NEQ : TOKEN_BANG);
        case '=': return makeToken(lexer, match(lexer, '=') ? TOKEN_EQ : TOKEN_ASSIGN);
        case '<': return makeToken(lexer, match(lexer, '=') ? TOKEN_LE : TOKEN_LT);
        case '>': return makeToken(lexer, match(lexer, '=') ? TOKEN_GE : TOKEN_GT);
        case '?': return makeToken(lexer, TOKEN_QUESTION);
        case '&': if (match(lexer, '&')) return makeToken(lexer, TOKEN_AND); break;
        case '|': if (match(lexer, '|')) return makeToken(lexer, TOKEN_OR); break;
        case '"': return string(lexer);
    }

    // Error: Unknown character.
    return makeToken(lexer, TOKEN_EOF);
}

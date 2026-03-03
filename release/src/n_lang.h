#ifndef N_LANG_H
#define N_LANG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

#define N_VERSION "1.0.0"
#define N_AUTHOR "Squ4sh000"

// Token Types
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_VAR,
    TOKEN_CONST,
    TOKEN_FN,
    TOKEN_CLASS,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_IMPORT,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_LBRACKET,  // [
    TOKEN_RBRACKET,  // ]
    TOKEN_COMMA,     // ,
    TOKEN_DOT,       // .
    TOKEN_SEMICOLON, // ;
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_STAR,      // *
    TOKEN_SLASH,     // /
    TOKEN_ASSIGN,    // =
    TOKEN_EQ,        // ==
    TOKEN_NEQ,       // !=
    TOKEN_GT,        // >
    TOKEN_LT,        // <
    TOKEN_GE,        // >=
    TOKEN_LE,        // <=
    TOKEN_AND,       // &&
    TOKEN_OR,        // ||
    TOKEN_BANG,      // !
    TOKEN_COLON,     // :
    TOKEN_QUESTION,  // ?
    TOKEN_IN         // in
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int line;
} Token;

// AST Node Types
typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_CONST_DECL,
    NODE_FN_DECL,
    NODE_CLASS_DECL,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_RETURN,
    NODE_IMPORT,
    NODE_EXPR_STMT,
    NODE_BLOCK,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_LITERAL,
    NODE_VARIABLE,
    NODE_CALL,
    NODE_GET,
    NODE_SET,
    NODE_SELF,
    NODE_TRY,
    NODE_ARRAY_LITERAL,
    NODE_INDEX_GET,
    NODE_INDEX_SET,
    NODE_FOR_IN
} NodeType;

typedef struct ASTNode {
    NodeType type;
    Token token;
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode** children;
    int child_count;
    // For literals
    void* value;
} ASTNode;

// Lexer State
typedef struct {
    const char* source;
    int start;
    int current;
    int line;
} Lexer;

// Value System
typedef enum {
    VAL_NUMBER,
    VAL_BOOL,
    VAL_STRING,
    VAL_NULL,
    VAL_OBJ,
    VAL_FN,
    VAL_CLASS,
    VAL_INSTANCE,
    VAL_ARRAY
} ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        bool boolean;
        char* string;
        void* obj;
    } as;
} Value;

// Environment
typedef struct Environment {
    struct Environment* enclosing;
    struct Entry {
        char* name;
        Value value;
        bool is_const;
        struct Entry* next;
    }* entries;
} Environment;

// Function Prototypes
Lexer* createLexer(const char* source);
Token scanToken(Lexer* lexer);
ASTNode* parse(Token* tokens, int token_count);
void interpret(ASTNode* root);
void freeAST(ASTNode* node);
char* readFile(const char* path);

// Global State for Return (Simple Hack)
extern Value returnValue;
extern bool isReturning;

#endif

#include "n_lang.h"

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        printf("N Language Interpreter v%s by %s\n", N_VERSION, N_AUTHOR);
        printf("Usage: n [path]\n");
        exit(0);
    }
    
    if (argc == 2 && strcmp(argv[1], "-v") == 0) {
        printf("N Language v%s\n", N_VERSION);
        exit(0);
    }

    if (argc != 2) {
        printf("Usage: n [path]\n");
        exit(64);
    }

    const char* source = readFile(argv[1]);
    Lexer* lexer = createLexer(source);
    
    // Tokenize
    int capacity = 1024;
    Token* tokens = (Token*)malloc(sizeof(Token) * capacity);
    int count = 0;
    
    for (;;) {
        Token token = scanToken(lexer);
        if (count >= capacity) {
            capacity *= 2;
            tokens = (Token*)realloc(tokens, sizeof(Token) * capacity);
        }
        tokens[count++] = token;
        if (token.type == TOKEN_EOF) break;
    }

    // Parse
    ASTNode* root = parse(tokens, count);
    
    // Interpret
    interpret(root);

    // Clean up (Simplified)
    // freeAST(root);
    // free(tokens);
    // free(lexer);
    // free((void*)source);

    return 0;
}

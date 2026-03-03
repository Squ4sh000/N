#include "n_lang.h"

typedef struct {
    Token* tokens;
    int current;
    int count;
} Parser;

ASTNode* createNode(NodeType type, Token token) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->token = token;
    node->left = NULL;
    node->right = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->value = NULL;
    return node;
}

void addChild(ASTNode* parent, ASTNode* child) {
    parent->child_count++;
    parent->children = (ASTNode**)realloc(parent->children, sizeof(ASTNode*) * parent->child_count);
    parent->children[parent->child_count - 1] = child;
}

bool parserIsAtEnd(Parser* parser) {
    return parser->tokens[parser->current].type == TOKEN_EOF;
}

Token parserPeek(Parser* parser) {
    return parser->tokens[parser->current];
}

Token parserPrevious(Parser* parser) {
    return parser->tokens[parser->current - 1];
}

Token parserAdvance(Parser* parser) {
    if (!parserIsAtEnd(parser)) parser->current++;
    return parserPrevious(parser);
}

bool parserCheck(Parser* parser, TokenType type) {
    if (parserIsAtEnd(parser)) return false;
    return parserPeek(parser).type == type;
}

bool parserMatch(Parser* parser, TokenType type) {
    if (parserCheck(parser, type)) {
        parserAdvance(parser);
        return true;
    }
    return false;
}

Token parserConsume(Parser* parser, TokenType type, const char* message) {
    if (parserCheck(parser, type)) return parserAdvance(parser);
    // Error handling: output message and potentially exit or synchronize
    fprintf(stderr, "Parser Error at line %d: %s\n", parserPeek(parser).line, message);
    exit(1);
}

// Forward declarations for recursive descent
ASTNode* expression(Parser* parser);
ASTNode* statement(Parser* parser);
ASTNode* declaration(Parser* parser);
ASTNode* varDeclaration(Parser* parser);

ASTNode* primary(Parser* parser) {
    if (parserMatch(parser, TOKEN_FALSE)) return createNode(NODE_LITERAL, parserPrevious(parser));
    if (parserMatch(parser, TOKEN_TRUE)) return createNode(NODE_LITERAL, parserPrevious(parser));
    if (parserMatch(parser, TOKEN_NULL)) return createNode(NODE_LITERAL, parserPrevious(parser));
    if (parserMatch(parser, TOKEN_NUMBER)) return createNode(NODE_LITERAL, parserPrevious(parser));
    if (parserMatch(parser, TOKEN_STRING)) return createNode(NODE_LITERAL, parserPrevious(parser));
    
    if (parserMatch(parser, TOKEN_IDENTIFIER)) {
        if (strcmp(parserPrevious(parser).lexeme, "self") == 0) {
            return createNode(NODE_SELF, parserPrevious(parser));
        }
        return createNode(NODE_VARIABLE, parserPrevious(parser));
    }

    if (parserMatch(parser, TOKEN_LPAREN)) {
        ASTNode* expr = expression(parser);
        parserConsume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
        return expr;
    }

    if (parserMatch(parser, TOKEN_LBRACKET)) {
        Token bracket = parserPrevious(parser);
        ASTNode* node = createNode(NODE_ARRAY_LITERAL, bracket);
        if (!parserCheck(parser, TOKEN_RBRACKET)) {
            do {
                addChild(node, expression(parser));
            } while (parserMatch(parser, TOKEN_COMMA));
        }
        parserConsume(parser, TOKEN_RBRACKET, "Expect ']' after array elements.");
        return node;
    }

    // Error: Expect expression.
    return NULL;
}

ASTNode* finishCall(Parser* parser, ASTNode* callee) {
    ASTNode* node = createNode(NODE_CALL, callee->token);
    node->left = callee;
    if (!parserCheck(parser, TOKEN_RPAREN)) {
        do {
            addChild(node, expression(parser));
        } while (parserMatch(parser, TOKEN_COMMA));
    }
    parserConsume(parser, TOKEN_RPAREN, "Expect ')' after arguments.");
    return node;
}

ASTNode* call(Parser* parser) {
    ASTNode* expr = primary(parser);

    for (;;) {
        if (parserMatch(parser, TOKEN_LPAREN)) {
            expr = finishCall(parser, expr);
        } else if (parserMatch(parser, TOKEN_DOT)) {
            Token name = parserConsume(parser, TOKEN_IDENTIFIER, "Expect property name after '.'.");
            ASTNode* get_node = createNode(NODE_GET, name);
            get_node->left = expr;
            expr = get_node;
        } else if (parserMatch(parser, TOKEN_LBRACKET)) {
            Token bracket = parserPrevious(parser);
            ASTNode* indexNode = createNode(NODE_INDEX_GET, bracket);
            indexNode->left = expr;
            indexNode->right = expression(parser);
            parserConsume(parser, TOKEN_RBRACKET, "Expect ']' after index.");
            expr = indexNode;
        } else {
            break;
        }
    }

    if (parserMatch(parser, TOKEN_QUESTION)) {
        ASTNode* tryNode = createNode(NODE_TRY, parserPrevious(parser));
        tryNode->left = expr;
        expr = tryNode;
    }

    return expr;
}

ASTNode* unary(Parser* parser) {
    if (parserMatch(parser, TOKEN_BANG) || parserMatch(parser, TOKEN_MINUS)) {
        Token op = parserPrevious(parser);
        ASTNode* right = unary(parser);
        ASTNode* node = createNode(NODE_UNARY_EXPR, op);
        node->right = right;
        return node;
    }
    return call(parser);
}

ASTNode* multiplication(Parser* parser) {
    ASTNode* expr = unary(parser);
    while (parserMatch(parser, TOKEN_STAR) || parserMatch(parser, TOKEN_SLASH)) {
        Token op = parserPrevious(parser);
        ASTNode* right = unary(parser);
        ASTNode* node = createNode(NODE_BINARY_EXPR, op);
        node->left = expr;
        node->right = right;
        expr = node;
    }
    return expr;
}

ASTNode* addition(Parser* parser) {
    ASTNode* expr = multiplication(parser);
    while (parserMatch(parser, TOKEN_PLUS) || parserMatch(parser, TOKEN_MINUS)) {
        Token op = parserPrevious(parser);
        ASTNode* right = multiplication(parser);
        ASTNode* node = createNode(NODE_BINARY_EXPR, op);
        node->left = expr;
        node->right = right;
        expr = node;
    }
    return expr;
}

ASTNode* comparison(Parser* parser) {
    ASTNode* expr = addition(parser);
    while (parserMatch(parser, TOKEN_GT) || parserMatch(parser, TOKEN_GE) ||
           parserMatch(parser, TOKEN_LT) || parserMatch(parser, TOKEN_LE)) {
        Token op = parserPrevious(parser);
        ASTNode* right = addition(parser);
        ASTNode* node = createNode(NODE_BINARY_EXPR, op);
        node->left = expr;
        node->right = right;
        expr = node;
    }
    return expr;
}

ASTNode* equality(Parser* parser) {
    ASTNode* expr = comparison(parser);
    while (parserMatch(parser, TOKEN_EQ) || parserMatch(parser, TOKEN_NEQ)) {
        Token op = parserPrevious(parser);
        ASTNode* right = comparison(parser);
        ASTNode* node = createNode(NODE_BINARY_EXPR, op);
        node->left = expr;
        node->right = right;
        expr = node;
    }
    return expr;
}

ASTNode* assignment(Parser* parser) {
    ASTNode* expr = equality(parser);

    if (parserMatch(parser, TOKEN_ASSIGN)) {
        Token equals = parserPrevious(parser);
        ASTNode* value = assignment(parser);

        if (expr->type == NODE_VARIABLE) {
            ASTNode* node = createNode(NODE_ASSIGN, expr->token);
            node->right = value;
            return node;
        } else if (expr->type == NODE_GET) {
            ASTNode* node = createNode(NODE_SET, expr->token);
            node->left = expr->left;
            node->right = value;
            return node;
        } else if (expr->type == NODE_INDEX_GET) {
            ASTNode* node = createNode(NODE_INDEX_SET, expr->token);
            node->left = expr->left;   // The collection
            node->right = expr->right; // The index
            // The value is the right side of the assignment
            ASTNode* assignNode = createNode(NODE_ASSIGN, equals);
            assignNode->right = value;
            addChild(node, value);
            return node;
        }
        // Error: Invalid assignment target.
    }

    return expr;
}

ASTNode* expression(Parser* parser) {
    return assignment(parser);
}

ASTNode* block(Parser* parser) {
    ASTNode* node = createNode(NODE_BLOCK, parserPrevious(parser));
    while (!parserCheck(parser, TOKEN_RBRACE) && !parserIsAtEnd(parser)) {
        addChild(node, declaration(parser));
    }
    parserConsume(parser, TOKEN_RBRACE, "Expect '}' after block.");
    return node;
}

ASTNode* ifStatement(Parser* parser) {
    parserConsume(parser, TOKEN_LPAREN, "Expect '(' after 'if'.");
    ASTNode* condition = expression(parser);
    parserConsume(parser, TOKEN_RPAREN, "Expect ')' after condition.");

    ASTNode* thenBranch = statement(parser);
    ASTNode* elseBranch = NULL;
    if (parserMatch(parser, TOKEN_ELSE)) {
        elseBranch = statement(parser);
    }

    ASTNode* node = createNode(NODE_IF, parserPrevious(parser));
    node->left = condition;
    node->right = thenBranch;
    if (elseBranch) addChild(node, elseBranch);
    return node;
}

ASTNode* whileStatement(Parser* parser) {
    parserConsume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");
    ASTNode* condition = expression(parser);
    parserConsume(parser, TOKEN_RPAREN, "Expect ')' after condition.");
    ASTNode* body = statement(parser);

    ASTNode* node = createNode(NODE_WHILE, parserPrevious(parser));
    node->left = condition;
    node->right = body;
    return node;
}

ASTNode* returnStatement(Parser* parser) {
    Token keyword = parserPrevious(parser);
    ASTNode* value = NULL;
    if (!parserCheck(parser, TOKEN_SEMICOLON)) {
        value = expression(parser);
    }
    parserConsume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
    ASTNode* node = createNode(NODE_RETURN, keyword);
    node->left = value;
    return node;
}

ASTNode* forStatement(Parser* parser) {
    Token forToken = parserPrevious(parser);
    parserConsume(parser, TOKEN_LPAREN, "Expect '(' after 'for'.");

    // Check for for-in loop: for (var item in collection) or for (item in collection)
    bool isForIn = false;
    Token itemToken;
    bool hasVar = false;

    if (parserMatch(parser, TOKEN_VAR)) {
        hasVar = true;
        if (parserCheck(parser, TOKEN_IDENTIFIER)) {
            itemToken = parserPeek(parser);
            if (parser->tokens[parser->current + 1].type == TOKEN_IN) {
                isForIn = true;
                parserAdvance(parser); // consume identifier
                parserAdvance(parser); // consume 'in'
            }
        }
    } else if (parserCheck(parser, TOKEN_IDENTIFIER)) {
        itemToken = parserPeek(parser);
        if (parser->tokens[parser->current + 1].type == TOKEN_IN) {
            isForIn = true;
            parserAdvance(parser); // consume identifier
            parserAdvance(parser); // consume 'in'
        }
    }

    if (isForIn) {
        ASTNode* collection = expression(parser);
        parserConsume(parser, TOKEN_RPAREN, "Expect ')' after for-in clauses.");
        ASTNode* body = statement(parser);

        ASTNode* node = createNode(NODE_FOR_IN, forToken);
        node->token = itemToken; // Use the item name token
        node->left = collection;
        node->right = body;
        // Mark if it has 'var' by adding a dummy child or something? 
        // Let's just assume it always defines a local variable in the loop scope.
        return node;
    }

    // Traditional for loop logic
    ASTNode* initializer = NULL;
    if (parserMatch(parser, TOKEN_SEMICOLON)) {
        initializer = NULL;
    } else if (hasVar) {
        // We already matched 'var' above. current is at the identifier.
        initializer = varDeclaration(parser);
    } else {
        initializer = expression(parser);
        parserConsume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    }

    ASTNode* condition = NULL;
    if (!parserCheck(parser, TOKEN_SEMICOLON)) {
        condition = expression(parser);
    }
    parserConsume(parser, TOKEN_SEMICOLON, "Expect ';' after loop condition.");

    ASTNode* increment = NULL;
    if (!parserCheck(parser, TOKEN_RPAREN)) {
        increment = expression(parser);
    }
    parserConsume(parser, TOKEN_RPAREN, "Expect ')' after for clauses.");

    ASTNode* body = statement(parser);

    // Desugar to while
    if (increment != NULL) {
        ASTNode* blockNode = createNode(NODE_BLOCK, body->token);
        addChild(blockNode, body);
        ASTNode* exprStmt = createNode(NODE_EXPR_STMT, increment->token);
        exprStmt->left = increment;
        addChild(blockNode, exprStmt);
        body = blockNode;
    }

    if (condition == NULL) {
        Token trueToken = {TOKEN_TRUE, "true", body->token.line};
        condition = createNode(NODE_LITERAL, trueToken);
    }
    ASTNode* whileNode = createNode(NODE_WHILE, body->token);
    whileNode->left = condition;
    whileNode->right = body;

    if (initializer != NULL) {
        ASTNode* blockNode = createNode(NODE_BLOCK, initializer->token);
        addChild(blockNode, initializer);
        addChild(blockNode, whileNode);
        return blockNode;
    }

    return whileNode;
}

ASTNode* statement(Parser* parser) {
    if (parserMatch(parser, TOKEN_FOR)) return forStatement(parser);
    if (parserMatch(parser, TOKEN_IF)) return ifStatement(parser);
    if (parserMatch(parser, TOKEN_WHILE)) return whileStatement(parser);
    if (parserMatch(parser, TOKEN_RETURN)) return returnStatement(parser);
    if (parserMatch(parser, TOKEN_LBRACE)) return block(parser);

    ASTNode* expr = expression(parser);
    parserConsume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    ASTNode* node = createNode(NODE_EXPR_STMT, expr->token);
    node->left = expr;
    return node;
}

ASTNode* varDeclaration(Parser* parser) {
    Token name = parserConsume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
    ASTNode* initializer = NULL;
    if (parserMatch(parser, TOKEN_ASSIGN)) {
        initializer = expression(parser);
    }
    parserConsume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    ASTNode* node = createNode(NODE_VAR_DECL, name);
    node->left = initializer;
    return node;
}

ASTNode* function(Parser* parser, const char* kind) {
    Token name = parserConsume(parser, TOKEN_IDENTIFIER, "Expect function name.");
    parserConsume(parser, TOKEN_LPAREN, "Expect '(' after function name.");
    ASTNode* node = createNode(NODE_FN_DECL, name);
    if (!parserCheck(parser, TOKEN_RPAREN)) {
        do {
            Token param = parserConsume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
            addChild(node, createNode(NODE_VARIABLE, param));
        } while (parserMatch(parser, TOKEN_COMMA));
    }
    parserConsume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");
    parserConsume(parser, TOKEN_LBRACE, "Expect '{' before function body.");
    node->left = block(parser);
    return node;
}

ASTNode* classDeclaration(Parser* parser) {
    Token name = parserConsume(parser, TOKEN_IDENTIFIER, "Expect class name.");
    parserConsume(parser, TOKEN_LBRACE, "Expect '{' before class body.");

    ASTNode* node = createNode(NODE_CLASS_DECL, name);
    while (!parserCheck(parser, TOKEN_RBRACE) && !parserIsAtEnd(parser)) {
        if (parserMatch(parser, TOKEN_FN)) {
            addChild(node, function(parser, "method"));
        } else if (parserMatch(parser, TOKEN_VAR)) {
            addChild(node, varDeclaration(parser));
        } else {
            // Error handling
            parserAdvance(parser);
        }
    }
    parserConsume(parser, TOKEN_RBRACE, "Expect '}' after class body.");
    return node;
}

ASTNode* importDeclaration(Parser* parser) {
    Token path = parserConsume(parser, TOKEN_STRING, "Expect import path string.");
    parserConsume(parser, TOKEN_SEMICOLON, "Expect ';' after import.");
    return createNode(NODE_IMPORT, path);
}

ASTNode* declaration(Parser* parser) {
    if (parserMatch(parser, TOKEN_CLASS)) return classDeclaration(parser);
    if (parserMatch(parser, TOKEN_FN)) return function(parser, "function");
    if (parserMatch(parser, TOKEN_VAR)) return varDeclaration(parser);
    if (parserMatch(parser, TOKEN_CONST)) {
        ASTNode* node = varDeclaration(parser);
        node->type = NODE_CONST_DECL;
        return node;
    }
    if (parserMatch(parser, TOKEN_IMPORT)) return importDeclaration(parser);

    return statement(parser);
}

ASTNode* parse(Token* tokens, int token_count) {
    Parser parser = {tokens, 0, token_count};
    ASTNode* program = createNode(NODE_PROGRAM, tokens[0]);
    while (!parserIsAtEnd(&parser)) {
        addChild(program, declaration(&parser));
    }
    return program;
}

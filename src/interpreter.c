#include "n_lang.h"

Value returnValue = {VAL_NULL, {0}};
bool isReturning = false;

Environment* createEnvironment(Environment* enclosing) {
    Environment* env = (Environment*)malloc(sizeof(Environment));
    env->enclosing = enclosing;
    env->entries = NULL;
    return env;
}

void define(Environment* env, const char* name, Value value, bool is_const) {
    struct Entry* entry = (struct Entry*)malloc(sizeof(struct Entry));
    entry->name = strdup(name);
    entry->value = value;
    entry->is_const = is_const;
    entry->next = env->entries;
    env->entries = entry;
}

typedef struct {
    ASTNode* declaration;
    Environment* closure;
} Function;

typedef struct {
    Value* fields;
    char** names;
    int count;
    ASTNode* classDecl;
} Instance;

typedef struct {
    Value* elements;
    int size;
    int capacity;
} NArray;

void runtimeError(int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Runtime Error at line %d: ", line);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

void printValue(Value val) {
    if (val.type == VAL_NUMBER) printf("%g", val.as.number);
    else if (val.type == VAL_STRING) printf("%s", val.as.string);
    else if (val.type == VAL_BOOL) printf(val.as.boolean ? "true" : "false");
    else if (val.type == VAL_ARRAY) {
        NArray* array = (NArray*)val.as.obj;
        printf("[");
        for (int i = 0; i < array->size; i++) {
            printValue(array->elements[i]);
            if (i < array->size - 1) printf(", ");
        }
        printf("]");
    }
    else if (val.type == VAL_NULL) printf("null");
    else if (val.type == VAL_FN) printf("<fn>");
    else if (val.type == VAL_CLASS) printf("<class>");
    else if (val.type == VAL_INSTANCE) printf("<instance>");
}

Value evaluate(ASTNode* node, Environment* env);
void execute(ASTNode* node, Environment* env);

Value callFunction(Function* fn, Value* args, int arg_count) {
    Environment* env = createEnvironment(fn->closure);
    for (int i = 0; i < fn->declaration->child_count; i++) {
        define(env, fn->declaration->children[i]->token.lexeme, args[i], false);
    }
    execute(fn->declaration->left, env);
    
    Value res = returnValue;
    returnValue = (Value){VAL_NULL, {0}};
    isReturning = false;
    return res;
}

void execute(ASTNode* node, Environment* env) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK: {
            Environment* localEnv = (node->type == NODE_BLOCK) ? createEnvironment(env) : env;
            for (int i = 0; i < node->child_count; i++) {
                execute(node->children[i], localEnv);
                if (isReturning) break;
            }
            break;
        }
        case NODE_CONST_DECL:
        case NODE_VAR_DECL: {
            Value val = {VAL_NULL, {0}};
            if (node->left) val = evaluate(node->left, env);
            define(env, node->token.lexeme, val, node->type == NODE_CONST_DECL);
            break;
        }
        case NODE_FN_DECL: {
            Function* fn = (Function*)malloc(sizeof(Function));
            fn->declaration = node;
            fn->closure = env;
            Value val = {VAL_FN, {.obj = fn}};
            define(env, node->token.lexeme, val, false);
            break;
        }
        case NODE_CLASS_DECL: {
            Value val = {VAL_CLASS, {.obj = node}};
            define(env, node->token.lexeme, val, false);
            break;
        }
        case NODE_IF: {
            Value cond = evaluate(node->left, env);
            bool isTruthy = (cond.type == VAL_BOOL && cond.as.boolean) || 
                            (cond.type == VAL_NUMBER && cond.as.number != 0) ||
                            (cond.type == VAL_STRING && strlen(cond.as.string) > 0) ||
                            (cond.type == VAL_FN || cond.type == VAL_CLASS || cond.type == VAL_INSTANCE);
            if (isTruthy) {
                execute(node->right, env);
            } else if (node->child_count > 0) {
                execute(node->children[0], env);
            }
            break;
        }
        case NODE_WHILE: {
            while (true) {
                Value cond = evaluate(node->left, env);
                bool isTruthy = (cond.type == VAL_BOOL && cond.as.boolean) || 
                                (cond.type == VAL_NUMBER && cond.as.number != 0) ||
                                (cond.type == VAL_STRING && strlen(cond.as.string) > 0) ||
                                (cond.type == VAL_FN || cond.type == VAL_CLASS || cond.type == VAL_INSTANCE);
                if (isTruthy) {
                    execute(node->right, env);
                    if (isReturning) break;
                } else {
                    break;
                }
            }
            break;
        }
        case NODE_FOR_IN: {
            Value collection = evaluate(node->left, env);
            if (collection.type == VAL_ARRAY) {
                NArray* array = (NArray*)collection.as.obj;
                for (int i = 0; i < array->size; i++) {
                    Environment* loopEnv = createEnvironment(env);
                    define(loopEnv, node->token.lexeme, array->elements[i], false);
                    execute(node->right, loopEnv);
                    // Free loopEnv? (Simplified for now)
                    if (isReturning) break;
                }
            } else if (collection.type == VAL_STRING) {
                char* str = collection.as.string;
                int len = strlen(str);
                for (int i = 0; i < len; i++) {
                    Environment* loopEnv = createEnvironment(env);
                    char* charStr = (char*)malloc(2);
                    charStr[0] = str[i];
                    charStr[1] = '\0';
                    Value val = {VAL_STRING, {.string = charStr}};
                    define(loopEnv, node->token.lexeme, val, false);
                    execute(node->right, loopEnv);
                    if (isReturning) break;
                }
            } else {
                runtimeError(node->token.line, "Can only iterate over arrays and strings.");
            }
            break;
        }
        case NODE_EXPR_STMT:
            evaluate(node->left, env);
            break;
        case NODE_RETURN:
            if (node->left) {
                returnValue = evaluate(node->left, env);
            } else {
                returnValue = (Value){VAL_NULL, {0}};
            }
            isReturning = true;
            break;
        case NODE_IMPORT: {
            char* path = strdup(node->token.lexeme + 1);
            path[strlen(path) - 1] = '\0'; // Remove trailing "
            char* source = readFile(path);
            Lexer* lexer = createLexer(source);
            
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

            ASTNode* importedRoot = parse(tokens, count);
            execute(importedRoot, env); // Execute in the current environment
            
            // Clean up
            free(path);
            free(source);
            // free(tokens); // Tokens are used by AST, need careful cleanup
            break;
        }
        default:
            break;
    }
}

Value evaluate(ASTNode* node, Environment* env) {
    if (node == NULL) return (Value){VAL_NULL, {0}};
    switch (node->type) {
        case NODE_LITERAL: {
            Value val;
            if (node->token.type == TOKEN_NUMBER) {
                val.type = VAL_NUMBER;
                val.as.number = atof(node->token.lexeme);
            } else if (node->token.type == TOKEN_STRING) {
                val.type = VAL_STRING;
                val.as.string = strdup(node->token.lexeme + 1);
                val.as.string[strlen(val.as.string) - 1] = '\0'; // Remove trailing "
            } else if (node->token.type == TOKEN_TRUE) {
                val.type = VAL_BOOL;
                val.as.boolean = true;
            } else if (node->token.type == TOKEN_FALSE) {
                val.type = VAL_BOOL;
                val.as.boolean = false;
            } else {
                val.type = VAL_NULL;
            }
            return val;
        }
        case NODE_UNARY_EXPR: {
            Value right = evaluate(node->right, env);
            if (node->token.type == TOKEN_MINUS) {
                if (right.type == VAL_NUMBER) return (Value){VAL_NUMBER, {.number = -right.as.number}};
            } else if (node->token.type == TOKEN_BANG) {
                bool isTruthy = (right.type == VAL_BOOL && right.as.boolean) || 
                                (right.type == VAL_NUMBER && right.as.number != 0) ||
                                (right.type == VAL_STRING && strlen(right.as.string) > 0);
                return (Value){VAL_BOOL, {.boolean = !isTruthy}};
            }
            return (Value){VAL_NULL, {0}};
        }
        case NODE_TRY: {
            Value val = evaluate(node->left, env);
            if (val.type == VAL_NULL) {
                returnValue = val;
                isReturning = true;
            }
            return val;
        }
        case NODE_ARRAY_LITERAL: {
            NArray* array = (NArray*)malloc(sizeof(NArray));
            array->size = node->child_count;
            array->capacity = array->size;
            array->elements = (Value*)malloc(sizeof(Value) * array->size);
            for (int i = 0; i < node->child_count; i++) {
                array->elements[i] = evaluate(node->children[i], env);
            }
            return (Value){VAL_ARRAY, {.obj = array}};
        }
        case NODE_INDEX_GET: {
            Value collection = evaluate(node->left, env);
            Value index = evaluate(node->right, env);
            
            if (collection.type == VAL_ARRAY) {
                NArray* array = (NArray*)collection.as.obj;
                if (index.type != VAL_NUMBER) runtimeError(node->token.line, "Array index must be a number.");
                int idx = (int)index.as.number;
                if (idx < 0 || idx >= array->size) runtimeError(node->token.line, "Array index out of bounds.");
                return array->elements[idx];
            } else if (collection.type == VAL_STRING) {
                if (index.type != VAL_NUMBER) runtimeError(node->token.line, "String index must be a number.");
                int idx = (int)index.as.number;
                int len = strlen(collection.as.string);
                if (idx < 0 || idx >= len) runtimeError(node->token.line, "String index out of bounds.");
                char* charStr = (char*)malloc(2);
                charStr[0] = collection.as.string[idx];
                charStr[1] = '\0';
                return (Value){VAL_STRING, {.string = charStr}};
            }
            runtimeError(node->token.line, "Only arrays and strings support indexing.");
            return (Value){VAL_NULL, {0}};
        }
        case NODE_INDEX_SET: {
            Value collection = evaluate(node->left, env);
            Value index = evaluate(node->right, env);
            Value val = evaluate(node->children[0], env);
            
            if (collection.type == VAL_ARRAY) {
                NArray* array = (NArray*)collection.as.obj;
                if (index.type != VAL_NUMBER) runtimeError(node->token.line, "Array index must be a number.");
                int idx = (int)index.as.number;
                if (idx < 0 || idx >= array->size) runtimeError(node->token.line, "Array index out of bounds.");
                array->elements[idx] = val;
                return val;
            }
            runtimeError(node->token.line, "Only arrays support index assignment.");
            return (Value){VAL_NULL, {0}};
        }
        case NODE_BINARY_EXPR: {
            Value left = evaluate(node->left, env);
            Value right = evaluate(node->right, env);
            Value res = {VAL_NULL, {0}};
            if (node->token.type == TOKEN_PLUS) {
                if (left.type == VAL_NUMBER && right.type == VAL_NUMBER) {
                    res.type = VAL_NUMBER;
                    res.as.number = left.as.number + right.as.number;
                } else if (left.type == VAL_STRING || right.type == VAL_STRING) {
                    char s1[128], s2[128];
                    if (left.type == VAL_STRING) strcpy(s1, left.as.string);
                    else if (left.type == VAL_NUMBER) sprintf(s1, "%g", left.as.number);
                    else strcpy(s1, "null");

                    if (right.type == VAL_STRING) strcpy(s2, right.as.string);
                    else if (right.type == VAL_NUMBER) sprintf(s2, "%g", right.as.number);
                    else strcpy(s2, "null");
                    
                    res.type = VAL_STRING;
                    res.as.string = (char*)malloc(strlen(s1) + strlen(s2) + 1);
                    strcpy(res.as.string, s1);
                    strcat(res.as.string, s2);
                }
            } else if (node->token.type == TOKEN_MINUS) {
                res.type = VAL_NUMBER;
                res.as.number = left.as.number - right.as.number;
            } else if (node->token.type == TOKEN_STAR) {
                res.type = VAL_NUMBER;
                res.as.number = left.as.number * right.as.number;
            } else if (node->token.type == TOKEN_SLASH) {
                if (right.as.number == 0) runtimeError(node->token.line, "Division by zero.");
                res.type = VAL_NUMBER;
                res.as.number = left.as.number / right.as.number;
            } else if (node->token.type == TOKEN_GT) {
                res.type = VAL_BOOL;
                res.as.boolean = left.as.number > right.as.number;
            } else if (node->token.type == TOKEN_LT) {
                res.type = VAL_BOOL;
                res.as.boolean = left.as.number < right.as.number;
            } else if (node->token.type == TOKEN_LE) {
                res.type = VAL_BOOL;
                res.as.boolean = left.as.number <= right.as.number;
            } else if (node->token.type == TOKEN_GE) {
                res.type = VAL_BOOL;
                res.as.boolean = left.as.number >= right.as.number;
            } else if (node->token.type == TOKEN_EQ) {
                res.type = VAL_BOOL;
                if (left.type == VAL_NUMBER && right.type == VAL_NUMBER) res.as.boolean = left.as.number == right.as.number;
                else if (left.type == VAL_BOOL && right.type == VAL_BOOL) res.as.boolean = left.as.boolean == right.as.boolean;
                else if (left.type == VAL_STRING && right.type == VAL_STRING) res.as.boolean = strcmp(left.as.string, right.as.string) == 0;
                else res.as.boolean = (left.type == right.type);
            } else if (node->token.type == TOKEN_NEQ) {
                res.type = VAL_BOOL;
                if (left.type == VAL_NUMBER && right.type == VAL_NUMBER) res.as.boolean = left.as.number != right.as.number;
                else if (left.type == VAL_BOOL && right.type == VAL_BOOL) res.as.boolean = left.as.boolean != right.as.boolean;
                else if (left.type == VAL_STRING && right.type == VAL_STRING) res.as.boolean = strcmp(left.as.string, right.as.string) != 0;
                else res.as.boolean = (left.type != right.type);
            }
            return res;
        }
        case NODE_ASSIGN: {
            Value val = evaluate(node->right, env);
            Environment* curr = env;
            while (curr) {
                struct Entry* entry = curr->entries;
                while (entry) {
                    if (strcmp(entry->name, node->token.lexeme) == 0) {
                        if (entry->is_const) {
                            runtimeError(node->token.line, "Cannot assign to constant '%s'.", entry->name);
                        }
                        entry->value = val;
                        return val;
                    }
                    entry = entry->next;
                }
                curr = curr->enclosing;
            }
            return val;
        }
        case NODE_SELF:
        case NODE_VARIABLE: {
            Environment* curr = env;
            while (curr) {
                struct Entry* entry = curr->entries;
                while (entry) {
                    if (strcmp(entry->name, node->token.lexeme) == 0) return entry->value;
                    entry = entry->next;
                }
                curr = curr->enclosing;
            }
            return (Value){VAL_NULL, {0}};
        }
        case NODE_SET: {
            Value objVal = evaluate(node->left, env);
            if (objVal.type != VAL_INSTANCE) {
                runtimeError(node->token.line, "Only instances have properties.");
            }
            Instance* instance = (Instance*)objVal.as.obj;
            Value val = evaluate(node->right, env);
            for (int i = 0; i < instance->count; i++) {
                if (strcmp(instance->names[i], node->token.lexeme) == 0) {
                    instance->fields[i] = val;
                    return val;
                }
            }
            instance->count++;
            instance->fields = (Value*)realloc(instance->fields, sizeof(Value) * instance->count);
            instance->names = (char**)realloc(instance->names, sizeof(char*) * instance->count);
            instance->fields[instance->count - 1] = val;
            instance->names[instance->count - 1] = strdup(node->token.lexeme);
            return val;
        }
        case NODE_GET: {
            Value objVal = evaluate(node->left, env);
            if (objVal.type != VAL_INSTANCE) {
                runtimeError(node->token.line, "Only instances have properties.");
            }
            Instance* instance = (Instance*)objVal.as.obj;
            for (int i = 0; i < instance->count; i++) {
                if (strcmp(instance->names[i], node->token.lexeme) == 0) return instance->fields[i];
            }
            // Look for method in class
            ASTNode* classDecl = instance->classDecl;
            for (int i = 0; i < classDecl->child_count; i++) {
                ASTNode* member = classDecl->children[i];
                if (member->type == NODE_FN_DECL && strcmp(member->token.lexeme, node->token.lexeme) == 0) {
                    Function* fn = (Function*)malloc(sizeof(Function));
                    fn->declaration = member;
                    // Bind self
                    fn->closure = createEnvironment(env);
                    define(fn->closure, "self", objVal, false);
                    return (Value){VAL_FN, {.obj = fn}};
                }
            }
            return (Value){VAL_NULL, {0}};
        }
        case NODE_CALL: {
            if (strcmp(node->left->token.lexeme, "print") == 0) {
                for (int i = 0; i < node->child_count; i++) {
                    Value val = evaluate(node->children[i], env);
                    printValue(val);
                }
                printf("\n");
                return (Value){VAL_NULL, {0}};
            }

            if (strcmp(node->left->token.lexeme, "len") == 0) {
                if (node->child_count > 0) {
                    Value val = evaluate(node->children[0], env);
                    if (val.type == VAL_STRING) return (Value){VAL_NUMBER, {.number = (double)strlen(val.as.string)}};
                    if (val.type == VAL_ARRAY) return (Value){VAL_NUMBER, {.number = (double)((NArray*)val.as.obj)->size}};
                }
                return (Value){VAL_NUMBER, {.number = 0}};
            }

            if (strcmp(node->left->token.lexeme, "input") == 0) {
                if (node->child_count > 0) {
                    Value prompt = evaluate(node->children[0], env);
                    if (prompt.type == VAL_STRING) printf("%s", prompt.as.string);
                }
                char buffer[1024];
                if (fgets(buffer, 1024, stdin)) {
                    buffer[strcspn(buffer, "\n")] = '\0';
                    return (Value){VAL_STRING, {.string = strdup(buffer)}};
                }
                return (Value){VAL_STRING, {.string = strdup("")}};
            }

            if (strcmp(node->left->token.lexeme, "read_file") == 0) {
                if (node->child_count > 0) {
                    Value path = evaluate(node->children[0], env);
                    if (path.type == VAL_STRING) {
                        char* content = readFile(path.as.string);
                        if (content) return (Value){VAL_STRING, {.string = content}};
                    }
                }
                return (Value){VAL_NULL, {0}};
            }

            if (strcmp(node->left->token.lexeme, "write_file") == 0) {
                if (node->child_count >= 2) {
                    Value path = evaluate(node->children[0], env);
                    Value content = evaluate(node->children[1], env);
                    if (path.type == VAL_STRING && content.type == VAL_STRING) {
                        FILE* f = fopen(path.as.string, "w");
                        if (f) {
                            fputs(content.as.string, f);
                            fclose(f);
                            return (Value){VAL_BOOL, {.boolean = true}};
                        }
                    }
                }
                return (Value){VAL_BOOL, {.boolean = false}};
            }

            if (strcmp(node->left->token.lexeme, "push") == 0) {
                if (node->child_count >= 2) {
                    Value collection = evaluate(node->children[0], env);
                    Value val = evaluate(node->children[1], env);
                    if (collection.type == VAL_ARRAY) {
                        NArray* array = (NArray*)collection.as.obj;
                        if (array->size >= array->capacity) {
                            array->capacity = (array->capacity == 0) ? 8 : array->capacity * 2;
                            array->elements = (Value*)realloc(array->elements, sizeof(Value) * array->capacity);
                        }
                        array->elements[array->size++] = val;
                        return val;
                    }
                }
                return (Value){VAL_NULL, {0}};
            }

            Value callee = evaluate(node->left, env);
            Value* args = (Value*)malloc(sizeof(Value) * node->child_count);
            for (int i = 0; i < node->child_count; i++) {
                args[i] = evaluate(node->children[i], env);
            }

            if (callee.type == VAL_FN) {
                Value res = callFunction((Function*)callee.as.obj, args, node->child_count);
                free(args);
                return res;
            } else if (callee.type == VAL_CLASS) {
                ASTNode* classDecl = (ASTNode*)callee.as.obj;
                Instance* instance = (Instance*)malloc(sizeof(Instance));
                instance->count = 0;
                instance->fields = NULL;
                instance->names = NULL;
                instance->classDecl = classDecl;
                
                // Initialize fields
                for (int i = 0; i < classDecl->child_count; i++) {
                    ASTNode* member = classDecl->children[i];
                    if (member->type == NODE_VAR_DECL) {
                        Value fieldVal = evaluate(member->left, env);
                        instance->count++;
                        instance->fields = (Value*)realloc(instance->fields, sizeof(Value) * instance->count);
                        instance->names = (char**)realloc(instance->names, sizeof(char*) * instance->count);
                        instance->fields[instance->count - 1] = fieldVal;
                        instance->names[instance->count - 1] = strdup(member->token.lexeme);
                    }
                }
                
                free(args);
                return (Value){VAL_INSTANCE, {.obj = instance}};
            }
            runtimeError(node->token.line, "Can only call functions and classes.");
            free(args);
            break;
        }
        default:
            break;
    }
    return (Value){VAL_NULL, {0}};
}

void interpret(ASTNode* root) {
    Environment* global = createEnvironment(NULL);
    execute(root, global);
}

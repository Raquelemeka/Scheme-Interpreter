/**
 * Completed by Raquel Emeka with guidance from Anika, lab assistants and Dave Musicant.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <ctype.h>
 #include <string.h>
 #include <stdbool.h>
 #include "schemeval.h"
 #include "linkedlist.h"
 #include "talloc.h"
 #include "tokenizer.h"
 
 #define MAX_TOKEN_LENGTH 300
 
 // Helper function to check if a character is a valid initial character for a symbol
 bool isInitial(char c) {
     return isalpha(c) || strchr("!$%&*/:<=>?~_^", c) != NULL;
 }
 
 // Helper function to check if a character is a valid subsequent character for a symbol
 bool isSubsequent(char c) {
     return isInitial(c) || isdigit(c) || c == '.' || c == '+' || c == '-';
 }
 
 // Helper function to peek at next character
 int peek(FILE *input) {
     int c = fgetc(input);
     if (c != EOF) {
         ungetc(c, input);
     }
     return c;
 }
 
 // Helper function to create a new SchemeVal with string type
 SchemeVal *makeStringToken(char *value) {
     SchemeVal *token = talloc(sizeof(SchemeVal));
     token->type = STR_TYPE;
     token->s = talloc(strlen(value) + 1);
     strcpy(token->s, value);
     return token;
 }
 
 // Helper function to create a new SchemeVal with symbol type
 SchemeVal *makeSymbolToken(char *value) {
     SchemeVal *token = talloc(sizeof(SchemeVal));
     token->type = SYMBOL_TYPE;
     token->s = talloc(strlen(value) + 1);
     strcpy(token->s, value);
     return token;
 }
 
 // Helper function to create a new SchemeVal with integer type
 SchemeVal *makeIntToken(int value) {
     SchemeVal *token = talloc(sizeof(SchemeVal));
     token->type = INT_TYPE;
     token->i = value;
     return token;
 }
 
 // Helper function to create a new SchemeVal with double type
 SchemeVal *makeDoubleToken(double value) {
     SchemeVal *token = talloc(sizeof(SchemeVal));
     token->type = DOUBLE_TYPE;
     token->d = value;
     return token;
 }
 
 // Helper function to create a new SchemeVal with boolean type
 SchemeVal *makeBoolToken(bool value) {
     SchemeVal *token = talloc(sizeof(SchemeVal));
     token->type = BOOL_TYPE;
     token->b = value;
     return token;
 }
 
 // Helper function to create a new SchemeVal with open parenthesis type
 SchemeVal *makeOpenToken() {
     SchemeVal *token = talloc(sizeof(SchemeVal));
     token->type = OPEN_TYPE;
     return token;
 }
 
 // Helper function to create a new SchemeVal with close parenthesis type
 SchemeVal *makeCloseToken() {
     SchemeVal *token = talloc(sizeof(SchemeVal));
     token->type = CLOSE_TYPE;
     return token;
 }
 
 // Helper function to create a new SchemeVal with quote
 SchemeVal *makeQuoteToken() {
     SchemeVal *token = talloc(sizeof(SchemeVal));
     token->type = QUOTE_TYPE;
     return token;
 }
 
 // Helper function to skip whitespace and comments
 void skipWhitespaceAndComments(FILE *input) {
     int c;
     while ((c = fgetc(input)) != EOF) {
         if (isspace(c)) {
             continue;
         } else if (c == ';') {
             //skip until end of line
             while ((c = fgetc(input)) != EOF && c != '\n') {
                 continue;
             }
         } else {
             ungetc(c, input);
             break;
         }
     }
 }
 
 // Helper function to read a string token
 SchemeVal *readString(FILE *input) {
     char buffer[MAX_TOKEN_LENGTH + 1];
     int index = 0;
     int c;
     
     while ((c = fgetc(input)) != EOF && c != '"' && index < MAX_TOKEN_LENGTH) {
         if (c == '\\') {
             // Handle escape sequences
             int next = fgetc(input);
             if (next == EOF) {
                 fprintf(stderr, "Syntax error: unterminated string literal\n");
                 texit(1);
             }
             buffer[index++] = next;
         } else {
             buffer[index++] = c;
         }
     }
     
     if (c != '"') {
         fprintf(stderr, "Syntax error: unterminated string literal\n");
         texit(1);
     }
     
     buffer[index] = '\0';
     return makeStringToken(buffer);
 }
 
 // Helper function to read a number token
 SchemeVal *readNumber(FILE *input, int firstChar) {
     char buffer[MAX_TOKEN_LENGTH + 1];
     int index = 0;
     bool hasDecimal = false;
     
     buffer[index++] = firstChar;
     
     int c;
     while ((c = fgetc(input)) != EOF && (isdigit(c) || c == '.') && index < MAX_TOKEN_LENGTH) {
         if (c == '.') {
             if (hasDecimal) {
                 fprintf(stderr, "Syntax error: invalid number format\n");
                 texit(1);
             }
             hasDecimal = true;
         }
         buffer[index++] = c;
     }
     
     // Put back the last character if it's not part of the number
     if (!isdigit(c) && c != '.') {
         ungetc(c, input);
     }
     
     buffer[index] = '\0';
     
     if (hasDecimal) {
         double value;
         if (sscanf(buffer, "%lf", &value) != 1) {
             fprintf(stderr, "Syntax error: invalid floating-point\n");
             texit(1);
         }
         return makeDoubleToken(value);
     } else {
         int value;
         if (sscanf(buffer, "%d", &value) != 1) {
             fprintf(stderr, "Syntax error: invalid integer\n");
             texit(1);
         }
         return makeIntToken(value);
     }
 }
 
 // Helper function to read a symbol token
 SchemeVal *readSymbol(FILE *input, int firstChar) {
     char buffer[MAX_TOKEN_LENGTH + 1];
     int index = 0;
     
     buffer[index++] = firstChar;
     
     int c;
     while ((c = fgetc(input)) != EOF && isSubsequent(c) && index < MAX_TOKEN_LENGTH) {
         buffer[index++] = c;
     }
     
     // put back the last character if it's not part of the symbol
     if (!isSubsequent(c)) {
         ungetc(c, input);
     }
     
     buffer[index] = '\0';
     
     if (strcmp(buffer, "#t") == 0) {
         return makeBoolToken(true);
     } else if (strcmp(buffer, "#f") == 0) {
         return makeBoolToken(false);
     }
     
     return makeSymbolToken(buffer);
 }
 
 // main tokenize function
 // reads a Scheme program from stdin and turns it into a list of tokens.
 // It ignores spaces and comments, and finds numbers, strings, symbols,
 // booleans, parentheses, and quotes. The tokens are returned in the order they appear.
 SchemeVal *tokenize() {
     SchemeVal *list = makeEmpty();
     SchemeVal *tail = makeEmpty();
     FILE *input = stdin;
     int c;
     
     while (1) {
         skipWhitespaceAndComments(input);
         c = fgetc(input);
         if (c == EOF) break;
         
         SchemeVal *token = NULL;
         
         if (c == '(') {
             token = makeOpenToken();
         } else if (c == ')') {
             token = makeCloseToken();
         } else if (c == '\'') {
             token = makeQuoteToken();
         } else if (c == '"') {
             token = readString(input);
         } else if (isdigit(c) || (c == '-' && isdigit(peek(input)))) {
             token = readNumber(input, c);
         } else if (c == '#') {
             int next = fgetc(input);
             if (next == 't' || next == 'f') {
                 token = makeBoolToken(next == 't');
             } else {
                 fprintf(stderr, "Syntax error: invalid boolean\n");
                 texit(1);
             }
         } else if (isInitial(c)) {
             token = readSymbol(input, c);
         } else if (c == '+' || c == '-') {
             char op[2] = {c, '\0'};
             token = makeSymbolToken(op);
         } else {
             fprintf(stderr, "Syntax error: invalid character '%c'\n", c);
             texit(1);
         }
         
         if (token != NULL) {
             if (isEmpty(list)) {
                 list = cons(token, makeEmpty());
                 tail = list;
             } else {
                 SchemeVal *newCell = cons(token, makeEmpty());
                 tail->cdr = newCell;
                 tail = newCell;
             }
         }
     }
     
     return list;
 }
 
 // Function to display tokens
 void displayTokens(SchemeVal *list) {
     while (!isEmpty(list)) {
         SchemeVal *current = car(list);
         
         switch (current->type) {
             case INT_TYPE:
                 printf("%d:integer\n", current->i);
                 break;
             case DOUBLE_TYPE:
                 printf("%g:double\n", current->d);
                 break;
             case STR_TYPE:
                 printf("\"%s\":string\n", current->s);
                 break;
             case SYMBOL_TYPE:
                 printf("%s:symbol\n", current->s);
                 break;
             case BOOL_TYPE:
                 printf("#%c:boolean\n", current->b ? 't' : 'f');
                 break;
             case OPEN_TYPE:
                 printf("(:open\n");
                 break;
             case CLOSE_TYPE:
                 printf("):close\n");
                 break;
             case QUOTE_TYPE:
                 printf("':quote\n");
                 break;
             default:
                 printf("?:unknown\n");
         }
         
         list = cdr(list);
     }
 }

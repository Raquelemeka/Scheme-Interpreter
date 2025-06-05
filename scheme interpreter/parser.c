#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "parser.h"
#include "linkedlist.h"
#include "schemeval.h"
#include "talloc.h"
#include "tokenizer.h"

/* Adds token to parse tree stack, handles parentheses and quotes. 
   Input: stack, current depth, token to add. Output: updated stack */
SchemeVal *addToParseTree(SchemeVal *stack, int *depth, SchemeVal *token) {
    assert(stack != NULL);

    if (token->type == CLOSE_TYPE) {
        SchemeVal *elements = makeEmpty();  
        bool found_open = false;

        // Pop elements until matching OPEN is found
        while (!isEmpty(stack)) {
            SchemeVal *top = car(stack);
            stack = cdr(stack);

            if (top->type == OPEN_TYPE) {
                found_open = true;
                *depth -= 1;
                break;  // Stop at the matching open parenthesis
            }
            elements = cons(top, elements); 
        }

        if (!found_open) {
            printf("Syntax error: unmatched close parenthesis\n");
            texit(1);
        }

        SchemeVal *subtree = elements;

        if (!isEmpty(stack) && car(stack)->type == QUOTE_TYPE) {
            stack = cdr(stack);  // pop the quote
            subtree = cons(makeSymbolToken("quote"), cons(subtree, makeEmpty()));
        }

        return cons(subtree, stack);
    }
    else if (token->type == OPEN_TYPE) {
        *depth += 1;
        return cons(token, stack);
    }
    else if (token->type == QUOTE_TYPE) {
        return cons(token, stack);
    }
    else {
        // normal tokens 
        if (!isEmpty(stack) && car(stack)->type == QUOTE_TYPE) {
            stack = cdr(stack);
            return cons(cons(makeSymbolToken("quote"), cons(token, makeEmpty())), stack);
        }
        return cons(token, stack);
    }
}

/* Parses list of tokens into a syntax tree. 
   Input: list of tokens. Output: parsed syntax tree */
SchemeVal *parse(SchemeVal *tokens) {
    SchemeVal *stack = makeEmpty();
    int depth = 0;
    SchemeVal *current = tokens;

    while (!isEmpty(current)) {
        SchemeVal *token = car(current);
        stack = addToParseTree(stack, &depth, token);
        current = cdr(current);
    }

    if (depth != 0) {
        printf("Syntax error: not enough close parentheses\n");
        texit(1);
    }

    // Handle any remaining top-level quotes
    while (!isEmpty(stack) && car(stack)->type == QUOTE_TYPE) {
        if (isEmpty(cdr(stack))) {
            printf("Syntax error: quote without expression\n");
            texit(1);
        }
        stack = cdr(stack);  // pop the quote
        SchemeVal *quoted = cons(makeSymbolToken("quote"), 
                               cons(car(stack), makeEmpty()));
        stack = cons(quoted, cdr(stack));
    }

    return reverse(stack);
}

/* Helper function to recursively print syntax tree nodes */
void printTreeHelper(SchemeVal *tree) {
    if (tree == NULL) return;

    switch (tree->type) {
        case EMPTY_TYPE:
            printf("()"); 
            break;
        case CONS_TYPE:
            // Check for (quote ...) 
            if (tree->car->type == SYMBOL_TYPE && !strcmp(tree->car->s, "quote") && 
                tree->cdr->type == CONS_TYPE && tree->cdr->cdr->type == EMPTY_TYPE) {
                SchemeVal *quoted = tree->cdr->car;
                printf("(quote ");
                printTreeHelper(quoted);
                printf(")");
            } else {
                printf("(");
                while (tree->type == CONS_TYPE) {
                    printTreeHelper(car(tree));
                    tree = cdr(tree);
                    if (tree->type != EMPTY_TYPE) printf(" ");
                }
                if (tree->type != EMPTY_TYPE) {
                    printf(" . ");
                    printTreeHelper(tree);
                }
                printf(")");
            }
            break;
        case INT_TYPE:
            printf("%d", tree->i);
            break;
        case DOUBLE_TYPE:
            printf("%g", tree->d);
            break;
        case STR_TYPE:
            printf("\"%s\"", tree->s);
            break;
        case CLOSURE_TYPE:
            printf("#<procedure>");
            break;
        case SYMBOL_TYPE:
            // special case for the quote symbol itself
            if (!strcmp(tree->s, "quote")) {
                printf("quote");
            } else {
                printf("%s", tree->s);
            }
            break;
        case BOOL_TYPE:
            printf("#%c", tree->b ? 't' : 'f');
            break;
        default:
            printf("");
            break;
    }
}

/* Prints the entire syntax tree. Input: syntax tree to print */
void printTree(SchemeVal *tree) {
    SchemeVal *current = tree;
    while (!isEmpty(current)) {
        printTreeHelper(car(current));
        printf("\n");
        current = cdr(current);
    }
}

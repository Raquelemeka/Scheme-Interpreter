#include "schemeval.h"

#ifndef _TOKENIZER
#define _TOKENIZER

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
SchemeVal *tokenize();

// Displays the contents of the linked list as tokens, with type information
void displayTokens(SchemeVal *list);

#endif

#include <stdio.h>
#include "tokenizer.h"
#include "schemeval.h"
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "interpreter.h"

int main() {

    SchemeVal *list = tokenize();
    SchemeVal *tree = parse(list);
    interpret(tree);

    tfree();
    return 0;
}

#ifndef _INTERPRETER
#define _INTERPRETER

#include "schemeval.h"

void interpret(SchemeVal *tree);
SchemeVal *eval(SchemeVal *tree, Frame *frame);

#endif


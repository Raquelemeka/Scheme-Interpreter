#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "schemeval.h"
#include "interpreter.h"
#include "talloc.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "parser.h"



/* Forward declarations for helper functions */
SchemeVal *makeVoid();
SchemeVal *evalEach(SchemeVal *args, Frame *frame);
SchemeVal *apply(SchemeVal *function, SchemeVal *args, Frame *frame);
SchemeVal *evalLambda(SchemeVal *args, Frame *frame);

// + can take any number of integer/real arguments
// Input: SchemeVal* args - list of numbers (int or double)
// Output: SchemeVal* - sum
SchemeVal *primitiveAdd(SchemeVal *args) {
    SchemeVal *result = talloc(sizeof(SchemeVal));
    result->type = INT_TYPE;
    result->i = 0;
    
    bool hasDouble = false;
    
    while (!isEmpty(args)) {
        SchemeVal *arg = car(args);
        if (arg->type != INT_TYPE && arg->type != DOUBLE_TYPE) {
            printf("Evaluation error: + requires numbers\n");
            texit(1);
        }
        
        if (arg->type == DOUBLE_TYPE) {
            hasDouble = true;
            if (result->type == INT_TYPE) {
                result->type = DOUBLE_TYPE;
                result->d = result->i;
            }
            result->d += arg->d;
        } else {
            if (hasDouble) {
                result->d += arg->i;
            } else {
                result->i += arg->i;
            }
        }
        
        args = cdr(args);
    }
    
    return result;
}

// < comparison function
// Input: SchemeVal* args - list of numbers (int or double)
// Output: SchemeVal* - bool_TYPE true if all arguments are in increasing order, false otherwise
SchemeVal *primitiveLessThan(SchemeVal *args) {
    if (isEmpty(args) || isEmpty(cdr(args))) {
        printf("Evaluation error: < requires at least 2 arguments\n");
        texit(1);
    }

    SchemeVal *result = talloc(sizeof(SchemeVal));
    result->type = BOOL_TYPE;
    result->b = true;

    SchemeVal *current = args;
    SchemeVal *prev = car(current);
    current = cdr(current);

    if (prev->type != INT_TYPE && prev->type != DOUBLE_TYPE) {
        printf("Evaluation error: < requires numbers\n");
        texit(1);
    }

    while (!isEmpty(current)) {
        SchemeVal *next = car(current);

        if (next->type != INT_TYPE && next->type != DOUBLE_TYPE) {
            printf("Evaluation error: < requires numbers\n");
            texit(1);
        }

        bool comparison;
        if (prev->type == DOUBLE_TYPE || next->type == DOUBLE_TYPE) {
            double prevVal = prev->type == DOUBLE_TYPE ? prev->d : prev->i;
            double nextVal = next->type == DOUBLE_TYPE ? next->d : next->i;
            comparison = (prevVal < nextVal);
        } else {
            comparison = (prev->i < next->i);
        }

        if (!comparison) {
            result->b = false;
            break;
        }

        prev = next;
        current = cdr(current);
    }

    return result;
}

// null? checks if argument is empty list
// Input: schemeVal* args - single argument list
// Output: SchemeVal* - bool_TYPE true if arg is empty list, false otherwise
SchemeVal *primitiveNull(SchemeVal *args) {
    if (length(args) != 1) {
        printf("Evaluation error: null? requires exactly one argument\n");
        texit(1);
    }
    
    SchemeVal *result = talloc(sizeof(SchemeVal));
    result->type = BOOL_TYPE;
    result->b = (car(args)->type == EMPTY_TYPE);
    return result;
}

// car returns first element of pair
// Input: SchemeVal* args - single argument list containing a pair (CONS_TYPE)
// Output: SchemeVal* - first element of the pair
SchemeVal *primitiveCar(SchemeVal *args) {
    if (length(args) != 1) {
        printf("Evaluation error: car requires exactly one argument\n");
        texit(1);
    }
    
    SchemeVal *pair = car(args);
    if (pair->type != CONS_TYPE) {
        printf("Evaluation error: car requires a pair\n");
        texit(1);
    }
    
    return pair->car;
}

// cdr returns second element of pair
// Input: SchemeVal* args - single argument list containing a pair
// Output: SchemeVal* - second element of the pair
SchemeVal *primitiveCdr(SchemeVal *args) {
    if (length(args) != 1) {
        printf("Evaluation error: cdr requires exactly one argument\n");
        texit(1);
    }
    
    SchemeVal *pair = car(args);
    if (pair->type != CONS_TYPE) {
        printf("Evaluation error: cdr requires a pair\n");
        texit(1);
    }
    
    return pair->cdr;
}

// cons creates a new pair from two arguments
// input: SchemeVal* args - list of exactly two elements
// output: SchemeVal* - new pair (CONS_TYPE) with car and cdr from args
SchemeVal *primitiveCons(SchemeVal *args) {
    if (length(args) != 2) {
        printf("Evaluation error: cons requires exactly two arguments\n");
        texit(1);
    }
    
    SchemeVal *pair = talloc(sizeof(SchemeVal));
    pair->type = CONS_TYPE;
    pair->car = car(args);
    pair->cdr = car(cdr(args));
    return pair;
}

// map applies function to each element of list
// Input: SchemeVal* args -  function and list
// Output: SchemeVal* - list of results after applying function to each element
SchemeVal *primitiveMap(SchemeVal *args) {
    if (length(args) != 2) {
        printf("Evaluation error: map requires exactly two arguments\n");
        texit(1);
    }
    
    SchemeVal *func = car(args);
    SchemeVal *lst = car(cdr(args));
    
    if (func->type != CLOSURE_TYPE && func->type != PRIMITIVE_TYPE) {
        printf("Evaluation error: first argument to map must be a function\n");
        texit(1);
    }
    
    SchemeVal *result = makeEmpty();
    SchemeVal *tail = makeEmpty();
    
    while (!isEmpty(lst)) {
        if (lst->type != CONS_TYPE) {
            printf("Evaluation error: second argument to map must be a list\n");
            texit(1);
        }
        
        SchemeVal *arg = car(lst);
        SchemeVal *applied;
        
        if (func->type == CLOSURE_TYPE) {
            SchemeVal *argList = cons(arg, makeEmpty());
            applied = apply(func, argList, func->frame);
        } else {
            SchemeVal *argList = cons(arg, makeEmpty());
            applied = func->pf(argList);
        }
        
        if (isEmpty(result)) {
            result = cons(applied, makeEmpty());
            tail = result;
        } else {
            tail->cdr = cons(applied, makeEmpty());
            tail = tail->cdr;
        }
        
        lst = cdr(lst);
    }
    
    return result;
}

//Creates and returns a VOID_TYPE SchemeVal (used for define expr)
SchemeVal *makeVoid() {
    SchemeVal *voidVal = talloc(sizeof(SchemeVal));
    voidVal->type = VOID_TYPE;
    return voidVal;
}


// Evaluates each argument in a list and returns a new list of evaluated values
// Input: SchemeVal* args - list of arguments to evaluate
// Output: SchemeVal* - new list of evaluated values
SchemeVal *evalEach(SchemeVal *args, Frame *frame) {
    SchemeVal *result = makeEmpty();
    SchemeVal *tail = makeEmpty();
    
    while (!isEmpty(args)) {
        SchemeVal *evaled = eval(car(args), frame);
        
        if (isEmpty(result)) {
            result = cons(evaled, makeEmpty());
            tail = result;
        } else {
            tail->cdr = cons(evaled, makeEmpty());
            tail = tail->cdr;
        }
        args = cdr(args);
    }
    
    return result;
}

// Applies a closure to a list of argument values in a new environment.
// Input: A function (closure), a list of evaluated arguments, and the current frame
// Output: The result of evaluating the function body in the new frame
SchemeVal *apply(SchemeVal *function, SchemeVal *args, Frame *frame) {
    if (function->type == PRIMITIVE_TYPE) {
        return function->pf(args);
    }
    else if (function->type != CLOSURE_TYPE) {
        printf("Evaluation error: not a procedure\n");
        texit(1);
    }

    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = function->frame;
    newFrame->bindings = makeEmpty();

    SchemeVal *params = function->paramNames;
    SchemeVal *argVals = args;

    while (!isEmpty(params) && !isEmpty(argVals)) {
        SchemeVal *param = car(params);
        SchemeVal *argVal = car(argVals);

        SchemeVal *binding = cons(param, argVal);
        newFrame->bindings = cons(binding, newFrame->bindings);

        params = cdr(params);
        argVals = cdr(argVals);
    }

    if (!isEmpty(params) || !isEmpty(argVals)) {
        printf("Evaluation error: incorrect number of arguments\n");
        texit(1);
    }

    SchemeVal *result = NULL;
    SchemeVal *body = function->functionCode;
    while (!isEmpty(body)) {
        result = eval(car(body), newFrame);
        body = cdr(body);
    }

    return result;
}

// Constructs and returns a closure from lambda parameters and body expressions.
// Input: A list where the first element is a parameter list and the rest is the body and the current frame (environment)
// output: A SchemeVal representing a closure
SchemeVal *evalLambda(SchemeVal *args, Frame *frame) {
    if (isEmpty(args) || isEmpty(cdr(args))) {
        printf("Evaluation error: lambda needs parameters and body\n");
        texit(1);
    }

    SchemeVal *params = car(args);
    SchemeVal *body = cdr(args);

    if (params->type == SYMBOL_TYPE) {
        params = cons(params, makeEmpty());
    }
    else {
        SchemeVal *temp = params;
        SchemeVal *seen = makeEmpty();
        while (!isEmpty(temp)) {
            if (temp->type != CONS_TYPE || car(temp)->type != SYMBOL_TYPE) {
                printf("Evaluation error: lambda parameters must be symbols\n");
                texit(1);
            }
            
            SchemeVal *check = seen;
            while (!isEmpty(check)) {
                if (!strcmp(car(temp)->s, car(check)->s)) {
                    printf("Evaluation error: duplicate parameter %s\n", car(temp)->s);
                    texit(1);
                }
                check = cdr(check);
            }
            seen = cons(car(temp), seen);
            
            temp = cdr(temp);
        }
    }

    SchemeVal *closure = talloc(sizeof(SchemeVal));
    closure->type = CLOSURE_TYPE;
    closure->paramNames = params;
    closure->functionCode = body;
    closure->frame = frame;
    return closure;
}

// Looks up the value of a symbol in the environment
// Input: A SchemeVal symbol and the current frame
// Output: The SchemeVal bound to the symbol, or an error if unbound
SchemeVal *lookUpSymbol(SchemeVal *symbol, Frame *frame) {
    Frame *curr = frame;
    while (curr != NULL) {
        SchemeVal *bindings = curr->bindings;
        while (!isEmpty(bindings)) {
            SchemeVal *pair = car(bindings);
            SchemeVal *key = car(pair);
            SchemeVal *value = cdr(pair);

            if (!strcmp(symbol->s, key->s)) {
                return value;
            }
            bindings = cdr(bindings);
        }
        curr = curr->parent;
    }

    printf("Evaluation error: unbound variable %s\n", symbol->s);
    texit(1);
    return NULL;
}

// Evaluates an if expression.
// Input: SchemeVal* args (test, true branch, optional false branch), Frame* frame
// Output: SchemeVal* (result of either true or false branch)
SchemeVal *evalIf(SchemeVal *args, Frame *frame) {
    int count = 0;
    SchemeVal *temp = args;
    while (!isEmpty(temp)) {
        count++;
        temp = cdr(temp);
    }

    if (count != 2 && count != 3) {
        printf("Evaluation error: if requires 2 or 3 expressions\n");
        texit(1);
    }

    SchemeVal *testExpr = car(args);
    SchemeVal *trueExpr = car(cdr(args));
    SchemeVal *falseExpr = count == 3 ? car(cdr(cdr(args))) : NULL;

    SchemeVal *testResult = eval(testExpr, frame);
    bool condition = !(testResult->type == BOOL_TYPE && !testResult->b);

    if (condition) {
        return eval(trueExpr, frame);
    } else {
        if (falseExpr == NULL) {
            printf("Evaluation error: missing else clause\n");
            texit(1);
        }
        return eval(falseExpr, frame);
    }
}

// Evaluates a let expression.
// Input: SchemeVal* args (bindings and body), Frame* parent
// Output: result of evaluating the body
SchemeVal *evalLet(SchemeVal *args, Frame *parent) {
    if (isEmpty(args)) {
        printf("Evaluation error: let needs bindings and body\n");
        texit(1);
    }

    SchemeVal *bindings = car(args);
    SchemeVal *body = cdr(args);

    if (bindings->type != CONS_TYPE && bindings->type != EMPTY_TYPE) {
        printf("Evaluation error: malformed bindings\n");
        texit(1);
    }

    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = parent;
    newFrame->bindings = makeEmpty();

    SchemeVal *seenBindings = makeEmpty();
    SchemeVal *current = bindings;
    while (!isEmpty(current)) {
        SchemeVal *binding = car(current);
        if (binding->type != CONS_TYPE || isEmpty(cdr(binding)) || !isEmpty(cdr(cdr(binding)))) {
            printf("Evaluation error: invalid binding form\n");
            texit(1);
        }

        SchemeVal *var = car(binding);
        if (var->type != SYMBOL_TYPE) {
            printf("Evaluation error: binding name must be a symbol\n");
            texit(1);
        }

        SchemeVal *check = seenBindings;
        while (!isEmpty(check)) {
            if (!strcmp(car(check)->s, var->s)) {
                printf("Evaluation error: duplicate binding '%s'\n", var->s);
                texit(1);
            }
            check = cdr(check);
        }
        seenBindings = cons(var, seenBindings);
        current = cdr(current);
    }

    current = bindings;
    while (!isEmpty(current)) {
        SchemeVal *binding = car(current);
        SchemeVal *var = car(binding);
        SchemeVal *valExpr = car(cdr(binding));
        SchemeVal *val = eval(valExpr, parent);
        newFrame->bindings = cons(cons(var, val), newFrame->bindings);
        current = cdr(current);
    }

    if (isEmpty(body)) {
        printf("Evaluation error: let body missing\n");
        texit(1);
    }

    SchemeVal *result = NULL;
    SchemeVal *currExpr = body;
    while (!isEmpty(currExpr)) {
        result = eval(car(currExpr), newFrame);
        currExpr = cdr(currExpr);
    }

    return result;
}

// Evaluates a letrec expression.
// Input: SchemeVal* args (bindings and body), Frame* parent
// Output: result of evaluating the body
SchemeVal *evalLetrec(SchemeVal *args, Frame *parent) {
    if (isEmpty(args)) {
        printf("Evaluation error: letrec needs bindings and body\n");
        texit(1);
    }

    SchemeVal *bindings = car(args);
    SchemeVal *body = cdr(args);

    if (bindings->type != CONS_TYPE && bindings->type != EMPTY_TYPE) {
        printf("Evaluation error: malformed bindings\n");
        texit(1);
    }

    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = parent;
    newFrame->bindings = makeEmpty();

    // create all bindings as unspecified
    SchemeVal *current = bindings;
    while (!isEmpty(current)) {
        SchemeVal *binding = car(current);
        if (binding->type != CONS_TYPE || isEmpty(cdr(binding)) || !isEmpty(cdr(cdr(binding)))) {
            printf("Evaluation error: invalid binding form\n");
            texit(1);
        }

        SchemeVal *var = car(binding);
        if (var->type != SYMBOL_TYPE) {
            printf("Evaluation error: binding name must be a symbol\n");
            texit(1);
        }

        // Check for duplicate bindings
        SchemeVal *existing = newFrame->bindings;
        while (!isEmpty(existing)) {
            SchemeVal *pair = car(existing);
            if (!strcmp(var->s, car(pair)->s)) {
                printf("Evaluation error: duplicate binding '%s'\n", var->s);
                texit(1);
            }
            existing = cdr(existing);
        }

        SchemeVal *unspecified = talloc(sizeof(SchemeVal));
        unspecified->type = UNSPECIFIED_TYPE;
        newFrame->bindings = cons(cons(var, unspecified), newFrame->bindings);
        current = cdr(current);
    }

    // evaluate all right-hand sides first (without assigning)
    SchemeVal *values = makeEmpty();
    current = bindings;
    while (!isEmpty(current)) {
        SchemeVal *binding = car(current);
        SchemeVal *valExpr = car(cdr(binding));
        SchemeVal *val = eval(valExpr, newFrame);
        values = cons(val, values);
        current = cdr(current);
    }
    values = reverse(values);

    // check for circular references
    current = values;
    while (!isEmpty(current)) {
        if (car(current)->type == UNSPECIFIED_TYPE) {
            printf("Evaluation error: circular reference in letrec\n");
            texit(1);
        }
        current = cdr(current);
    }

    // assign all the values
    current = bindings;
    SchemeVal *vals = values;
    while (!isEmpty(current) && !isEmpty(vals)) {
        SchemeVal *binding = car(current);
        SchemeVal *var = car(binding);
        
        // Update the binding
        SchemeVal *bindingsList = newFrame->bindings;
        while (!isEmpty(bindingsList)) {
            SchemeVal *pair = car(bindingsList);
            if (!strcmp(var->s, car(pair)->s)) {
                pair->cdr = car(vals);
                break;
            }
            bindingsList = cdr(bindingsList);
        }
        current = cdr(current);
        vals = cdr(vals);
    }

    // Evaluate body
    if (isEmpty(body)) {
        printf("Evaluation error: letrec body missing\n");
        texit(1);
    }

    SchemeVal *result = NULL;
    current = body;
    while (!isEmpty(current)) {
        result = eval(car(current), newFrame);
        current = cdr(current);
    }

    return result;
}

// Evaluates a set! expression.
// Input: SchemeVal* args (variable and value), Frame* frame
// Output: SchemeVal* - void value if successful, or error if variable not found
SchemeVal *evalSet(SchemeVal *args, Frame *frame) {
    if (isEmpty(args) || isEmpty(cdr(args)) || !isEmpty(cdr(cdr(args)))) {
        printf("Evaluation error: set! requires exactly 2 arguments\n");
        texit(1);
    }

    SchemeVal *var = car(args);
    if (var->type != SYMBOL_TYPE) {
        printf("Evaluation error: set! variable must be a symbol\n");
        texit(1);
    }

    SchemeVal *valExpr = car(cdr(args));
    SchemeVal *value = eval(valExpr, frame);

    Frame *curr = frame;
    while (curr != NULL) {
        SchemeVal *bindings = curr->bindings;
        while (!isEmpty(bindings)) {
            SchemeVal *pair = car(bindings);
            if (!strcmp(var->s, car(pair)->s)) {
                pair->cdr = value;
                return makeVoid();
            }
            bindings = cdr(bindings);
        }
        curr = curr->parent;
    }

    printf("Evaluation error: unbound variable %s\n", var->s);
    texit(1);
    return NULL;
}

// Evaluates a Scheme expression in the given frame.
// Input: SchemeVal* expr (expression), Frame* frame (context)
// Output: SchemeVal* (evaluated result)
SchemeVal *eval(SchemeVal *expr, Frame *frame) {
    switch (expr->type) {
        case INT_TYPE:
        case DOUBLE_TYPE:
        case STR_TYPE:
        case BOOL_TYPE:
            return expr;

        case SYMBOL_TYPE:
            return lookUpSymbol(expr, frame);

        case CONS_TYPE: {
            SchemeVal *first = car(expr);
            SchemeVal *args = cdr(expr);

            if (first->type == CONS_TYPE) {
                SchemeVal *proc = eval(first, frame);
                SchemeVal *evalledArgs = evalEach(args, frame);
                return apply(proc, evalledArgs, frame);
            }
            else if (first->type != SYMBOL_TYPE) {
                printf("Evaluation error: bad form\n");
                texit(1);
            }

            if (!strcmp(first->s, "if")) {
                return evalIf(args, frame);
            } 
            else if (!strcmp(first->s, "let")) {
                return evalLet(args, frame);
            }
            else if (!strcmp(first->s, "letrec")) {
                return evalLetrec(args, frame);
            }
            else if (!strcmp(first->s, "define")) {
                if (isEmpty(args) || isEmpty(cdr(args)) || !isEmpty(cdr(cdr(args)))) {
                    printf("Evaluation error: define requires exactly 2 arguments\n");
                    texit(1);
                }

                SchemeVal *var = car(args);
                if (var->type != SYMBOL_TYPE) {
                    printf("Evaluation error: define variable must be a symbol\n");
                    texit(1);
                }

                SchemeVal *bindings = frame->bindings;
                while (!isEmpty(bindings)) {
                    SchemeVal *pair = car(bindings);
                    if (!strcmp(var->s, car(pair)->s)) {
                        printf("Evaluation error: %s already defined\n", var->s);
                        texit(1);
                    }
                    bindings = cdr(bindings);
                }

                SchemeVal *value = eval(car(cdr(args)), frame);
                frame->bindings = cons(cons(var, value), frame->bindings);
                return makeVoid();
            }
            else if (!strcmp(first->s, "set!")) {
                return evalSet(args, frame);
            }
            else if (!strcmp(first->s, "lambda")) {
                return evalLambda(args, frame);
            }
            else if (!strcmp(first->s, "quote")) {
                if (isEmpty(args) || !isEmpty(cdr(args))) {
                    printf("Evaluation error: quote requires one expression\n");
                    texit(1);
                }
                return car(args);
            }
            else {
                SchemeVal *proc = eval(first, frame);
                SchemeVal *evalledArgs = evalEach(args, frame);
                return apply(proc, evalledArgs, frame);
            }
        }

        default:
            printf("Evaluation error: unsupported expression type\n");
            texit(1);
            return NULL;
    }
}

// Binds a primitive function to a name in a frame
void bind(char *name, SchemeVal *(*function)(SchemeVal *), Frame *frame) {
    SchemeVal *value = talloc(sizeof(SchemeVal));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    
    SchemeVal *symbol = talloc(sizeof(SchemeVal));
    symbol->type = SYMBOL_TYPE;
    symbol->s = name;
    
    frame->bindings = cons(cons(symbol, value), frame->bindings);
}

// Interprets a list of Scheme expressions and prints results.
// Input: SchemeVal* tree (list of expressions)
void interpret(SchemeVal *tree) {
    Frame *global = talloc(sizeof(Frame));
    global->bindings = makeEmpty();
    global->parent = NULL;

    bind("+", primitiveAdd, global);
    bind("<", primitiveLessThan, global);
    bind("null?", primitiveNull, global);
    bind("car", primitiveCar, global);
    bind("cdr", primitiveCdr, global);
    bind("cons", primitiveCons, global);
    bind("map", primitiveMap, global);

    while (!isEmpty(tree)) {
        SchemeVal *result = eval(car(tree), global);
        printTreeHelper(result);
        printf("\n");
        tree = cdr(tree);
    }
}
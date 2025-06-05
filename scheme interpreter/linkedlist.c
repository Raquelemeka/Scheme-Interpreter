#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "schemeval.h"
#include "linkedlist.h"
#include "talloc.h"

/* Creates and returns a new EMPTY_TYPE SchemeVal */
SchemeVal *makeEmpty() {
    SchemeVal *empty = (SchemeVal *)talloc(sizeof(SchemeVal));  
    assert(empty != NULL);
    empty->type = EMPTY_TYPE;
    return empty;
}

/* Creates a new cons cell with given car and cdr */
SchemeVal *cons(SchemeVal *newCar, SchemeVal *newCdr) {
    SchemeVal *cell = (SchemeVal *)talloc(sizeof(SchemeVal));  
    assert(cell != NULL);
    cell->type = CONS_TYPE;
    cell->car = newCar;
    cell->cdr = newCdr;
    return cell;
}

/* Returns the car of a cons cell */
SchemeVal *car(SchemeVal *list) {
    assert(list != NULL && list->type == CONS_TYPE);
    return list->car;
}

/* Returns the cdr of a cons cell */
SchemeVal *cdr(SchemeVal *list) {
    assert(list != NULL && list->type == CONS_TYPE);
    return list->cdr;
}

/* Checks if a SchemeVal is EMPTY_TYPE */
bool isEmpty(SchemeVal *value) {
    assert(value != NULL);
    return (value->type == EMPTY_TYPE);
}

/* Calculates the length of a proper list */
int length(SchemeVal *value) {
    assert(value != NULL);
    int count = 0;
    while (value->type != EMPTY_TYPE) {
        assert(value->type == CONS_TYPE);
        count++;
        value = value->cdr;
    }
    return count;
}

/* Displays the list contents */
void display(SchemeVal *list) {
    assert(list != NULL);
    printf("(");
    while (list->type != EMPTY_TYPE) {
        assert(list->type == CONS_TYPE);
        SchemeVal *current = list->car;
        
        switch (current->type) {
            case INT_TYPE:
                printf("%d", current->i);
                break;
            case DOUBLE_TYPE:
                printf("%f", current->d);
                break;
            case STR_TYPE:
                printf("\"%s\"", current->s);
                break;
            case CONS_TYPE:
                display(current);
                break;
            case EMPTY_TYPE:
                printf("()");
                break;
            default:
                printf("?");
        }
        
        list = list->cdr;
        if (list->type != EMPTY_TYPE) printf(" ");
    }
    printf(")\n");
}

/* Creates a new reversed copy of the list */
SchemeVal *reverse(SchemeVal *list) {
    assert(list != NULL);
    SchemeVal *newList = makeEmpty();
    
    while (list->type != EMPTY_TYPE) {
        assert(list->type == CONS_TYPE);
        //shares the car pointer
        newList = cons(list->car, newList);
        list = list->cdr;
    }
    
    return newList;
}


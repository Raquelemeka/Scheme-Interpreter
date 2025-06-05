#include <stdbool.h>
#include "schemeval.h"

#ifndef _LINKEDLIST
#define _LINKEDLIST

// Create a new EMPTY_TYPE value node.
SchemeVal *makeEmpty();

// Create a new CONS_TYPE value node.
SchemeVal *cons(SchemeVal *newCar, SchemeVal *newCdr);

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(SchemeVal *list);

// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
SchemeVal *reverse(SchemeVal *list);

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
SchemeVal *car(SchemeVal *list);

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
SchemeVal *cdr(SchemeVal *list);

// Utility to check if pointing to a EMPTY_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isEmpty(SchemeVal *value);

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(SchemeVal *value);


#endif

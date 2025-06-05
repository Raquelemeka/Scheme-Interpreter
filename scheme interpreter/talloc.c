#include "talloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static SchemeVal *active_list = NULL;

// Allocates memory and tracks it for later cleanup.
// Returns a pointer to the allocated memory, or NULL if allocation fails.
void *talloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) return NULL;

    SchemeVal *node = malloc(sizeof(SchemeVal));
    assert(node != NULL);
    
    node->type = CONS_TYPE;
    node->car = (SchemeVal*)ptr;
    node->cdr = active_list;

    active_list = node;
    return ptr;
}

// Frees all memory previously allocated with talloc.
// Takes no input and returns nothing.
void tfree() {
    while (active_list != NULL) {
        SchemeVal *current = active_list;
        active_list = current->cdr;
        
        free(current->car);
        free(current);
    }
}

// Frees all tracked memory and exits the program with the given status code.
void texit(int status) {
    tfree();
    exit(status);
}

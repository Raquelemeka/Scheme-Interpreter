#ifndef _SCHEMEVAL
#define _SCHEMEVAL

#include <stdbool.h>

typedef enum {
  INT_TYPE, DOUBLE_TYPE, STR_TYPE, CONS_TYPE, EMPTY_TYPE, PTR_TYPE,
  OPEN_TYPE, CLOSE_TYPE, BOOL_TYPE, SYMBOL_TYPE, QUOTE_TYPE,
  UNSPECIFIED_TYPE, VOID_TYPE, CLOSURE_TYPE, PRIMITIVE_TYPE
} objectType;

typedef struct SchemeVal {
    objectType type;
    union {
        int i;
        double d;
        char *s;
        struct {
            struct SchemeVal *car;
            struct SchemeVal *cdr;
        }; // For CONS_TYPE
        struct {
            struct SchemeVal *paramNames;
            struct SchemeVal *functionCode;
            struct Frame *frame;
        }; // For CLOSURE_TYPE
        void *ptr;
        bool b;
        // A primitive style function; just a pointer to it, with the right
        // signature (pf = primitive function)
        struct SchemeVal *(*pf)(struct SchemeVal *);
    };
} SchemeVal;

// A frame is a linked list of bindings, and a pointer to another frame.  A
// binding is a variable name (represented as a string), and a pointer to the
// Object it is bound to. Specifically how you implement the list of bindings
// is up to you.
typedef struct Frame {
    SchemeVal *bindings;
    struct Frame *parent;
} Frame;

#endif

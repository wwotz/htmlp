#ifndef HTMLP_H_
#define HTMLP_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct buffer_t {
        char *data;
        int size;
        int capacity;
} buffer_t;

typedef enum HTML_TYPE {
        HTML_EOF = 0,
        HTML_EMPTY,
        HTML_OPEN_ANGLE,
        HTML_CLOSE_ANGLE,
        HTML_NUMBER,
        HTML_STRING
} HTML_TYPE;

#endif // HTMLP_H_

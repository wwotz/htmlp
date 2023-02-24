#ifndef HTMLP_H_
#define HTMLP_H_

/*
 *  To include the implementation in your program,
 *      write: #define HTMLP_IMPLEMENTATION
 *  before including the source file in ONE C/C++ source file.
*/

#define HTMLP_STATIC static
#define HTMLP_EXTERN extern

#define HTMLP_VERSION 1

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* stretchy buffer */
#define HTMLP_BUFFER_CAPACITY 256

typedef struct {
        char *data;
        int size;
        int capacity;
} buffer_t;

/* json token types */
typedef enum {
        HTMLP_TYPE_EOF = 0,
        HTMLP_TYPE_EMPTY,
        HTMLP_TYPE_OPEN_TAG,
        HTMLP_TYPE_CLOSE_TAG,
        HTMLP_TYPE_STRING,
        HTMLP_TYPE_UNDEFINED,
        HTMLP_TYPE_ERROR,
        HTMLP_TYPE_COUNT,
} HTMLP_TYPE;

/* json parser initialisation error codes */
typedef enum {
        HTMLP_NO_ERROR = 0,
        HTMLP_FILE_ERROR,
        HTMLP_BUFFER_ERROR,
        HTMLP_ERROR_COUNT,
} HTMLP_ERROR;

/* structure storing token information */
typedef struct {
        buffer_t token;
        HTMLP_TYPE type;
} htmlp_token;

/* json info type enums used to describe the data
   being passed into the htmlp_info structure */
typedef enum {
        HTMLP_FILE = 0,
        HTMLP_TEXT,
        HTMLP_INFO_DATA_COUNT
} HTMLP_INFO_DATA_TYPE;

/* structure storing the parser initialisation
   information; @type describes how @data should be
   interpreted:
             HTMLP_FILE - @data is a file path
             HTMLP_TEXT - @data is the json data
*/
typedef struct {
        HTMLP_INFO_DATA_TYPE type;
        const char *data;
} htmlp_info_t;

/* json buffer errors to describe the type of error that occurred within
   the operations on the buffer_t structure */
typedef enum {
        HTMLP_NO_BUFFER_ERROR = 0, HTMLP_NULL_BUFFER_ERROR,
        HTMLP_DATA_BUFFER_ERROR, HTMLP_RESIZE_BUFFER_ERROR,
        HTMLP_BUFFER_ERRORS_COUNT,
} HTMLP_BUFFER_ERRORS;

/* create a htmlp info structure */
HTMLP_EXTERN htmlp_info_t htmlp_create_json_info(HTMLP_INFO_DATA_TYPE type,
                                                 const char *data);
/* initialise the json parpser using the information stored in
   the htmlp_info_t structure */
HTMLP_EXTERN int htmlp_init(htmlp_info_t info);

/* free the contents held in the json parser global state,
   this will free the parser's copy of the buffer, and/or close
   the open file it has */
HTMLP_EXTERN int htmlp_free(void);

/* operations on the buffer_t structure, returns zero on success,
   otherwise non-zero on error, errors can be queried using a call
   to 'htmlp_get_error()' */
HTMLP_EXTERN int htmlp_init_buffer(buffer_t *buffer);
HTMLP_EXTERN int htmlp_clear_buffer(buffer_t *buffer);
HTMLP_EXTERN int htmlp_append_buffer(buffer_t *buffer, char c);
HTMLP_EXTERN int htmlp_write_buffer(buffer_t *buffer, const char *data);
HTMLP_EXTERN int htmlp_insert_buffer(buffer_t *buffer, const char *data, int offset);
HTMLP_EXTERN int htmlp_resize_buffer(buffer_t *buffer);
HTMLP_EXTERN int htmlp_free_buffer(buffer_t *buffer);

/* operations on the htmlp_token structure */
HTMLP_EXTERN HTMLP_TYPE htmlp_get_type_token(htmlp_token tok);
HTMLP_EXTERN const char *htmlp_get_data_token(htmlp_token tok);

/* token operations */
HTMLP_EXTERN htmlp_token htmlp_peek_token();
HTMLP_EXTERN htmlp_token htmlp_get_token();
HTMLP_EXTERN htmlp_token htmlp_unget_token(htmlp_token tok);
HTMLP_EXTERN int htmlp_rewind(void);

/* error code operations */
HTMLP_EXTERN int htmlp_had_error(void);
HTMLP_EXTERN const char *htmlp_get_error(void);


#ifdef __cplusplus
}
#endif

#ifdef HTMLP_IMPLEMENTATION

/* removed when debugging is being removed */
#define HTMLP_DEBUG

#ifdef HTMLP_DEBUG

#define HTMLP_TOKEN_STACK_CAPACITY 10

/* @tok stores the current token
   @lookahead stores the current character in the file or buffer
   @curr_fd stores the file being read
*/
HTMLP_STATIC htmlp_token tok;
HTMLP_STATIC int lookahead;
HTMLP_STATIC FILE *curr_fd;
HTMLP_STATIC buffer_t curr_buffer;
HTMLP_STATIC int curr_buffer_ptr;
HTMLP_STATIC int (* next_char)(void);
HTMLP_STATIC int peek_char(void);

/* used in order to un-get tokens, and peek tokens */
HTMLP_STATIC int htmlp_token_stack_size = 0;
HTMLP_STATIC int htmlp_token_stack_ptr = 0;
HTMLP_STATIC htmlp_token htmlp_token_stack[HTMLP_TOKEN_STACK_CAPACITY] = {0};

/* htmlp token stack used for token manipulation */
HTMLP_STATIC int htmlp_push_token_stack(htmlp_token tok);
HTMLP_STATIC htmlp_token htmlp_pop_token_stack(void);
HTMLP_STATIC int htmlp_empty_token_stack(void);
HTMLP_STATIC int htmlp_full_token_stack(void);

#define HTMLP_DEBUG_STACK_CAPACITY 20

HTMLP_STATIC int htmlp_debug_stack_size = 0;
HTMLP_STATIC int htmlp_debug_stack_ptr = 0;
HTMLP_STATIC char *htmlp_debug_stack[HTMLP_DEBUG_STACK_CAPACITY];

HTMLP_STATIC int htmlp_push_error_debug(const char *msg);
HTMLP_STATIC const char *htmlp_pop_error_debug(void);
HTMLP_STATIC int htmlp_stack_full_debug(void);
HTMLP_STATIC int htmlp_stack_empty_debug(void);

/* functions to return token primitives */
HTMLP_STATIC htmlp_token htmlp_empty_token();
HTMLP_STATIC htmlp_token htmlp_eof_token();
HTMLP_STATIC htmlp_token htmlp_open_tag_token();
HTMLP_STATIC htmlp_token htmlp_close_tag_token();
HTMLP_STATIC htmlp_token htmlp_undefined_token();
HTMLP_STATIC htmlp_token htmlp_error_token(const char *msg);

HTMLP_STATIC int htmlp_push_token_stack(htmlp_token tok)
{
        if (!htmlp_full_token_stack())
                htmlp_token_stack_size++;
        htmlp_token_stack[htmlp_token_stack_ptr].type = tok.type;

        int status;
        if ((status = htmlp_write_buffer(&htmlp_token_stack[htmlp_token_stack_ptr].token, tok.token.data)) != HTMLP_NO_BUFFER_ERROR)
                return status;
        htmlp_token_stack_ptr = (htmlp_token_stack_ptr + 1) % HTMLP_TOKEN_STACK_CAPACITY;
        return HTMLP_NO_ERROR;
}

HTMLP_STATIC htmlp_token htmlp_pop_token_stack(void)
{
        if (htmlp_empty_token_stack())
                return htmlp_empty_token();

        htmlp_token_stack_ptr--;
        htmlp_token_stack_size--;
        if (htmlp_token_stack_ptr < 0)
                htmlp_token_stack_ptr += HTMLP_TOKEN_STACK_CAPACITY;

        tok.type = htmlp_token_stack[htmlp_token_stack_ptr].type;
        htmlp_write_buffer(&tok.token, htmlp_token_stack[htmlp_token_stack_ptr].token.data);
        return tok;
}

HTMLP_STATIC int htmlp_empty_token_stack(void)
{
        return htmlp_token_stack_size == 0;
}

HTMLP_STATIC int htmlp_full_token_stack(void)
{
        return htmlp_token_stack_size == HTMLP_TOKEN_STACK_CAPACITY;
}

HTMLP_STATIC int htmlp_push_error_debug(const char *msg)
{
        if (msg != NULL) {
                if (!htmlp_stack_full_debug())
                        htmlp_debug_stack_size++;
                if (htmlp_debug_stack[htmlp_debug_stack_ptr] != NULL)
                        free(htmlp_debug_stack[htmlp_debug_stack_ptr]);
                if (!(htmlp_debug_stack[htmlp_debug_stack_ptr] = strdup(msg))) {
                        htmlp_debug_stack_ptr--;
                        return -1;
                } else {
                        htmlp_debug_stack_ptr = (htmlp_debug_stack_ptr + 1)
                                % HTMLP_DEBUG_STACK_CAPACITY;
                }
        }
        return 0;
}

HTMLP_STATIC const char *htmlp_pop_error_debug(void)
{
        if (!htmlp_stack_empty_debug()) {
                htmlp_debug_stack_ptr--;
                if (htmlp_debug_stack_ptr < 0)
                        htmlp_debug_stack_ptr += HTMLP_DEBUG_STACK_CAPACITY;
                return htmlp_debug_stack[htmlp_debug_stack_ptr];
        }

        return "No Error";
}

HTMLP_STATIC int htmlp_stack_full_debug(void)
{
        return htmlp_debug_stack_size == HTMLP_DEBUG_STACK_CAPACITY;
}

HTMLP_STATIC int htmlp_stack_empty_debug(void)
{
        return htmlp_debug_stack_size == 0;
}

#define JP_PUSH_ERROR_DEBUG(msg) jp_push_error_debug(msg);

#else /* !defined(HTMLP_DEBUG) */

#define JP_PUSH_ERROR_DEBUG(msg) void(0)

#endif /* HTMLP_DEBUG */

HTMLP_STATIC int next_char_file(void)
{
        return fgetc(curr_fd);
}

HTMLP_STATIC int next_char_buffer(void)
{
        if (curr_buffer_ptr < curr_buffer.size)
                return curr_buffer.data[curr_buffer_ptr++];
        return EOF;
}

HTMLP_STATIC htmlp_token htmlp_empty_token()
{
        memset(&tok, 0, sizeof(tok));
        if (htmlp_init_buffer(&tok.token) != HTMLP_NO_BUFFER_ERROR)
                return tok;

        tok.type = HTMLP_TYPE_EMPTY;
        htmlp_clear_buffer(&tok.token);
        return tok;
}

HTMLP_STATIC htmlp_token htmlp_eof_token()
{
        tok = htmlp_empty_token();
        tok.type = HTMLP_TYPE_EOF;
        htmlp_write_buffer(&tok.token, "EOF");
        lookahead = next_char();
        return tok;
}

HTMLP_STATIC htmlp_token htmlp_open_tag_token()
{
        tok = htmlp_empty_token();
        tok.type = HTMLP_TYPE_OPEN_TAG;
        return tok;
}

HTMLP_STATIC htmlp_token htmlp_close_tag_token()
{
        tok = htmlp_empty_token();
        tok.type = HTMLP_TYPE_CLOSE_TAG;
        htmlp_append_buffer(&tok.token, lookahead);
        lookahead = next_char();
        while (lookahead != '>' && lookahead != '<' && lookahead != EOF) {
                htmlp_append_buffer(&tok.token, lookahead);
                lookahead = next_char();
        }

        if (lookahead == EOF || lookahead == '<') {
                tok = htmlp_error_token("Unterminated html tag");
        } else {
                lookahead = next_char();
        }
        return tok;
}

HTMLP_STATIC htmlp_token htmlp_undefined_token()
{
        tok = htmlp_empty_token();
        tok.type = HTMLP_TYPE_UNDEFINED;
        htmlp_write_buffer(&tok.token, "UNDEFINED");
        lookahead = next_char();
        return tok;
}

HTMLP_STATIC htmlp_token htmlp_error_token(const char *msg)
{
        tok = htmlp_empty_token();
        tok.type = HTMLP_TYPE_ERROR;
        if (msg != NULL)
                htmlp_write_buffer(&tok.token, msg);
        else
                htmlp_write_buffer(&tok.token, "Error: no description");
        return tok;
}

HTMLP_STATIC const char *htmlp_get_error_init(int status)
{
        static const char *msgs[HTMLP_ERROR_COUNT] = {
                "No Error",
                "File does not exist!",
                "Could not create buffer!",
        };

        return (status >= 0 && status < HTMLP_ERROR_COUNT
            ? msgs[status] : "Undefined Error!");
}

HTMLP_EXTERN htmlp_info_t htmlp_create_json_info(HTMLP_INFO_DATA_TYPE type,
                                              const char *data)
{
        HTMLP_INFO_DATA_TYPE my_type = type;
        const char *my_data = data;
        if (type < 0 || type >= HTMLP_INFO_DATA_COUNT)
                type = HTMLP_TEXT;

        return (htmlp_info_t) {
                .type = type,
                .data = data
        };
}

HTMLP_EXTERN int htmlp_init(htmlp_info_t info)
{
        tok = htmlp_empty_token();
        curr_buffer_ptr = 0;

        switch (info.type) {
        case HTMLP_FILE:
                next_char = next_char_file;
                curr_fd = fopen(info.data, "r");
                if (!curr_fd) {
                        htmlp_push_error_debug(htmlp_get_error_init(HTMLP_FILE_ERROR));
                        return HTMLP_FILE_ERROR;
                }
                break;
        case HTMLP_TEXT:
        default: /* HTMLP_TEXT is also the default case */
                next_char = next_char_buffer;
                htmlp_init_buffer(&curr_buffer);
                htmlp_write_buffer(&curr_buffer, info.data);
                if (htmlp_had_error()) {
                        htmlp_push_error_debug(htmlp_get_error_init(HTMLP_BUFFER_ERROR));
                        return HTMLP_BUFFER_ERROR;
                }
                break;
        }

        lookahead = next_char();
        return HTMLP_NO_ERROR;
}

HTMLP_EXTERN int htmlp_free(void)
{
        if (curr_fd) {
                fclose(curr_fd);
                curr_fd = NULL;
        }

        htmlp_free_buffer(&curr_buffer);

        return HTMLP_NO_ERROR;
}

HTMLP_STATIC const char *htmlp_get_error_buffer(int status)
{
        static const char *msgs[HTMLP_BUFFER_ERRORS_COUNT] = {
                "No error",
                "buffer was null!",
                "data was null!",
                "failed to resize buffer!",
        };

        return (status >= 0 && status < HTMLP_BUFFER_ERRORS_COUNT
                ? msgs[status] : "Unknown Error");
}

HTMLP_EXTERN int htmlp_init_buffer(buffer_t *buffer)
{
        if (buffer == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_NULL_BUFFER_ERROR));
                return HTMLP_NULL_BUFFER_ERROR;
        }

        if (buffer->data == NULL)
                buffer->data = (typeof(buffer->data))malloc(sizeof(*buffer->data)
                                      * (HTMLP_BUFFER_CAPACITY + 1));

        if (buffer->data == NULL) {
                htmlp_free_buffer(buffer);
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_DATA_BUFFER_ERROR));
                return HTMLP_DATA_BUFFER_ERROR;
        }

        buffer->size = 0;
        buffer->capacity = HTMLP_BUFFER_CAPACITY;
        return htmlp_clear_buffer(buffer);
}

HTMLP_EXTERN int htmlp_clear_buffer(buffer_t *buffer)
{
        if (buffer == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_NULL_BUFFER_ERROR));
                return HTMLP_NULL_BUFFER_ERROR;
        }

        if (buffer->data == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_DATA_BUFFER_ERROR));
                return HTMLP_DATA_BUFFER_ERROR;
        }

        memset(buffer->data, 0,
               (buffer->capacity + 1) * sizeof(*buffer->data));
        buffer->size = 0;
        return HTMLP_NO_BUFFER_ERROR;
}

HTMLP_EXTERN int htmlp_append_buffer(buffer_t *buffer, char c)
{
        if (buffer == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_NULL_BUFFER_ERROR));
                return HTMLP_NULL_BUFFER_ERROR;
        }

        if (buffer->data == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_DATA_BUFFER_ERROR));
                return HTMLP_DATA_BUFFER_ERROR;
        }

        if (buffer->size >= buffer->capacity) {
                if (htmlp_resize_buffer(buffer) != 0) {
                        htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_RESIZE_BUFFER_ERROR));
                        return HTMLP_RESIZE_BUFFER_ERROR;
                }
        }

        buffer->data[buffer->size++] = c;
        buffer->data[buffer->size] = '\0';
        return HTMLP_NO_BUFFER_ERROR;
}

HTMLP_EXTERN int htmlp_write_buffer(buffer_t *buffer, const char *data)
{
        return htmlp_insert_buffer(buffer, data, 0);
}

HTMLP_EXTERN int htmlp_insert_buffer(buffer_t *buffer, const char *data, int offset)
{
        if (buffer == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_NULL_BUFFER_ERROR));
                return HTMLP_NULL_BUFFER_ERROR;
        }

        if (buffer->data == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_DATA_BUFFER_ERROR));
                return HTMLP_DATA_BUFFER_ERROR;
        }

        if (data != NULL) {
                int data_len = strlen(data) + offset;
                if (data_len >= buffer->capacity) {
                        int status;
                        if ((status = htmlp_resize_buffer(buffer))
                            != HTMLP_NO_BUFFER_ERROR)
                                return status;
                }

                strncpy(buffer->data+offset, data, buffer->capacity);
                buffer->size = data_len;
                buffer->data[data_len] = '\0';
        }

        return HTMLP_NO_BUFFER_ERROR;
}

HTMLP_EXTERN int htmlp_resize_buffer(buffer_t *buffer)
{
        if (buffer == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_NULL_BUFFER_ERROR));
                return HTMLP_NULL_BUFFER_ERROR;
        }

        if (buffer->data == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_DATA_BUFFER_ERROR));
                return HTMLP_DATA_BUFFER_ERROR;
        }

        char *new_data = (typeof(new_data))realloc(buffer->data, (buffer->capacity*2) + 1);
        if (new_data == NULL) {
                htmlp_push_error_debug(htmlp_get_error_buffer(HTMLP_RESIZE_BUFFER_ERROR));
                return HTMLP_RESIZE_BUFFER_ERROR;
        }

        buffer->data = new_data;
        buffer->capacity *= 2;
        return HTMLP_NO_BUFFER_ERROR;
}

HTMLP_EXTERN int htmlp_free_buffer(buffer_t *buffer)
{
        if (buffer != NULL) {
                if (buffer->data != NULL) {
                        htmlp_clear_buffer(buffer);
                        free(buffer->data);
                        buffer->data = NULL;
                }

                buffer->size = 0;
                buffer->capacity = 0;
        }
        return HTMLP_NO_BUFFER_ERROR;
}

HTMLP_EXTERN HTMLP_TYPE htmlp_get_type_token(htmlp_token tok)
{
        return tok.type;
}

HTMLP_EXTERN const char *htmlp_get_data_token(htmlp_token tok)
{
        return tok.token.data;
}

HTMLP_EXTERN htmlp_token htmlp_peek_token()
{
        htmlp_push_token_stack(htmlp_get_token());
        return tok;
}

HTMLP_EXTERN htmlp_token htmlp_get_token()
{
        if (!htmlp_empty_token_stack())
                return htmlp_pop_token_stack();

        while (lookahead == ' ' || lookahead == '\t'
               || lookahead == '\n' || lookahead == '\r')
                lookahead = next_char();

        switch (lookahead) {
        case EOF:
                return htmlp_eof_token();
        case '<':

                return htmlp_open_tag_token();
                return htmlp_close_tag_token();
        default:
                return htmlp_undefined_token();
        }
}

HTMLP_EXTERN htmlp_token htmlp_unget_token(htmlp_token tok)
{
        htmlp_push_token_stack(tok);
        return tok;
}

HTMLP_EXTERN int htmlp_rewind(void)
{
        curr_buffer_ptr = 0;
        return fseek(curr_fd, 0, SEEK_SET);
}

HTMLP_EXTERN int htmlp_had_error(void)
{
        return !htmlp_stack_empty_debug();
}

HTMLP_EXTERN const char *htmlp_get_error(void)
{
        return htmlp_pop_error_debug();
}

#endif /* HTMLP_IMPLEMENTATION */

#endif // HTMLP_H_
